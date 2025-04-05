#ifndef _SHA256_
#define _SHA256_


#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>
#include <stdint.h>


// Основная функция SHA-256
std::string sha256(const std::string& input);

// Функция для применения соли к хешу с использованием побитовых операций
std::string salt_hash_bitwise(const std::string& hash, uint8_t salt_value);


#endif