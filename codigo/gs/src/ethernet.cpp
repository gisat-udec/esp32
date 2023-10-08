#include <Arduino.h>
#include <FreeRTOS/FreeRTOS.h>
#include <iostream>
#include <asio.hpp>
#include <esp_netif.h>
#include <esp_event.h>
#include "ethernet.hpp"

using asio::ip::udp;

namespace eth {
	asio::io_context *io_context;
	udp::socket *socket;
	static esp_eth_handle_t s_eth_handle = NULL;
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
	// start eth
	ESP_ERROR_CHECK(esp_eth_start(s_eth_handle));
	// start udp server
	io_context = new asio::io_context();
	socket = new udp::socket(*io_context, udp::endpoint(udp::v4(), PORT));
	xTaskCreate([](void *_socket) -> void {
		udp::socket *socket = static_cast<udp::socket *>(_socket);
		while (1) {
			char data[1500];
			udp::endpoint client;
			socket->receive_from(asio::buffer(data, 1500), client);
			std::cout << data << std::endl;
		}
		}, "asio_recv", 4000, socket, 1, nullptr);
}

void eth::send(void *ptr, size_t len) {
	socket->send_to(
		asio::buffer(ptr, len),
		udp::endpoint(asio::ip::address_v4::broadcast(), PORT));
}