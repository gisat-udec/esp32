#pragma once
#include <vector>
#include <esp_wifi.h>
#include "packet.hpp"

constexpr uint8_t address1[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
constexpr uint8_t address2[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
constexpr uint8_t address3[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };

const uint8_t dot11_header[] = {
        0x08, 0x00, 0x00, 0x00,             /* preamble */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* addr1 - destination */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* addr2 - source */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* addr3 - source ssid */
        0x00, 0x00,                         /* sc */
        0x00, 0x00, 0x00,                   /* LLC */
        0x00, 0x00, 0x00, 0x00, 0x00        /* random padding that happens to work*/
};

namespace wifi {
    void init();
    IRAM_ATTR void send(const std::vector<Packet> &packets);
}