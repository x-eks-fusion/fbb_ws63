/**
 * @file port_xf_netif.c
 * @author catcatBlue (catcatblue@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-06-06
 *
 * Copyright (c) 2024, CorAL. All rights reserved.
 *
 */

/*
    TODO port_xf_netif 目前不是一个单独的组件。
 */

/* ==================== [Includes] ========================================== */

#include <string.h>

#include "lwip/netifapi.h"
#include "lwip/dns.h"

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

#include "port_xf_netif.h"

#include "xf_utils.h"
#include "xf_netif.h"

/* ==================== [Defines] =========================================== */

#define PORT_XF_NETIF_NOT_SUPPORT               0

#if LWIP_NETIF_HOSTNAME
#   define PORT_XF_NETIF_HOSTNAME_SUPPORTED     1
#else
#   define PORT_XF_NETIF_HOSTNAME_SUPPORTED     0
#endif

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

static const char *TAG = "port_xf_netif";
static port_xf_netif_context_t s_port_xf_netif_context = {0};
static port_xf_netif_context_t *sp_port_xf_netif_context = &s_port_xf_netif_context;
#define ctx_n() sp_port_xf_netif_context

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

port_xf_netif_context_t *port_xf_netif_get_context(void)
{
    return sp_port_xf_netif_context;
}

#if PORT_XF_NETIF_HOSTNAME_SUPPORTED

xf_err_t xf_netif_set_hostname(
    xf_netif_t netif_hdl, const char *hostname, uint32_t len)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(NULL == hostname, XF_ERR_INVALID_ARG,
             TAG, "NULL==hostname");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    netifapi_set_hostname(p_hdl->pf_netif, hostname, len);

    return XF_OK;
}

xf_err_t xf_netif_get_hostname(
    xf_netif_t netif_hdl, char hostname[], uint32_t len)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(NULL == hostname, XF_ERR_INVALID_ARG,
             TAG, "NULL==hostname");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    netifapi_get_hostname(p_hdl->pf_netif, hostname, len);

    return XF_OK;
}

#else

xf_err_t xf_netif_set_hostname(
    xf_netif_t netif_hdl, const char *hostname, uint32_t len)
{
    UNUSED(netif_hdl);
    UNUSED(hostname);
    UNUSED(len);
    XF_LOGW(TAG, "NOT_SUPPORTED");
    return XF_ERR_NOT_SUPPORTED;
}

xf_err_t xf_netif_get_hostname(
    xf_netif_t netif_hdl, char hostname[], uint32_t len)
{
    UNUSED(netif_hdl);
    UNUSED(hostname);
    UNUSED(len);
    XF_LOGW(TAG, "NOT_SUPPORTED");
    return XF_ERR_NOT_SUPPORTED;
}

#endif /* PORT_XF_NETIF_HOSTNAME_SUPPORTED */

bool xf_netif_is_netif_up(xf_netif_t netif_hdl)
{
    XF_CHECK(NULL == netif_hdl, false,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;
    return netif_is_up(p_hdl->pf_netif);
}

xf_err_t xf_netif_get_ip_info(xf_netif_t netif_hdl, xf_netif_ip_info_t *ip_info)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(NULL == ip_info, XF_ERR_INVALID_ARG,
             TAG, "NULL==ip_info");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    /* TODO 暂未支持 ipv6 */
    ip4_addr_t st_ipaddr;
    ip4_addr_t st_gw;
    ip4_addr_t st_netmask;

    netifapi_netif_get_addr(p_hdl->pf_netif, &st_ipaddr, &st_netmask, &st_gw);

    ip_info->ip.addr        = st_ipaddr.addr;
    ip_info->netmask.addr   = st_netmask.addr;
    ip_info->gw.addr        = st_gw.addr;

    return XF_OK;
}

xf_err_t xf_netif_set_ip_info(xf_netif_t netif_hdl, const xf_netif_ip_info_t *ip_info)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(NULL == ip_info, XF_ERR_INVALID_ARG,
             TAG, "NULL==ip_info");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    /* TODO 暂未支持 ipv6 */
    ip4_addr_t st_ipaddr;
    ip4_addr_t st_gw;
    ip4_addr_t st_netmask;

    st_ipaddr.addr  = ip_info->ip.addr;
    st_gw.addr      = ip_info->netmask.addr;
    st_netmask.addr = ip_info->gw.addr;
    netifapi_netif_set_addr(p_hdl->pf_netif, &st_ipaddr, &st_netmask, &st_gw);

    return XF_OK;
}

