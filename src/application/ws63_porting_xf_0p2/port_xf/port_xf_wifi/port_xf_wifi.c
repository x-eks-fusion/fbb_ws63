/**
 * @file port_xf_wifi.c
 * @author catcatBlue (catcatblue@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-06-06
 *
 * Copyright (c) 2024, CorAL. All rights reserved.
 *
 */

/* ==================== [Includes] ========================================== */

#include <string.h>

#include "platform_core_rom.h"
#include "common_def.h"
#include "app_init.h"

#include "lwip/netifapi.h"
#include "wifi_hotspot.h"
#include "lwip/nettool/ifconfig.h"

#include "wifi_hotspot_config.h"
#include "td_base.h"
#include "td_type.h"

#include "cmsis_os2.h"
#include "soc_osal.h"

#include "wifi_device.h"
#include "wifi_event.h"
#include "wifi_device_config.h"
#include "wifi_alg.h"
#include "wifi_hotspot.h"

#include "securec.h"

#include "port_xf_wifi.h"
#include "port_xf_netif.h"
#include "port_utils.h"

#include "xf_utils.h"

#include "xf_wifi.h"

/* ==================== [Defines] =========================================== */

#define WIFI_IFNAME_MAX_SIZE        16
#define WIFI_SCAN_AP_LIMIT          64

#define WAIT_CONN_SLEEP_TIME_MS     10
#define WAIT_CONN_SLEEP_COUNT_MAX   100

/*
    default max num of station.CNcomment:默认支持的station最大个数.CNend
    见 src/protocol/wifi/source/host/inc/liteOS/soc_wifi_api.h
 */
#define WIFI_DEFAULT_MAX_NUM_STA    8

#define __IFNAME_AP                 "ap0"
#define __IFNAME_STA                "wlan0"

/* ==================== [Typedefs] ========================================== */

typedef wifi_security_enum pf_wifi_auth_mode_t;

typedef struct port_convert_wifi_auth_mode_s {
    xf_wifi_auth_mode_t xf;
    pf_wifi_auth_mode_t pf;
} port_convert_wifi_auth_mode_t;

/*
    见 src/protocol/wifi/source/host/inc/liteOS/soc_wifi_api.h
    此头文件无法直接 include，在此重新定义。
 */
typedef enum {
    EXT_WIFI_BW_HIEX_5M,     /**< 5M bandwidth. CNcomment: 窄带5M带宽.CNend */
    EXT_WIFI_BW_HIEX_10M,    /**< 10M bandwidth. CNcomment: 窄带10M带宽.CNend */
    EXT_WIFI_BW_LEGACY_20M,  /**< 20M bandwidth. CNcomment: 20M带宽.CNend */
    EXT_WIFI_BW_BUTT
} ext_wifi_bw;

/* ==================== [Static Prototypes] ================================= */

static void port_ap_netif_init(void);
static void port_ap_netif_deinit(void);

static void port_sta_netif_init(void);
static void port_sta_netif_deinit(void);

static xf_err_t port_convert_wifi_auth_mode(
    xf_wifi_auth_mode_t *xf, pf_wifi_auth_mode_t *pf, bool is_xf2pf);
static pf_wifi_auth_mode_t  port_convert_xf2pf_wifi_auth_mode(xf_wifi_auth_mode_t authmode);
static xf_wifi_auth_mode_t  port_convert_pf2xf_wifi_auth_mode(pf_wifi_auth_mode_t authmode);

static int _register_wifi_event(void);

static void _scan_state_changed_handler(int state, int size);
static void _sta_connection_changed_handler(
    int state, const wifi_linked_info_stru *info, int reason_code);
static void _softap_sta_join_handler(const wifi_sta_info_stru *info);
static void _softap_sta_leave_handler(const wifi_sta_info_stru *info);
static void _softap_state_changed_handler(int state);

static void _sta_ip_assigned_handler(struct netif *netif_p);

/* 用于通知 IP 分配事件 */
static void *softap_task(const char *arg);

/* ==================== [Static Variables] ================================== */

static const char *TAG = "port_xf_wifi";

static const char sc_mac_zero[XF_MAC_LEN_MAX] = "\0\0\0\0\0\0";

static const wifi_event_stru s_event_handler = {
    .wifi_event_scan_state_changed        = _scan_state_changed_handler,
    .wifi_event_connection_changed        = _sta_connection_changed_handler,
    .wifi_event_softap_sta_join           = _softap_sta_join_handler,
    .wifi_event_softap_sta_leave          = _softap_sta_leave_handler,
    .wifi_event_softap_state_changed      = _softap_state_changed_handler,
};

static port_xf_wifi_context_t s_port_xf_wifi_context = {0};
static port_xf_wifi_context_t *sp_port_xf_wifi_context = &s_port_xf_wifi_context;
#define ctx_w() sp_port_xf_wifi_context

// static port_xf_netif_context_t *sp_port_xf_netif_context = NULL;
static port_xf_netif_context_t s_port_xf_netif_context = {0};
static port_xf_netif_context_t *sp_port_xf_netif_context = &s_port_xf_netif_context;
#define ctx_n() sp_port_xf_netif_context

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

port_xf_wifi_context_t *port_xf_wifi_get_context(void)
{
    return sp_port_xf_wifi_context;
}

xf_err_t xf_wifi_enable(void)
{
    ctx_n() = port_xf_netif_get_context();
    if (ctx_w()->b_inited) {
        return XF_ERR_INITED;
    }

    (void)osDelay(10); /* 1: 等待100ms后判断状态 */
    /* 等待wifi初始化完成 */
    while (wifi_is_wifi_inited() == 0) {
        wifi_init();
        (void)osDelay(10); /* 1: 等待100ms后判断状态 */
    }

    ctx_w()->b_inited = true;

    return XF_OK;
}

xf_err_t xf_wifi_disable(void)
{
    if (!ctx_w()->b_inited) {
        return XF_ERR_UNINIT;
    }

    /*
        FIXME 目前发现反初始化 wifi 有概率导致 task:tcpip_thread 崩溃，
        因此不反初始化 wifi
     */
    // wifi_deinit();

    ctx_w()->b_inited = false;
    return XF_OK;
}

