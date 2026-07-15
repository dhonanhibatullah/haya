#ifndef INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_TYPES_H
#define INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool registration_ack_subscribed;
    bool update_subscribed;
    bool restart_subscribed;
} inf_messaging_subscribe_stub_impl_cfg_t;

#define INF_MESSAGING_SUBSCRIBE_STUB_IMPL_CFG_DEFAULT() \
    {                                                   \
        .registration_ack_subscribed = false,           \
        .update_subscribed           = false,           \
        .restart_subscribed          = false,           \
    }

typedef struct {
    bool   registration_ack_subscribed;
    bool   update_subscribed;
    bool   restart_subscribed;
    size_t registration_ack_subscribe_cnt;
    size_t update_subscribe_cnt;
    size_t restart_subscribe_cnt;
} inf_messaging_subscribe_stub_impl_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* INFRASTRUCTURE_MESSAGING_SUBSCRIBE_STUB_IMPL_TYPES_H */
