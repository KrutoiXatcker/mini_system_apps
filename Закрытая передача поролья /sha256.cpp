#include "sha256.h"

// Константы для SHA-256
const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Функция для правого сдвига
inline uint32_t right_rotate(uint32_t value, uint32_t bits) {
    return (value >> bits) | (value << (32 - bits));
}

// Основная функция SHA-256
std::string sha256(const std::string& input) {
    // Инициализация переменных
    uint32_t h0 = 0x6a09e667;
    uint32_t h1 = 0xbb67ae85;
    uint32_t h2 = 0x3c6ef372;
    uint32_t h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f;
    uint32_t h5 = 0x9b05688c;
    uint32_t h6 = 0x1f83d9ab;
    uint32_t h7 = 0x5be0cd19;

    // Подготовка сообщения
    std::vector<uint8_t> message(input.begin(), input.end());
    uint64_t original_length = message.size() * 8;
    message.push_back(0x80);
    while ((message.size() * 8 + 64) % 512 != 0) {
        message.push_back(0x00);
    }
    for (int i = 0; i < 8; ++i) {
        message.push_back((original_length >> (56 - i * 8)) & 0xFF);
    }

    // Обработка сообщения блоками по 512 бит
    for (size_t chunk_start = 0; chunk_start < message.size(); chunk_start += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (message[chunk_start + i * 4] << 24) |
                    (message[chunk_start + i * 4 + 1] << 16) |
                    (message[chunk_start + i * 4 + 2] << 8) |
                    (message[chunk_start + i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = right_rotate(w[i - 15], 7) ^ right_rotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = right_rotate(w[i - 2], 17) ^ right_rotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;
        uint32_t f = h5;
        uint32_t g = h6;
        uint32_t h = h7;

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t temp1 = h + S1 + ch + K[i] + w[i];
            uint32_t S0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }

    // Формирование итогового хеша
    std::ostringstream result;
    result << std::hex << std::setfill('0')
           << std::setw(8) << h0
           << std::setw(8) << h1
           << std::setw(8) << h2
           << std::setw(8) << h3
           << std::setw(8) << h4
           << std::setw(8) << h5
           << std::setw(8) << h6
           << std::setw(8) << h7;

    return result.str();
}


// Функция для применения соли к хешу с использованием побитовых операций
std::string salt_hash_bitwise(const std::string& hash, uint8_t salt_value) {
    std::string salted_hash = hash; // Копируем исходный хеш

    // Применяем побитовые операции к каждому байту хеша
    for (size_t i = 0; i < salted_hash.length(); ++i) {
        salted_hash[i] = (salted_hash[i] + salt_value) ^ salt_value;
    }

    // Преобразуем результат в hex-строку
    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0'); // Устанавливаем вывод в hex и заполнение нулями

    for (unsigned char byte : salted_hash) {
        hex_stream << std::setw(2) << static_cast<int>(byte); // Преобразуем каждый байт в hex
    }

    return hex_stream.str();
}