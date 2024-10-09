/**
 * @file port_xf_netif.h
 * @author catcatBlue (catcatblue@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-06-11
 *
 * Copyright (c) 2024, CorAL. All rights reserved.
 *
 */

#ifndef __PORT_XF_NETIF_H__
#define __PORT_XF_NETIF_H__

/* ==================== [Includes] ========================================== */

#include "lwip/netifapi.h"
#include "wifi_hotspot.h"
#include "wifi_hotspot_config.h"
#include "td_base.h"
#include "td_type.h"
#include "stdlib.h"

#include "cmsis_os2.h"
#include "app_init.h"
#include "soc_osal.h"

#include "port_common.h"

#include "xf_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

/* ==================== [Typedefs] ========================================== */

typedef struct port_xf_netif_hdl_ctx_s {
    struct netif *pf_netif;
} port_xf_netif_hdl_ctx_t;

typedef struct port_xf_netif_context_s {
    bool b_netif_inited;
} port_xf_netif_context_t;

/* ==================== [Global Prototypes] ================================= */

port_xf_netif_context_t *port_xf_netif_get_context(void);

/* ==================== [Macros] ============================================ */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PORT_XF_NETIF_H__ */
