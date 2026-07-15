#include "application/netif/impl_utils.h"

#include <stdbool.h>

#include "application/netif/impl_types.h"
#include "domain/contracts/network/interface.h"
#include "domain/models/error.h"

/* Helper Function Prototypes */

static bool has_network_interface_functions(dom_contracts_network_interface_t* network_interface);

dom_models_error_t app_netif_impl_validate_cfg(const app_netif_impl_cfg_t* cfg) {
    if (!cfg ||
        !cfg->logger ||
        !cfg->logger->error ||
        !cfg->logger->info ||
        !has_network_interface_functions(cfg->network_interface)) {
        return DOMAIN_MODELS_ERROR_BAD_ARGUMENT;
    }

    return DOMAIN_MODELS_ERROR_OK;
}

/* Helper Function Implementations */

static bool has_network_interface_functions(dom_contracts_network_interface_t* network_interface) {
    return network_interface &&
           network_interface->get_all &&
           network_interface->get_wifi_sta &&
           network_interface->get_ethernet;
}
