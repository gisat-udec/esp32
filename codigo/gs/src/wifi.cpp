#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_private/wifi.h>
#include <cstring>
#include "packet.hpp"
#include "wifi.hpp"

TaskHandle_t handle;
uint8_t tx_buf[MTU];

void wifi::init() {
	ESP_ERROR_CHECK(esp_wifi_internal_set_log_level(WIFI_LOG_NONE));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_11M_S));
	ESP_ERROR_CHECK(esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE));
	wifi_promiscuous_filter_t filter =
	{
		.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA
	};
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_ctrl_filter(&filter));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(rx_callback));
	ESP_ERROR_CHECK(esp_wifi_set_tx_done_cb(tx_callback));
	ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(80));
	std::copy_n(dot11_header, sizeof(dot11_header), tx_buf);
	handle = xTaskGetCurrentTaskHandle();
}

IRAM_ATTR void wifi::rx_callback(void *buf, wifi_promiscuous_pkt_type_t type) {
	auto *packet = reinterpret_cast<wifi_promiscuous_pkt_t *>(buf);

	// Filtrar paquetes por MAC
	if (std::memcmp(&packet->payload[4], address1, 6) != 0) { return; }
	if (std::memcmp(&packet->payload[10], address2, 6) != 0) { return; }
	if (std::memcmp(&packet->payload[16], address3, 6) != 0) { return; }

	// Enviar por ethernet
	const int payloadlen = packet->rx_ctrl.sig_len - 4;
	int cursor = sizeof(dot11_header);
	while (cursor < payloadlen - 1) {
		Packet p(&packet->payload[cursor]);
		if (p.subtype == 1) {
			//Serial.print("\npacket\n");
			for (int i = 0; i < p.data_len(); i++) {
				//Serial.print(*(p.data_ptr() + i), HEX);
			}
		}
		cursor += p.len();
	}

}

IRAM_ATTR void wifi::tx_callback(uint8_t ifidx, uint8_t *data, uint16_t *data_len, bool txStatus) {
	xTaskNotifyGive(handle);
}