#pragma once
#include <asio.hpp>

#define PORT 27015

#define EMAC_CLK_MODE   EMAC_CLK_OUT
#define EMAC_CLK_GPIO   EMAC_APPL_CLK_OUT_GPIO
#define ETH_POWER_PIN   4
#define ETH_ADDR        0
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

namespace eth {
	void init();
	void send();
}