xf_err_t xf_wifi_set_mac(
    xf_wifi_interface_t ifx, const uint8_t mac[XF_MAC_LEN_MAX])
{
    XF_CHECK(ifx >= XF_WIFI_IF_MAX, XF_ERR_INVALID_ARG,
             TAG, "ifx invalid");
    XF_CHECK(mac == NULL, XF_ERR_INVALID_ARG,
             TAG, "mac invalid");
    switch (ifx) {
    case XF_WIFI_IF_STA: {
        wifi_set_base_mac_addr((const int8_t *)mac, XF_MAC_LEN_MAX);
    } break;
    case XF_WIFI_IF_AP: {
        wifi_softap_set_mac_addr((const int8_t *)mac, XF_MAC_LEN_MAX);
    } break;
    default:
        break;
    }
    return XF_OK;
}

xf_err_t xf_wifi_get_mac(
    xf_wifi_interface_t ifx, uint8_t mac[XF_MAC_LEN_MAX])
{
    XF_CHECK(ifx >= XF_WIFI_IF_MAX, XF_ERR_INVALID_ARG,
             TAG, "ifx invalid");
    XF_CHECK(mac == NULL, XF_ERR_INVALID_ARG,
             TAG, "mac invalid");
    switch (ifx) {
    case XF_WIFI_IF_STA: {
        wifi_get_base_mac_addr((int8_t *)mac, XF_MAC_LEN_MAX);
    } break;
    case XF_WIFI_IF_AP: {
        wifi_softap_get_mac_addr((int8_t *)mac, XF_MAC_LEN_MAX);
    } break;
    default:
        break;
    }
    return XF_OK;
}

/* ap */

xf_err_t xf_wifi_ap_init(const xf_wifi_ap_cfg_t *p_cfg)
{
    if (ctx_w()->b_ap_start) {
        return XF_ERR_INITED;
    }

    XF_CHECK(NULL == p_cfg, XF_ERR_INVALID_ARG,
             TAG, "NULL==p_cfg");
    XF_CHECK(((false == p_cfg->ssid_hidden)
              && ('\0' == p_cfg->ssid[0])), XF_ERR_INVALID_ARG,
             TAG, "ssid length is 0.");

    pf_err_t pf_ret;

    /* SoftAp接口的信息 */
    softap_config_stru hapd_conf        = {0};
    softap_config_advance_stru config   = {0};

    (td_void)memcpy_s(hapd_conf.ssid,   sizeof(hapd_conf.ssid),
                      p_cfg->ssid,      ARRAY_SIZE(p_cfg->ssid));
    (td_void)memcpy_s(hapd_conf.pre_shared_key,     ARRAY_SIZE(hapd_conf.pre_shared_key),
                      p_cfg->password,              ARRAY_SIZE(p_cfg->password));
    hapd_conf.security_type     = port_convert_xf2pf_wifi_auth_mode(p_cfg->authmode); /* 加密方式 */
    if (WIFI_SEC_TYPE_INVALID == hapd_conf.security_type) {
        XF_LOGE(TAG, "auth_mode(%d) Not supported.", p_cfg->authmode);
        return XF_FAIL;
    }
    hapd_conf.channel_num       = p_cfg->channel;
    hapd_conf.wifi_psk_type     = 0;        /* TODO: wifi_psk_type 未知参数 */

    /* 配置SoftAp网络参数 */
    config.beacon_interval      = 100;      /*!< 100：Beacon周期设置为100ms */
    config.dtim_period          = 2;        /*!< 2：DTIM周期设置为2 */
    config.gi                   = 0;        /*!< 0：short GI默认关闭 */
    config.group_rekey          = 86400;    /*!< 86400：组播秘钥更新时间设置为1天 */
    config.protocol_mode        = WIFI_MODE_11B_G_N_AX;        /*!< 协议类型 */
    config.hidden_ssid_flag     = (p_cfg->ssid_hidden) ? (2) : (1);    /*!< 1：不隐藏SSID */

    /* 如果有扩展配置 */
    if (p_cfg->p_cfg_ext) {
        if (p_cfg->p_cfg_ext->b_set_beacon_interval) {
            config.beacon_interval  = p_cfg->p_cfg_ext->beacon_interval;
            if (config.beacon_interval < 25) {
                config.beacon_interval = 25;
            }
            if (config.beacon_interval > 1000) {
                config.beacon_interval = 1000;
            }
        }
        if (p_cfg->p_cfg_ext->b_set_country) {
            pf_ret = wifi_set_country_code(
                         (const int8_t *)p_cfg->p_cfg_ext->country.cc,
                         strnlen(p_cfg->p_cfg_ext->country.cc,
                                 ARRAY_SIZE(p_cfg->p_cfg_ext->country.cc)));
            if (ERRCODE_SUCC != pf_ret) {
                XF_LOGW(TAG, "Failed to set the country code.");
            }
        }
        if (p_cfg->p_cfg_ext->b_set_mac) {
            wifi_softap_set_mac_addr((const int8_t *)p_cfg->p_cfg_ext->mac,
                                     XF_MAC_LEN_MAX);
        }
    }

    /* 注册事件回调 */
    _register_wifi_event();

    pf_ret = wifi_set_softap_config_advance(&config);
    if (ERRCODE_SUCC != pf_ret) {
        XF_LOGE(TAG, "wifi_set_softap_config_advance() failed.");
        goto l_err;
    }

    /* 启动SoftAp接口 */
    if (wifi_softap_enable(&hapd_conf) != 0) {
        XF_LOGE(TAG, "wifi_softap_enable failed() failed.");
        goto l_err;
    }

    pf_ret = uapi_wifi_set_bandwidth(__IFNAME_AP, strlen(__IFNAME_AP) + 1,
                                     EXT_WIFI_BW_LEGACY_20M);
    if (ERRCODE_SUCC != pf_ret) {
        XF_LOGE(TAG, "uapi_wifi_set_bandwidth() failed.");
        goto l_err;
    }

    ctx_w()->b_ap_start = true;

    /* netif 放到 ap 启动后， "ap0" 必须要在 wifi 初始化之后才有 */

    /* ap 申请内部 netif 句柄并存储 */
    port_ap_netif_init();

    ip4_addr_t st_ipaddr;
    ip4_addr_t st_gw;
    ip4_addr_t st_netmask;
    IP4_ADDR(&st_ipaddr,    192, 168,   4, 1);  /* IP地址设置：192.168.4.1 */
    IP4_ADDR(&st_gw,        192, 168,   4, 1);  /* 网关地址设置：192.168.4.2 */
    IP4_ADDR(&st_netmask,   255, 255, 255, 0);  /* 子网掩码设置：255.255.255.0 */

    /* 配置DHCP服务器 */
    struct netif *netif_p = TD_NULL;
    td_char ifname[WIFI_IFNAME_MAX_SIZE + 1] = __IFNAME_AP; /* 创建的SoftAp接口名 */
    netif_p = netif_find(ifname);
    if (TD_NULL == netif_p) {
        XF_LOGE(TAG, "netif_find() failed.");
        goto l_err;
    }

    if (p_cfg->p_static_ip) {
        if (XF_IPADDR_TYPE_V4 == p_cfg->p_static_ip->type) {
            st_ipaddr.addr      = p_cfg->p_static_ip->ip4.ip.addr;
            st_gw.addr          = p_cfg->p_static_ip->ip4.netmask.addr;
            st_netmask.addr     = p_cfg->p_static_ip->ip4.gw.addr;
        } else {
            XF_LOGW(TAG, "IPv6 is not currently supported.");
        }
    }

    if (netifapi_netif_set_addr(netif_p, &st_ipaddr, &st_netmask, &st_gw) != 0) {
        XF_LOGE(TAG, "netifapi_netif_set_addr() failed.");
        goto l_err;
    }

    if (NULL == p_cfg->p_static_ip) {
        if (netifapi_dhcps_start(netif_p, NULL, 0) != 0) {
            XF_LOGE(TAG, "netifapi_dhcps_start() failed.");
            goto l_err;
        }
    }

    osal_mutex_init(&ctx_w()->ap_task_mtx);
    osal_kthread_lock();
    ctx_w()->ap_task_handle = osal_kthread_create(
                                  (osal_kthread_handler)softap_task,
                                  0,
                                  "APTask",
                                  AP_TASK_STACK_SIZE);
    if (ctx_w()->ap_task_handle != NULL) {
        osal_kthread_set_priority(ctx_w()->ap_task_handle, AP_TASK_PRIO);
        osal_kfree(ctx_w()->ap_task_handle);
    }
    osal_kthread_unlock();

    XF_LOGD(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
            p_cfg->ssid, p_cfg->password, p_cfg->channel);

    ctx_w()->b_ap_start = true;

    return XF_OK;

l_err:;
    ctx_w()->b_ap_start = false;
    (td_void)wifi_softap_disable();
    return XF_FAIL;
}

