#pragma once

#include <vector>
#include <esp_wifi.h>

#include "packet.hpp"

const uint8_t dot11_header[] = {
		0x08, 0x00, 0x00, 0x00,				/* preamble */
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* addr1 - destination */
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* addr2 - source */
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* addr3 - source ssid */
		0x00, 0x00,							/* sc */
		0x00, 0x00, 0x00,					/* LLC */
		0x00, 0x00, 0x00, 0x00, 0x00		/* random padding that happens to work*/
};

void wifi();
IRAM_ATTR void rx_callback(void *buf, wifi_promiscuous_pkt_type_t type);
IRAM_ATTR void tx_callback(uint8_t ifidx, uint8_t *data, uint16_t *data_len, bool txStatus);
IRAM_ATTR void send(const std::vector<Packet> &packets);