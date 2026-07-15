#ifndef INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_H
#define INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_H

#include "domain/contracts/messaging/publish.h"
#include "infrastructure/messaging/publish/stub_impl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dom_contracts_messaging_publish_t* inf_messaging_publish_stub_impl_new(
    const inf_messaging_publish_stub_impl_cfg_t* cfg
);

void inf_messaging_publish_stub_impl_delete(dom_contracts_messaging_publish_t* self);

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_PUBLISH_STUB_IMPL_H */
