#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_private/wifi.h>
#include <cstring>
#include "packet.hpp"
#include "wifi.hpp"
#include "mac.hpp"
#include "ethernet.hpp"

namespace wifi {
    TaskHandle_t task;
    uint8_t tx_buffer[MTU];
    IRAM_ATTR void tx_callback(uint8_t ifidx, uint8_t *data, uint16_t *data_len, bool txStatus);
    IRAM_ATTR void rx_callback(void *buf, wifi_promiscuous_pkt_type_t type);
}

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
    std::copy_n(dot11_header, sizeof(dot11_header), tx_buffer);
    task = xTaskGetCurrentTaskHandle();
}

IRAM_ATTR void wifi::tx_callback(uint8_t ifidx, uint8_t *data, uint16_t *data_len, bool txStatus) {
    xTaskNotifyGive(task);
}

IRAM_ATTR void wifi::rx_callback(void *buf, wifi_promiscuous_pkt_type_t type) {
    auto *packet = reinterpret_cast<wifi_promiscuous_pkt_t *>(buf);

    // Filtrar paquetes por MAC
    if (std::memcmp(&packet->payload[4], address1, 6) != 0) { return; }
    if (std::memcmp(&packet->payload[10], address2, 6) != 0) { return; }
    if (std::memcmp(&packet->payload[16], address3, 6) != 0) { return; }

    // Enviar paquete por ethernet
    const int payloadlen = packet->rx_ctrl.sig_len - 4;
    int cursor = sizeof(dot11_header);
    while (cursor < payloadlen - 1) {
        Packet p(&packet->payload[cursor]);
        cursor += p.len();
        eth::send(std::move(p));
    }

    // Enviar estadisticas
    struct {
        int32_t rssi;
        uint32_t bytes;
    } stats = {
        packet->rx_ctrl.rssi,
        packet->rx_ctrl.sig_len
    };
    eth::send(Packet(PacketType::Stats, sizeof(stats), &stats));
}

size_t len = sizeof(dot11_header);
IRAM_ATTR void wifi::send(const std::vector<Packet> &packets) {
    /*
    for (auto packet = packets.begin(); packet != packets.end(); packet++) {
        // copy packet data into a buffer
        if ((len + packet->len()) <= MTU) {
            std::copy_n(packet->header_ptr(), packet->header_len(), &tx_buffer[len]);
            len += packet->header_len();
            std::copy_n(packet->data_ptr(), packet->data_len(), &tx_buffer[len]);
            len += packet->data_len();
        }
        // send buffer data over wifi if its going to exceed MTU
        auto next_packet = std::next(packet);
        if (next_packet == packets.end() || len + next_packet->len() > MTU) {
            ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_STA, tx_buffer, len, false));
            ulTaskNotifyTake(true, portMAX_DELAY);
            len = sizeof(dot11_header);
        }
    }
    */
}