xf_err_t xf_wifi_ap_deinit(void)
{
    if (!ctx_w()->b_ap_start) {
        return XF_ERR_UNINIT;
    }

    osal_mutex_lock_timeout(&ctx_w()->ap_task_mtx, OSAL_WAIT_FOREVER);
    osal_kthread_destroy(ctx_w()->ap_task_handle, 0);
    osal_msleep(200);

    wifi_softap_disable();
    port_ap_netif_deinit();

    osal_mutex_destroy(&ctx_w()->ap_task_mtx);

    ctx_w()->b_ap_start = false;
    return XF_OK;
}

xf_err_t xf_wifi_ap_get_netif(xf_netif_t *p_netif_hdl)
{
    XF_CHECK(false == ctx_w()->b_ap_start, XF_ERR_UNINIT,
             TAG, "AP is not initialized");
    XF_CHECK(NULL == p_netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==p_netif_hdl");
    *p_netif_hdl = &ctx_w()->netif_ap;
    return XF_OK;
}

xf_err_t xf_wifi_ap_set_cb(xf_wifi_cb_t cb_func, void *user_args)
{
    ctx_w()->ap_cb         = cb_func;
    ctx_w()->ap_user_args  = user_args;
    return XF_OK;
}

xf_err_t xf_wifi_ap_set_ip_cb(xf_ip_cb_t cb_func, void *user_args)
{
    ctx_w()->ap_ip_cb           = cb_func;
    ctx_w()->ap_ip_user_args    = user_args;
    return XF_OK;
}

xf_err_t xf_wifi_ap_get_sta_list(
    xf_wifi_sta_info_t sta_array[], uint32_t sta_array_size,
    uint32_t *p_sta_num)
{
    XF_CHECK((NULL == p_sta_num), XF_ERR_INVALID_ARG,
             TAG, "NULL==p_sta_num");

    pf_err_t pf_ret;
    wifi_sta_info_stru sta_list_temp[WIFI_DEFAULT_MAX_NUM_STA] = {0};
    uint32_t pf_sta_num = WIFI_DEFAULT_MAX_NUM_STA;

    /* 没有只查询已连接的 sta 的个数的 api，直接读取全部 */
    pf_ret = wifi_softap_get_sta_list(sta_list_temp, &pf_sta_num);
    if (ERRCODE_SUCC != pf_ret) {
        return XF_FAIL;
    }

    *p_sta_num = pf_sta_num;

    if ((NULL == sta_array) || (0 == sta_array_size)) {
        return XF_OK;
    }

    for (uint32_t i = 0; (i < sta_array_size) && (i < ARRAY_SIZE(sta_list_temp)); ++i) {
        memcpy_s(sta_array[i].mac,              ARRAY_SIZE(sta_array[i].mac),
                 sta_list_temp[i].mac_addr,     ARRAY_SIZE(sta_list_temp[i].mac_addr));
        sta_array[i].rssi = sta_list_temp[i].rssi;
    }

    return XF_OK;
}

xf_err_t xf_wifi_ap_deauth_sta(const uint8_t mac[])
{
    XF_CHECK((NULL == mac), XF_ERR_INVALID_ARG,
             TAG, "NULL==mac");

    xf_err_t xf_ret;
    pf_err_t pf_ret;

    pf_ret = wifi_softap_deauth_sta(mac, XF_MAC_LEN_MAX);
    xf_ret = port_convert_pf2xf_err(pf_ret);
    return xf_ret;
}

/* ap */

/* sta */

xf_err_t xf_wifi_sta_init(const xf_wifi_sta_cfg_t *p_cfg)
{
    if (ctx_w()->b_sta_start) {
        return XF_ERR_INITED;
    }

    pf_err_t pf_ret;

    /* 需要在 uapi_wifi_set_bandwidth() 之前 */
    if (p_cfg && p_cfg->p_cfg_ext && p_cfg->p_cfg_ext->b_set_mac) {
        pf_ret = wifi_set_base_mac_addr((const int8_t *)p_cfg->p_cfg_ext->mac,
                                        XF_MAC_LEN_MAX);
    }

    /* 创建STA接口 */
    if (wifi_sta_enable() != 0) {
        XF_LOGE(TAG, "sta enable fail!");
        return XF_FAIL;
    }

    pf_ret = uapi_wifi_set_bandwidth(__IFNAME_STA, strlen(__IFNAME_STA) + 1,
                                     EXT_WIFI_BW_LEGACY_20M);
    if (ERRCODE_SUCC != pf_ret) {
        XF_LOGE(TAG, "uapi_wifi_set_bandwidth() failed.");
        return XF_FAIL;
    }

    ctx_w()->b_sta_start = true;

    _register_wifi_event();

    if (p_cfg) {
        memcpy_s(ctx_w()->expected_bss.ssid,        ARRAY_SIZE(ctx_w()->expected_bss.ssid),
                 p_cfg->ssid,                       ARRAY_SIZE(p_cfg->ssid));
        memcpy_s(ctx_w()->expected_bss.pre_shared_key,  ARRAY_SIZE(ctx_w()->expected_bss.pre_shared_key),
                 p_cfg->password,                       ARRAY_SIZE(p_cfg->password));
        ctx_w()->expected_bss.security_type    = port_convert_xf2pf_wifi_auth_mode(p_cfg->authmode);
    }
    ctx_w()->expected_bss.ip_type          = DHCP;
    ctx_w()->expected_bss.wifi_psk_type    = 0;     /*!< wifi_psk_type 未知参数 */

    if (ctx_w()->sta_cb) {
        ctx_w()->sta_cb(XF_WIFI_EVENT_STA_START, NULL, ctx_w()->sta_user_args);
    }

    port_sta_netif_init();

    struct netif *netif_p = TD_NULL;
    td_char ifname[WIFI_IFNAME_MAX_SIZE + 1] = __IFNAME_STA; /* 创建的STA接口名 */

    /* DHCP获取IP地址 */
    netif_p = netifapi_netif_find(ifname);
    if (netif_p == TD_NULL) {
        XF_LOGE(TAG, "netif_find() failed.");
        goto l_err;
    }

    // ip 事件回调
    netif_set_status_callback(netif_p, _sta_ip_assigned_handler);

    ctx_w()->netif_sta.pf_netif = netif_p;

    if ((NULL == p_cfg) || (NULL == p_cfg->p_static_ip)) {
        if (netifapi_dhcp_start(netif_p) != 0) {
            XF_LOGE(TAG, "STA DHCP Fail.");
            goto l_err;
        }
    } else if (XF_IPADDR_TYPE_V4 == p_cfg->p_static_ip->type) {
        /* TODO 暂未支持 IPv6 */
        ctx_w()->expected_bss.ip_type               = STATIC_IP;
        ctx_w()->expected_bss.static_ip.ip_address  = p_cfg->p_static_ip->ip4.ip.addr;
        ctx_w()->expected_bss.static_ip.gateway     = p_cfg->p_static_ip->ip4.gw.addr;
        ctx_w()->expected_bss.static_ip.netmask     = p_cfg->p_static_ip->ip4.netmask.addr;
        /* TODO 未配置 dns */
        ctx_w()->expected_bss.static_ip.dns_servers[0] = 0;
        ctx_w()->expected_bss.static_ip.dns_servers[1] = 0;
    }

    return XF_OK;

l_err:;
    wifi_sta_disable();
    ctx_w()->b_sta_start = false;
    return XF_FAIL;
}

xf_err_t xf_wifi_sta_deinit(void)
{
    if (!ctx_w()->b_sta_start) {
        return XF_ERR_UNINIT;
    }

    wifi_sta_disable();

    if (ctx_w()->sta_cb) {
        ctx_w()->sta_cb(XF_WIFI_EVENT_STA_STOP, NULL, ctx_w()->sta_user_args);
    }

    port_sta_netif_deinit();

    ctx_w()->expected_bss = (wifi_sta_config_stru) {
        0
    };
    ctx_w()->b_sta_start = false;

    return XF_OK;
}

xf_err_t xf_wifi_sta_get_netif(xf_netif_t *p_netif_hdl)
{
    XF_CHECK(false == ctx_w()->b_sta_start, XF_ERR_UNINIT,
             TAG, "STA is not initialized");
    XF_CHECK(NULL == p_netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==p_netif_hdl");
    *p_netif_hdl = &ctx_w()->netif_sta;
    return XF_OK;
}

xf_err_t xf_wifi_sta_set_cb(xf_wifi_cb_t cb_func, void *user_args)
{
    ctx_w()->sta_cb             = cb_func;
    ctx_w()->sta_user_args      = user_args;
    return XF_OK;
}

xf_err_t xf_wifi_sta_set_ip_cb(xf_ip_cb_t cb_func, void *user_args)
{
    ctx_w()->sta_ip_cb          = cb_func;
    ctx_w()->sta_ip_user_args   = user_args;
    return XF_OK;
}

xf_err_t xf_wifi_sta_connect(xf_wifi_sta_cfg_t *p_cfg)
{
    xf_err_t xf_ret = XF_FAIL;
    pf_err_t pf_ret;
    /* 防止中断回调中重入 */
    if (false == ctx_w()->b_try_connect) {
        ctx_w()->b_try_connect = true;
        if (NULL != p_cfg) {
            memcpy_s(ctx_w()->expected_bss.ssid,        ARRAY_SIZE(ctx_w()->expected_bss.ssid),
                     p_cfg->ssid,                       ARRAY_SIZE(p_cfg->ssid));
            memcpy_s(ctx_w()->expected_bss.pre_shared_key,  ARRAY_SIZE(ctx_w()->expected_bss.pre_shared_key),
                     p_cfg->password,                       ARRAY_SIZE(p_cfg->password));
            ctx_w()->expected_bss.security_type    = port_convert_xf2pf_wifi_auth_mode(p_cfg->authmode);
            if (p_cfg->bssid_set) {
                memcpy_s(ctx_w()->expected_bss.bssid,   ARRAY_SIZE(ctx_w()->expected_bss.bssid),
                         p_cfg->bssid,                  ARRAY_SIZE(p_cfg->bssid));
            }
            ctx_w()->expected_bss.channel = p_cfg->channel;
        }
        pf_ret = wifi_sta_connect(&ctx_w()->expected_bss);
        xf_ret = port_convert_pf2xf_err(pf_ret);
        ctx_w()->b_try_connect = false;
    }
    return xf_ret;
}

bool xf_wifi_sta_is_connected(void)
{
    xf_err_t xf_ret;
    pf_err_t pf_ret;
    wifi_linked_info_stru ap_info = {0};
    pf_ret = wifi_sta_get_ap_info(&ap_info);
    xf_ret = port_convert_pf2xf_err(pf_ret);
    return ((WIFI_CONNECTED == ap_info.conn_state)
            && (XF_OK == xf_ret));
}

xf_err_t xf_wifi_sta_disconnect(void)
{
    xf_err_t xf_ret;
    pf_err_t pf_ret;
    pf_ret = wifi_sta_disconnect();
    xf_ret = port_convert_pf2xf_err(pf_ret);
    /* 不需要手动清除 ctx_w()->b_sta_connected */
    return xf_ret;
}

xf_err_t xf_wifi_sta_get_ap_info(xf_wifi_ap_info_t *p_info)
{
    XF_CHECK(NULL == p_info, XF_ERR_INVALID_ARG,
             TAG, "NULL==p_info");
    xf_err_t xf_ret;
    pf_err_t pf_ret;
    wifi_linked_info_stru ap_info = {0};
    pf_ret = wifi_sta_get_ap_info(&ap_info);
    xf_ret = port_convert_pf2xf_err(pf_ret);
    if (XF_OK == xf_ret) {
        memcpy_s(p_info->bssid,     ARRAY_SIZE(p_info->bssid),
                 ap_info.bssid,     ARRAY_SIZE(ap_info.bssid));
        memcpy_s(p_info->ssid,      ARRAY_SIZE(p_info->ssid),
                 ap_info.ssid,      ARRAY_SIZE(ap_info.ssid));
        p_info->rssi        = ap_info.rssi;
        p_info->channel     = ap_info.channel_num;
    }
    return xf_ret;
}

xf_err_t xf_wifi_scan_start(const xf_wifi_scan_cfg_t *p_cfg, bool block)
{
    XF_CHECK(NULL == p_cfg, XF_ERR_INVALID_ARG,
             TAG, "NULL==p_cfg");

    /* FIXME 已连接状态下不会触发 _scan_state_changed_handler, 导致 b_scanning 不会修改 */
    // if (ctx_w()->b_scanning) {
    //     return XF_ERR_BUSY;
    // }
    ctx_w()->b_scanning = true;

    xf_err_t xf_ret;
    pf_err_t pf_ret;
    if (0 == p_cfg->channel) {
        pf_ret = wifi_sta_scan();
    } else {
        wifi_scan_params_stru scan_config = {0};
        UNUSED(scan_config.ssid);
        UNUSED(scan_config.ssid_len);
        UNUSED(scan_config.bssid);
        scan_config.channel_num     = p_cfg->channel;
        scan_config.scan_type       = WIFI_CHANNEL_SCAN;
        pf_ret = wifi_sta_scan_advance(&scan_config);
    }
    xf_ret = port_convert_pf2xf_err(pf_ret);
    UNUSED(block);
    if (block) {
        int count = 0;
        /* FIXME 已连接状态下不会触发 _scan_state_changed_handler */
        while (true == ctx_w()->b_scanning) {
            osal_msleep(WAIT_CONN_SLEEP_TIME_MS);
            count++;
            if (count >= WAIT_CONN_SLEEP_COUNT_MAX) {
                xf_ret = XF_ERR_TIMEOUT;
                xf_wifi_scan_stop();
                xf_wifi_scan_clear_result();
                break;
            }
        }
        ctx_w()->b_scanning = false;
    }
    return xf_ret;
}

xf_err_t xf_wifi_scan_stop(void)
{
    xf_err_t xf_ret;
    pf_err_t pf_ret;
    ctx_w()->b_scanning = false;
    pf_ret = wifi_sta_scan_stop();
    xf_ret = port_convert_pf2xf_err(pf_ret);
    return xf_ret;
}

xf_err_t xf_wifi_scan_get_result(
    xf_wifi_ap_info_t result[], uint32_t result_size,
    uint32_t *p_result_num)
{
    XF_CHECK((NULL == p_result_num), XF_ERR_INVALID_ARG,
             TAG, "NULL==p_result_num");

    pf_err_t pf_ret;

    if (true == ctx_w()->b_scanning) {
        return XF_ERR_BUSY;
    }

    *p_result_num = (uint32_t)ctx_w()->scan_result_size;
    if ((NULL == result) || (0 == result_size)
            || (0 == ctx_w()->scan_result_size)) {
        return XF_OK;
    }

    uint32_t size_min = min((uint32_t)ctx_w()->scan_result_size, result_size);

    uint32_t scan_len = sizeof(wifi_scan_info_stru) * size_min;
    wifi_scan_info_stru *result_tmp = osal_kmalloc(scan_len, OSAL_GFP_ATOMIC);
    if (result_tmp == TD_NULL) {
        return XF_ERR_NO_MEM;
    }
    memset_s(result_tmp, scan_len, 0, scan_len);
    pf_ret = wifi_sta_get_scan_info(result_tmp, &size_min);
    if (pf_ret != 0) {
        osal_kfree(result_tmp);
        return XF_FAIL;
    }

    uint32_t i = 0;
    for (; i < size_min; ++i) {
        memcpy(result[i].bssid, result_tmp[i].bssid,    XF_MAC_LEN_MAX);
        memcpy(result[i].ssid,  result_tmp[i].ssid,     ARRAY_SIZE(result_tmp[i].ssid));
        result[i].rssi      = result_tmp[i].rssi;
        result[i].authmode  = port_convert_pf2xf_wifi_auth_mode(result_tmp[i].security_type);
        result[i].channel   = result_tmp[i].channel_num;
    }
    *p_result_num = i;
    osal_kfree(result_tmp);

    return XF_OK;
}

xf_err_t xf_wifi_scan_clear_result(void)
{
    xf_err_t xf_ret;
    pf_err_t pf_ret;
    ctx_w()->scan_result_size = 0;
    pf_ret = wifi_sta_scan_result_clear();
    xf_ret = port_convert_pf2xf_err(pf_ret);
    return xf_ret;
}

/* sta */

/* ==================== [Static Functions] ================================== */

static void *softap_task(const char *arg)
{
    unused(arg);

    struct netif *netif_p = TD_NULL;
    ip_addr_t ip_addr[WIFI_MAX_NUM_USER] = {0};
    xf_ip_event_ip_assigned_t ip_assigned = {0};
    uint8_t *p_mac = NULL;

    xf_ip_event_id_t xf_eid = XF_IP_EVENT_IP_ASSIGNED;
    uint32_t osal_ret = OSAL_FAILURE;

    for (;;) {

        osal_msleep(50);

        if (!ctx_w()->ap_ip_cb) {
            continue;
        }

        netif_p = ctx_w()->netif_ap.pf_netif;
        if (netif_p == TD_NULL) {
            continue;
        }

        if (!osal_mutex_trylock(&ctx_w()->ap_task_mtx)) {
            continue;
        }

        /* 处理 XF_IP_EVENT_IP_ASSIGNED 事件 */
        for (int i = 0; i < (int)ARRAY_SIZE(ctx_w()->apsta_mac); i++) {
            if (true != ctx_w()->arr_new_join[i]) {
                continue;
            }
            p_mac = ctx_w()->apsta_mac[i];
            netifapi_dhcps_get_client_ip(
                netif_p, p_mac, ARRAY_SIZE(ctx_w()->apsta_mac[i]),
                &ip_addr[i]);
            if (0 != ip_addr[i].u_addr.ip4.addr) {
                ip_assigned.ip.addr         = xf_netif_htonl(ip_addr[i].u_addr.ip4.addr);
                ctx_w()->arr_new_join[i]    = 0;
                memcpy_s(ip_assigned.mac,   ARRAY_SIZE(ip_assigned.mac),
                         p_mac,             ARRAY_SIZE(ctx_w()->apsta_mac[i]));
                ctx_w()->ap_ip_cb(xf_eid, &ip_assigned, ctx_w()->ap_ip_user_args);
            }
        }

        osal_mutex_unlock(&ctx_w()->ap_task_mtx);
    }

    return NULL;
}

static void _sta_ip_assigned_handler(struct netif *netif_p)
{
    /*
        TODO 添加 XF_IP_EVENT_LOST_IP 事件支持
        TODO XF_IP_EVENT_GOT_IP6 暂未规划
     */
    if (netif_is_up(netif_p)) {
        if (!ctx_w()->sta_ip_cb) {
            return;
        }
        if (netif_p->ip_addr.u_addr.ip4.addr == ctx_w()->ip_curr.addr) {
            /* ip 地址未改变，忽略事件 */
            return;
        }

        /* ip_prev 暂时无用，之后用于指示 ip 是否改变（除了 0 地址） */
        ctx_w()->ip_prev.addr = ctx_w()->ip_curr.addr;
        ctx_w()->ip_curr.addr = netif_p->ip_addr.u_addr.ip4.addr;

        xf_ip_event_id_t xf_eid;
        xf_ip_event_got_ip_t got_ip = {0};
        xf_eid                      = XF_IP_EVENT_GOT_IP;
        got_ip.netif_hdl            = (xf_netif_t)&ctx_w()->netif_sta;
        got_ip.ip_info.ip.addr      = netif_p->ip_addr.u_addr.ip4.addr;
        got_ip.ip_info.netmask.addr = netif_p->netmask.u_addr.ip4.addr;
        got_ip.ip_info.gw.addr      = netif_p->gw.u_addr.ip4.addr;
        ctx_w()->sta_ip_cb(xf_eid, &got_ip, ctx_w()->sta_ip_user_args);
    }
}

static void _scan_state_changed_handler(int state, int size)
{
    xf_wifi_event_id_t xf_eid;
    xf_eid = XF_WIFI_EVENT_SCAN_DONE;
    XF_LOGD(TAG, "scan_state_changed: state: %d, size: %d", state, size);
    ctx_w()->b_scanning         = false;
    if (state == WIFI_STATE_AVALIABLE) {
        ctx_w()->scan_result_size   = size;
        if (!ctx_w()->sta_cb) {
            return;
        }
        ctx_w()->sta_cb(xf_eid, NULL, ctx_w()->sta_user_args);
    }
    return;
}

/**
 * @brief
 *
 * @param state WIFI_STATE_AVALIABLE, WIFI_STATE_NOT_AVALIABLE @see wifi_event_state_enum.
 * @param info
 * @param reason_code 对端主动断开的, bit 15置为 1 (1 << 15)
 * @see wifi_convert_disconnect_reason_code() [middleware/services/wifi_service/service/soc_wifi_service_api.c: 345]
 */
static void _sta_connection_changed_handler(
    int state, const wifi_linked_info_stru *info, int reason_code)
{
    unused(reason_code);
    if (info == NULL) {
        XF_LOGE(TAG, "param invalid");
        return;
    }

    UNUSED(state);
    XF_LOGD(TAG, "sta_conn_changed: state: %d, conn_state: %u", state, info->conn_state);

    /* xf_wifi 中没有对应状态，直接返回 */
    if (WIFI_CONNECTING == info->conn_state) {
        return;
    }

    if (!ctx_w()->sta_cb) {
        return;
    }

    xf_wifi_event_id_t xf_eid;

    xf_eid = (WIFI_CONNECTED == info->conn_state)
             ? (XF_WIFI_EVENT_STA_CONNECTED)
             : (XF_WIFI_EVENT_STA_DISCONNECTED);

    switch (xf_eid) {
    case XF_WIFI_EVENT_STA_CONNECTED: {
        ctx_w()->b_sta_connected = true;
        if (!ctx_w()->sta_cb) {
            return;
        }
        xf_wifi_event_sta_conn_t    sta_conn    = {0};
        wifi_linked_info_stru *e    = (wifi_linked_info_stru *)info;
        memcpy_s(sta_conn.ssid,     ARRAY_SIZE(sta_conn.ssid),
                 e->ssid,           ARRAY_SIZE(e->ssid));
        memcpy_s(sta_conn.bssid,    ARRAY_SIZE(sta_conn.bssid),
                 e->bssid,          ARRAY_SIZE(e->bssid));
        sta_conn.channel    = e->channel_num;
        /* 此处回调没提供 authmode 信息 */
        sta_conn.authmode   = port_convert_pf2xf_wifi_auth_mode(
                                  ctx_w()->expected_bss.security_type);
        ctx_w()->sta_cb(xf_eid, &sta_conn, ctx_w()->sta_user_args);
    } break;
    case XF_WIFI_EVENT_STA_DISCONNECTED: {
        ctx_w()->b_sta_connected = false;
        if (!ctx_w()->sta_cb) {
            return;
        }
        xf_wifi_event_sta_disconn_t sta_disconn = {0};
        wifi_linked_info_stru *e    = (wifi_linked_info_stru *)info;
        memcpy_s(sta_disconn.ssid,  ARRAY_SIZE(sta_disconn.ssid),
                 e->ssid,           ARRAY_SIZE(e->ssid));
        memcpy_s(sta_disconn.bssid, ARRAY_SIZE(sta_disconn.bssid),
                 e->bssid,          ARRAY_SIZE(e->bssid));
        sta_disconn.rssi            = e->rssi;
        /* TODO 断连原因 */
        UNUSED(reason_code);
        ctx_w()->sta_cb(xf_eid, &sta_disconn, ctx_w()->sta_user_args);
    } break;
    default:
        break;
    }
    return;
}

static void _softap_sta_join_handler(const wifi_sta_info_stru *info)
{
    if (!ctx_w()->ap_cb) {
        return;
    }

    xf_wifi_event_ap_sta_conn_t ap_sta_conn = {0};
    xf_wifi_event_id_t xf_eid;

    xf_eid = XF_WIFI_EVENT_AP_STA_CONNECTED;

    int cmp_res = 0;

    /* 首先检查 mac 是否重复，可能存在 sta 没有通知断开的情况 */
    for (int i = 0; i < (int)ARRAY_SIZE(ctx_w()->apsta_mac); i++) {
        cmp_res = osal_memcmp(ctx_w()->apsta_mac[i],
                              info->mac_addr,
                              (int)ARRAY_SIZE(ctx_w()->apsta_mac[i]));
        if (0 == cmp_res) {
            ctx_w()->arr_new_join[i] = true;
            goto l_skip_copy_mac;
        }
    }

    /* 如果 mac 没有重复则复制 mac 到列表中 */
    for (int i = 0; i < (int)ARRAY_SIZE(ctx_w()->apsta_mac); i++) {
        cmp_res = osal_memcmp(ctx_w()->apsta_mac[i],
                              sc_mac_zero,
                              (int)ARRAY_SIZE(ctx_w()->apsta_mac[i]));
        if (0 != cmp_res) {
            continue;
        }
        ctx_w()->arr_new_join[i] = true;
        memcpy_s(ctx_w()->apsta_mac[i], ARRAY_SIZE(ctx_w()->apsta_mac[i]),
                 info->mac_addr,        ARRAY_SIZE(info->mac_addr));
        break;
    }

l_skip_copy_mac:;

    memcpy_s(ap_sta_conn.mac,   ARRAY_SIZE(ap_sta_conn.mac),
             info->mac_addr,    ARRAY_SIZE(info->mac_addr));

    ctx_w()->ap_cb(xf_eid, &ap_sta_conn, ctx_w()->ap_user_args);
}

static void _softap_sta_leave_handler(const wifi_sta_info_stru *info)
{
    if (!ctx_w()->ap_cb) {
        return;
    }

    /* FIXME: 已知问题, 如果 STA 是掉电断连, 没有发出断连请求, 该回调不会触发 */

    xf_wifi_event_ap_sta_disconn_t sta_disconn = {0};
    xf_wifi_event_id_t xf_eid;

    xf_eid = XF_WIFI_EVENT_AP_STA_DISCONNECTED;

    int cmp_res = 0;
    for (int i = 0; i < (int)ARRAY_SIZE(ctx_w()->apsta_mac); i++) {
        cmp_res = osal_memcmp(ctx_w()->apsta_mac[i],
                              info->mac_addr,
                              (int)ARRAY_SIZE(info->mac_addr));
        if (0 != cmp_res) {
            continue;
        }
        memset_s(ctx_w()->apsta_mac[i],     sizeof(ctx_w()->apsta_mac[i]),
                 0,                         sizeof(ctx_w()->apsta_mac[i]));
        break;
    }

    memcpy_s(sta_disconn.mac,   ARRAY_SIZE(sta_disconn.mac),
             info->mac_addr,    ARRAY_SIZE(info->mac_addr));

    ctx_w()->ap_cb(xf_eid, &sta_disconn, ctx_w()->ap_user_args);
}

static void _softap_state_changed_handler(int state)
{
    (void)state;

    xf_wifi_event_id_t xf_eid;

    xf_eid = (WIFI_STATE_AVALIABLE == state)
             ? (XF_WIFI_EVENT_AP_START)
             : (XF_WIFI_EVENT_AP_STOP);

    ctx_w()->ap_cb(xf_eid, NULL, ctx_w()->ap_user_args);
}

static int _register_wifi_event(void)
{
    if (ctx_w()->b_register_wifi_event) {
        return XF_OK;
    }
    if (wifi_register_event_cb(&s_event_handler) != ERRCODE_SUCC) {
        XF_LOGE(TAG, "wifi_register_event_cb() failed.");
        return XF_FAIL;
    }
    ctx_w()->b_register_wifi_event = true;
    return XF_OK;
}

static void port_ap_netif_init(void)
{
    if (ctx_w()->b_ap_netif_is_inited) {
        return;
    }
    ctx_w()->b_ap_netif_is_inited = true;
    ctx_w()->netif_ap.pf_netif = netif_find(__IFNAME_AP);
}

static void port_ap_netif_deinit(void)
{
    if (!ctx_w()->b_ap_netif_is_inited) {
        return;
    }
    ctx_w()->b_ap_netif_is_inited = false;
    ctx_w()->netif_ap.pf_netif = NULL;
}

static void port_sta_netif_init(void)
{
    if (ctx_w()->b_sta_netif_is_inited) {
        return;
    }
    ctx_w()->b_sta_netif_is_inited = true;
    ctx_w()->netif_sta.pf_netif = netif_find(__IFNAME_STA);
}

static void port_sta_netif_deinit(void)
{
    if (!ctx_w()->b_sta_netif_is_inited) {
        return;
    }
    ctx_w()->b_sta_netif_is_inited = false;
    ctx_w()->netif_sta.pf_netif = NULL;
}

static xf_err_t port_convert_wifi_auth_mode(
    xf_wifi_auth_mode_t *xf, pf_wifi_auth_mode_t *pf, bool is_xf2pf)
{
    if ((NULL == xf) || (NULL == pf)) {
        return XF_FAIL;
    }
    static const port_convert_wifi_auth_mode_t port_convert_wifi_auth_mode[] = {
        { XF_WIFI_AUTH_OPEN,            WIFI_SEC_TYPE_OPEN, },
        { XF_WIFI_AUTH_WEP,             WIFI_SEC_TYPE_WEP, },
        { XF_WIFI_AUTH_WPA_PSK,         WIFI_SEC_TYPE_WPAPSK, },
        { XF_WIFI_AUTH_WPA2_PSK,        WIFI_SEC_TYPE_WPA2PSK, },
        { XF_WIFI_AUTH_WPA_WPA2_PSK,    WIFI_SEC_TYPE_WPA2_WPA_PSK_MIX, },
        { XF_WIFI_AUTH_WPA3_PSK,        WIFI_SEC_TYPE_SAE, },
        { XF_WIFI_AUTH_WPA2_WPA3_PSK,   WIFI_SEC_TYPE_WPA3_WPA2_PSK_MIX, },
        { XF_WIFI_AUTH_WAPI_PSK,        WIFI_SEC_TYPE_WAPI_PSK, },
        { XF_WIFI_AUTH_OWE,             WIFI_SEC_TYPE_OWE, },
        { XF_WIFI_AUTH_WPA3_ENT_192,    WIFI_SEC_TYPE_INVALID, },
        { XF_WIFI_AUTH_ENTERPRISE,      WIFI_SEC_TYPE_INVALID, },
        { XF_WIFI_AUTH_MAX,             WIFI_SEC_TYPE_INVALID, },
    };
#define _MODE_MAP(n) port_convert_wifi_auth_mode[n]
    if (is_xf2pf) {
        for (uint32_t i = 0; i < ARRAY_SIZE(port_convert_wifi_auth_mode); i++) {
            if (*xf == _MODE_MAP(i).xf) {
                *pf = _MODE_MAP(i).pf;
                return XF_OK;
            }
        }
        return XF_FAIL;
    }
    for (uint32_t i = 0; i < ARRAY_SIZE(port_convert_wifi_auth_mode); i++) {
        if (*pf == _MODE_MAP(i).pf) {
            *xf = _MODE_MAP(i).xf;
            return XF_OK;
        }
    }
#undef _MODE_MAP
    return XF_FAIL;
}

static pf_wifi_auth_mode_t port_convert_xf2pf_wifi_auth_mode(xf_wifi_auth_mode_t authmode)
{
    xf_err_t xf_ret = XF_OK;
    pf_wifi_auth_mode_t pf_auth_mode = 0;
    xf_ret = port_convert_wifi_auth_mode(&authmode, &pf_auth_mode, true);
    if (XF_OK == xf_ret) {
        return pf_auth_mode;
    }
    return WIFI_SEC_TYPE_INVALID;
}

static xf_wifi_auth_mode_t port_convert_pf2xf_wifi_auth_mode(pf_wifi_auth_mode_t authmode)
{
    xf_err_t xf_ret = XF_OK;
    xf_wifi_auth_mode_t xf_auth_mode = 0;
    xf_ret = port_convert_wifi_auth_mode(&xf_auth_mode, &authmode, false);
    if (XF_OK == xf_ret) {
        return xf_auth_mode;
    }
    return XF_WIFI_AUTH_MAX;
}
