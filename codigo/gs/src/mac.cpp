// MAC address packet filtering from https://github.com/esp32-open-mac/esp32-open-mac
#include <Arduino.h>
#include "mac.hpp"

inline void write_register(uint32_t address, uint32_t value) {
    *((volatile uint32_t *)address) = value;
}

inline uint32_t read_register(uint32_t address) {
    return *((volatile uint32_t *)address);
}

static void set_enable_mac_addr_filter(uint8_t slot, bool enable) {
    // This will allow packets that match the filter to be queued in our reception queue
    // will also ack them once they arrive
    assert(slot <= 1);
    uint32_t addr = WIFI_MAC_ADDR_ACK_ENABLE_SLOT_0 + 8 * slot;
    if (enable) {
        write_register(addr, read_register(addr) | 0x10000);
    } else {
        write_register(addr, read_register(addr) & ~(0x10000));
    }
}

static void set_mac_addr_filter(uint8_t slot, uint8_t *addr) {
    assert(slot <= 1);
    write_register(WIFI_MAC_ADDR_SLOT_0 + slot * 8, addr[0] | addr[1] << 8 | addr[2] << 16 | addr[3] << 24);
    write_register(WIFI_MAC_ADDR_SLOT_0 + slot * 8 + 4, addr[4] | addr[5] << 8);
    write_register(WIFI_MAC_ADDR_SLOT_0 + slot * 8 + 8 * 4, ~0); // ?
}