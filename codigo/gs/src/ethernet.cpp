#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "ethernet.hpp"


static const char *TAG = "eth_example";
static esp_eth_handle_t s_eth_handle = NULL;

static void do_retransmit(const int sock) {
	int len;
	char rx_buffer[128];

	do {
		len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
		if (len < 0) {
			ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
		} else if (len == 0) {
			ESP_LOGW(TAG, "Connection closed");
		} else {
			rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
			ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

			// send() can return less bytes than supplied length.
			// Walk-around for robust implementation.
			int to_write = len;
			while (to_write > 0) {
				int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
				if (written < 0) {
					ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
					// Failed to retransmit, giving up
					return;
				}
				to_write -= written;
			}
		}
	} while (len > 0);
}


void eth::init() {
	// set up eth config
	eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
	phy_config.phy_addr = ETH_ADDR;
	phy_config.reset_gpio_num = ETH_POWER_PIN;
	mac_config.smi_mdc_gpio_num = ETH_MDC_PIN;
	mac_config.smi_mdio_gpio_num = ETH_MDIO_PIN;
	mac_config.clock_config.rmii.clock_mode = EMAC_CLK_MODE;
	mac_config.clock_config.rmii.clock_gpio = EMAC_CLK_GPIO;
	esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
	esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
	esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
	ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));
	// set up netif
	ESP_ERROR_CHECK(esp_netif_init());
	const esp_netif_ip_info_t ipinfo = {
		ESP_IP4TOADDR(10,0,0,1),
		ESP_IP4TOADDR(255,255,255,0),
		ESP_IP4TOADDR(10,0,0,1)
	};
	esp_netif_inherent_config_t base_cfg = {
			.flags = (esp_netif_flags_t)(ESP_NETIF_FLAG_AUTOUP | ESP_NETIF_DHCP_SERVER),
			.ip_info = &ipinfo,
			.if_key = "wired",
			.if_desc = "ethernet config device",
			.route_prio = 10
	};
	esp_netif_config_t cfg = {
		.base = &base_cfg,
		.stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
	};
	esp_netif_t *netif = esp_netif_new(&cfg);
	esp_eth_netif_glue_handle_t eth_glue = esp_eth_new_netif_glue(s_eth_handle);
	uint8_t mac_addr[6] = { 0 };
	esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
	ESP_ERROR_CHECK(esp_netif_attach(netif, eth_glue));
	esp_netif_set_mac(netif, mac_addr);
	uint32_t lease_opt = 1;
	esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_IP_ADDRESS_LEASE_TIME, &lease_opt, sizeof(lease_opt));
	// run
	ESP_ERROR_CHECK(esp_eth_start(s_eth_handle));
	xTaskCreate(tcp_task, "tcp_task", 4096, (void *)AF_INET, 5, NULL);
}

static void eth::tcp_task(void *pvParameters) {
	char addr_str[128];
	int addr_family = (int)pvParameters;
	int ip_protocol = 0;
	int keepAlive = 1;
	int keepIdle = 5;
	int keepInterval = 5;
	int keepCount = 3;
	struct sockaddr_storage dest_addr;

	if (addr_family == AF_INET) {
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(PORT);
		ip_protocol = IPPROTO_IP;
	}


	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (listen_sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}
	int opt = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ESP_LOGI(TAG, "Socket created");

	int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
		goto CLEAN_UP;
	}
	ESP_LOGI(TAG, "Socket bound, port %d", PORT);

	err = listen(listen_sock, 1);
	if (err != 0) {
		ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
		goto CLEAN_UP;
	}

	while (1) {

		ESP_LOGI(TAG, "Socket listening");

		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
		socklen_t addr_len = sizeof(source_addr);
		int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
		if (sock < 0) {
			ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			break;
		}

		// Set tcp keepalive option
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

		if (source_addr.ss_family == PF_INET) {
			inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
		}

		ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

		do_retransmit(sock);

		shutdown(sock, 0);
		close(sock);
	}

CLEAN_UP:
	close(listen_sock);
	vTaskDelete(NULL);
}