/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "esp_eth.h"
#include "esp_eth_mac.h"
#include "esp_eth_phy.h"
#include <platform/ESP32/NetworkCommissioningDriver.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

static void on_eth_event(void * esp_netif, esp_event_base_t event_base, int32_t event_id, void * event_data)
{
    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED: {
        esp_netif_t * eth_netif = (esp_netif_t *) esp_netif;
        ChipLogProgress(DeviceLayer, "Ethernet Connected");
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(eth_netif));
    }
    break;
    default:
        break;
    }
}

CHIP_ERROR ESPEthernetDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    /* The W5500 SPI Ethernet interface is already set up by the application (eth.c).
     * Find the existing ESP-Netif instance by its well-known key and register an event
     * handler to create IPv6 link-local addresses on future connect events (needed for
     * Matter mDNS operational discovery over Ethernet).
     *
     * If Ethernet was already connected before Matter initialised (the common case when
     * ETH cable is plugged in at boot), call esp_netif_create_ip6_linklocal() immediately
     * as well, since no future ETHERNET_EVENT_CONNECTED will fire for that connection. */
    esp_netif_t * eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
    VerifyOrReturnError(eth_netif != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "ESPEthernetDriver: ETH_DEF netif not found — "
                                                  "ensure eth_init() runs before esp_matter::start()"));

    esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event, eth_netif);

    /* Harmless if link is not yet up or if an LL address already exists. */
    esp_netif_create_ip6_linklocal(eth_netif);

    return CHIP_NO_ERROR;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