xf_err_t xf_netif_dhcpc_start(xf_netif_t netif_hdl)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    xf_err_t xf_ret;
    err_t lwip_ret;
    lwip_ret = netifapi_dhcp_start(p_hdl->pf_netif);
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

xf_err_t xf_netif_dhcpc_stop(xf_netif_t netif_hdl)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    xf_err_t xf_ret;
    err_t lwip_ret;
    lwip_ret = netifapi_dhcp_stop(p_hdl->pf_netif);
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

xf_err_t xf_netif_dhcps_start(xf_netif_t netif_hdl)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    xf_err_t xf_ret;
    err_t lwip_ret;
    lwip_ret = netifapi_dhcps_start(p_hdl->pf_netif, NULL, 0);
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

xf_err_t xf_netif_dhcps_stop(xf_netif_t netif_hdl)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    xf_err_t xf_ret;
    err_t lwip_ret;
    lwip_ret = netifapi_dhcps_stop(p_hdl->pf_netif);
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

xf_err_t xf_netif_dhcps_get_clients_by_mac(
    xf_netif_t netif_hdl,
    xf_netif_pair_mac_ip_t mac_ip_pair_array[], uint32_t pair_array_size)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    port_xf_netif_hdl_ctx_t *p_hdl = (port_xf_netif_hdl_ctx_t *)netif_hdl;

    err_t lwip_ret;
    ip_addr_t ip_addr = {0};

    for (uint32_t i = 0; i < pair_array_size; i++) {
        lwip_ret = netifapi_dhcps_get_client_ip(
                       p_hdl->pf_netif,
                       mac_ip_pair_array[i].mac,
                       ARRAY_SIZE(mac_ip_pair_array[i].mac),
                       &ip_addr);
        if (ERR_OK == lwip_ret) {
            /* TODO 暂未支持 ipv6 */
            mac_ip_pair_array[i].ip.addr = xf_netif_htonl(ip_addr.u_addr.ip4.addr);
        } else {
            XF_LOGW(TAG, "netifapi_dhcps_get_client_ip-%d", (int)lwip_ret);
        }
        ip_addr = (ip_addr_t) {
            0
        };
    }

    return XF_OK;
}

xf_err_t xf_netif_set_dns_info(
    xf_netif_t netif_hdl, xf_netif_dns_type_t type, xf_netif_dns_info_t *dns)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(type >= XF_NETIF_DNS_MAX, XF_ERR_INVALID_ARG,
             TAG, "dns_type invalid");
    XF_CHECK(NULL == dns, XF_ERR_INVALID_ARG,
             TAG, "NULL==dns");

    xf_err_t xf_ret;

    ip_addr_t dnsserver;
    dnsserver.type = dns->ip.type;
    if (IPADDR_TYPE_V4 == dns->ip.type) {
        dnsserver.u_addr.ip4.addr = dns->ip.u_addr.ip4.addr;
    } else {
        xf_memcpy(&dnsserver.u_addr.ip6, &dns->ip.u_addr.ip6, sizeof(dnsserver.u_addr.ip6));
    }
    err_t lwip_ret;
    lwip_ret = lwip_dns_setserver(type, &dnsserver);
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

xf_err_t xf_netif_get_dns_info(
    xf_netif_t netif_hdl, xf_netif_dns_type_t type, xf_netif_dns_info_t *dns)
{
    XF_CHECK(NULL == netif_hdl, XF_ERR_INVALID_ARG,
             TAG, "NULL==netif_hdl");
    XF_CHECK(type >= XF_NETIF_DNS_MAX, XF_ERR_INVALID_ARG,
             TAG, "dns_type invalid");
    XF_CHECK(NULL == dns, XF_ERR_INVALID_ARG,
             TAG, "NULL==dns");

    xf_err_t xf_ret;

    ip_addr_t dnsserver;
    err_t lwip_ret;
    lwip_ret = lwip_dns_getserver(type, &dnsserver);
    dns->ip.type = dnsserver.type;
    if (IPADDR_TYPE_V4 == dns->ip.type) {
        dns->ip.u_addr.ip4.addr = dnsserver.u_addr.ip4.addr;
    } else {
        xf_memcpy(&dns->ip.u_addr.ip6, &dnsserver.u_addr.ip6, sizeof(dnsserver.u_addr.ip6));
    }
    xf_ret = (ERR_OK == lwip_ret) ? (XF_OK) : (XF_FAIL);

    return xf_ret;
}

/* ==================== [Static Functions] ================================== */
