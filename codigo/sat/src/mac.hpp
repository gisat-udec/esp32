// MAC address packet filtering from https://github.com/esp32-open-mac/esp32-open-mac

#define WIFI_MAC_ADDR_SLOT_0 0x3ff73040
#define WIFI_MAC_ADDR_ACK_ENABLE_SLOT_0 0x3ff73064

static void set_enable_mac_addr_filter(uint8_t slot, bool enable);
static void set_mac_addr_filter(uint8_t slot, uint8_t *addr);