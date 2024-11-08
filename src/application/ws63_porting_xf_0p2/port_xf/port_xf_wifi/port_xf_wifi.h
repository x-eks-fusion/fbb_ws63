/**
 * @file port_xf_wifi.h
 * @author catcatBlue (catcatblue@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-06-06
 *
 * Copyright (c) 2024, CorAL. All rights reserved.
 *
 */

#ifndef __PORT_XF_WIFI_H__
#define __PORT_XF_WIFI_H__

/* ==================== [Includes] ========================================== */

#include "stdlib.h"

#include "lwip/netifapi.h"
#include "wifi_hotspot.h"
#include "wifi_hotspot_config.h"
#include "td_base.h"
#include "td_type.h"

#include "cmsis_os2.h"
#include "app_init.h"
#include "soc_osal.h"

#include "port_common.h"
#include "port_xf_netif.h"

#include "xf_task.h"
#include "xf_utils.h"
#include "xf_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

#if !defined(WIFI_MAX_NUM_USER)
#define WIFI_MAX_NUM_USER           8
#endif

#define AP_TASK_DURATION_MS         500
#define AP_TASK_PRIO                24
#define AP_TASK_STACK_SIZE          0x2000

/* ==================== [Typedefs] ========================================== */

typedef struct port_xf_wifi_context_s {
    /* ap */
    xf_wifi_cb_t    ap_cb;
    void            *ap_user_args;
    /* sta */
    xf_wifi_cb_t    sta_cb;
    void            *sta_user_args;

    osal_task *ap_task_handle;
    osal_mutex ap_task_mtx;
    uint8_t apsta_mac[WIFI_MAX_NUM_USER][WIFI_MAC_LEN];
    uint8_t arr_new_join[WIFI_MAX_NUM_USER];

    wifi_sta_config_stru expected_bss;

    volatile uint32_t b_inited                  : 1;
    volatile uint32_t b_ap_start                : 1;
    volatile uint32_t b_sta_start               : 1;
    volatile uint32_t b_sta_connected           : 1;
    volatile uint32_t b_scanning                : 1;
    volatile uint32_t b_register_wifi_event     : 1;
    volatile uint32_t b_try_connect             : 1;
    volatile uint32_t b_ap_netif_is_inited      : 1;
    volatile uint32_t b_sta_netif_is_inited     : 1;
    volatile uint32_t reserve                   : 23;

    int scan_result_size;

    port_xf_netif_hdl_ctx_t netif_ap;
    port_xf_netif_hdl_ctx_t netif_sta;

    xf_ip4_addr_t ip_prev;
    xf_ip4_addr_t ip_curr;
    /*
    TODO 暂未支持 IPv6
    xf_ip6_addr_t ip6_prev;
    xf_ip6_addr_t ip6_curr;
     */

    xf_ip_cb_t  ap_ip_cb;
    void        *ap_ip_user_args;
    xf_ip_cb_t  sta_ip_cb;
    void        *sta_ip_user_args;

} port_xf_wifi_context_t;

/* ==================== [Global Prototypes] ================================= */

port_xf_wifi_context_t *port_xf_wifi_get_context(void);

/* ==================== [Macros] ============================================ */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PORT_XF_WIFI_H__ */
