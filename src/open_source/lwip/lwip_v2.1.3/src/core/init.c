/**
 * @file
 * Modules initialization
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 */

#include "lwip/opt.h"

#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/ip.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/igmp.h"
#include "lwip/dns.h"
#include "lwip/timeouts.h"
#include "lwip/etharp.h"
#include "lwip/ip6.h"
#include "lwip/nd6.h"
#include "lwip/mld6.h"
#include "lwip/api.h"
#if LWIP_RIPPLE
#include "lwip/lwip_rpl.h"
#endif

#ifdef LWIP_EPOLL
void EpollEventPtrInit(void);
#endif

#ifndef LWIP_SKIP_PACKING_CHECK

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct packed_struct_test {
  PACK_STRUCT_FLD_8(u8_t  dummy1);
  PACK_STRUCT_FIELD(u32_t dummy2);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif
#define PACKED_STRUCT_TEST_EXPECTED_SIZE 5

#endif

/* Compile-time sanity checks for configuration errors.
 * These can be done independently of LWIP_DEBUG, without penalty.
 */
#ifndef BYTE_ORDER
#error "BYTE_ORDER is not defined, you have to define it in your cc.h"
#endif
#if (!IP_SOF_BROADCAST && IP_SOF_BROADCAST_RECV)
#error "If you want to use broadcast filter per pcb on recv operations, you have to define IP_SOF_BROADCAST=1 in your lwipopts.h"
#endif
#if (!LWIP_UDP && LWIP_UDPLITE)
#error "If you want to use UDP Lite, you have to define LWIP_UDP=1 in your lwipopts.h"
#endif
#if (!LWIP_UDP && LWIP_DHCP)
#error "If you want to use DHCP, you have to define LWIP_UDP=1 in your lwipopts.h"
#endif
#if (!LWIP_UDP && !LWIP_RAW && LWIP_MULTICAST_TX_OPTIONS)
#error "If you want to use LWIP_MULTICAST_TX_OPTIONS, you have to define LWIP_UDP=1 and/or LWIP_RAW=1 in your lwipopts.h"
#endif
#if (!LWIP_UDP && LWIP_DNS)
#error "If you want to use DNS, you have to define LWIP_UDP=1 in your lwipopts.h"
#endif
#ifdef LWIP_SNTP
#if (!LWIP_UDP && LWIP_SNTP)
#error "If you want to use SNTP, you have to define LWIP_UDP=1 in your lwipopts.h"
#endif
#endif
#if LWIP_WND_SCALE
#if (LWIP_TCP && (TCP_WND > 0xffffffff))
#error "If you want to use TCP, TCP_WND must fit in an u32_t, so, you have to reduce it in your lwipopts.h"
#endif
#if (LWIP_TCP && (TCP_RCV_SCALE > 14))
#error "The maximum valid window scale value is 14!"
#endif
#if (LWIP_TCP && (TCP_WND > (0xFFFFU << TCP_RCV_SCALE)))
#error "TCP_WND is bigger than the configured LWIP_WND_SCALE allows!"
#endif
#if (LWIP_TCP && ((TCP_WND >> TCP_RCV_SCALE) == 0))
#error "TCP_WND is too small for the configured LWIP_WND_SCALE (results in zero window)!"
#endif
#else /* LWIP_WND_SCALE */
#if (LWIP_TCP && (TCP_WND > 0xffff))
#error "If you want to use TCP, TCP_WND must fit in an u16_t, so, you have to reduce it in your lwipopts.h (or enable window scaling)"
#endif
#endif /* LWIP_WND_SCALE */

#if (LWIP_TCP && (INITIAL_SSTHRESH < 10 || INITIAL_SSTHRESH > 2000))
#pragma message(VAR_NAME_VALUE(INITIAL_SSTHRESH))
  #error "If you want to use TCP, LWIP_CONGCNTRL_INITIAL_SSTHRESH must greater than 4 and less than 200, you can modify in lwipopts.h"
#endif

#if (LWIP_TCP && ((TCP_MAXRTX < TCP_MAXRTX_MIN_VALUE || TCP_MAXRTX > 255)))
#error "If you want to use TCP, TCP_MAXRTX must be greater than 3 and less or equal to 255,\
        so, you have to reduce them in your lwipopts.h"
#endif
#if (LWIP_TCP && (TCP_SYNMAXRTX < TCP_MAXRTX_MIN_VALUE || TCP_SYNMAXRTX > 12))
#error "If you want to use TCP, TCP_SYNMAXRTX must be greater than 3 and less or equal to 12 ,\
        so, you have to reduce them in your lwipopts.h"
#endif
#if (LWIP_TCP && ((TCP_FW1MAXRTX < TCP_MAXRTX_MIN_VALUE || TCP_FW1MAXRTX > 255)))
  #error "If you want to use TCP, TCP_FW1MAXRTX must be greater than 3 and less or equal to 255,\
          so, you have to reduce them in your lwipopts.h"
#endif
#if (LWIP_TCP && (TCP_TTL < TCP_TTL_MIN_VALUE || TCP_TTL > 255))
#error "If you want to use TCP, TCP_TTL must be greater than 8 and less or equal to 255, so, you have to reduce them in your lwipopts.h"
#endif
#if (LWIP_TCP && TCP_LISTEN_BACKLOG && ((TCP_DEFAULT_LISTEN_BACKLOG < 0) || (TCP_DEFAULT_LISTEN_BACKLOG > 0xff)))
#error "If you want to use TCP backlog, TCP_DEFAULT_LISTEN_BACKLOG must fit into an u8_t"
#endif
#if (LWIP_TCP && LWIP_TCP_ER_SUPPORT && (DUPACK_THRESH != 3))
  #error "If you want to use TCP Early Retransmit, DUPACK_THRESH must be 3"
#endif
#if (defined(LWIP_TCP_TLP_SUPPORT) && (LWIP_TCP_TLP_SUPPORT == 1) && (!defined(LWIP_SACK) || (LWIP_SACK == 0)))
  #error "If you want to use TCP support for TLP, LWIP_SACK must be enabled."
#endif
#if (LWIP_NETIF_API && (NO_SYS==1))
#error "If you want to use NETIF API, you have to define NO_SYS=0 in your lwipopts.h"
#endif
#if ((LWIP_SOCKET || LWIP_NETCONN) && (NO_SYS==1))
#error "If you want to use Sequential API, you have to define NO_SYS=0 in your lwipopts.h"
#endif
#if (!LWIP_NETCONN && LWIP_SOCKET)
  #error "If you want to use Socket API, you have to define LWIP_NETCONN=1 in your lwipopts.h"
#endif
#if (((!LWIP_DHCP) || (!LWIP_AUTOIP)) && LWIP_DHCP_AUTOIP_COOP)
#error "If you want to use DHCP/AUTOIP cooperation mode, you have to define LWIP_DHCP=1 and LWIP_AUTOIP=1 in your lwipopts.h"
#endif
#if (((!LWIP_DHCP) || (!LWIP_ARP)) && DHCP_DOES_ARP_CHECK)
#error "If you want to use DHCP ARP checking, you have to define LWIP_DHCP=1 and LWIP_ARP=1 in your lwipopts.h"
#endif
#if (!LWIP_ARP && LWIP_AUTOIP)
#error "If you want to use AUTOIP, you have to define LWIP_ARP=1 in your lwipopts.h"
#endif
#if (LWIP_TCP && ((LWIP_EVENT_API && LWIP_CALLBACK_API) || (!LWIP_EVENT_API && !LWIP_CALLBACK_API)))
#error "One and exactly one of LWIP_EVENT_API and LWIP_CALLBACK_API has to be enabled in your lwipopts.h"
#endif
#if (LWIP_ALTCP && LWIP_EVENT_API)
#error "The application layered tcp API does not work with LWIP_EVENT_API"
#endif
#if (MEM_LIBC_MALLOC && MEM_USE_POOLS)
#error "MEM_LIBC_MALLOC and MEM_USE_POOLS may not both be simultaneously enabled in your lwipopts.h"
#endif
#if (MEM_USE_POOLS && !MEMP_USE_CUSTOM_POOLS)
#error "MEM_USE_POOLS requires custom pools (MEMP_USE_CUSTOM_POOLS) to be enabled in your lwipopts.h"
#endif
#if (PBUF_POOL_BUFSIZE <= MEM_ALIGNMENT)
#error "PBUF_POOL_BUFSIZE must be greater than MEM_ALIGNMENT or the offset may take the full first pbuf"
#endif
#if (DNS_LOCAL_HOSTLIST && !DNS_LOCAL_HOSTLIST_IS_DYNAMIC && !(defined(DNS_LOCAL_HOSTLIST_INIT)))
#error "you have to define define DNS_LOCAL_HOSTLIST_INIT {{'host1', 0x123}, {'host2', 0x234}} to initialize DNS_LOCAL_HOSTLIST"
#endif
#if !LWIP_ETHERNET && (LWIP_ARP || PPPOE_SUPPORT)
#error "LWIP_ETHERNET needs to be turned on for LWIP_ARP or PPPOE_SUPPORT"
#endif
#if LWIP_IGMP && !defined(LWIP_RAND)
#error "When using IGMP, LWIP_RAND() needs to be defined to a random-function returning an u32_t random value"
#endif
#if LWIP_TCPIP_CORE_LOCKING_INPUT && !LWIP_TCPIP_CORE_LOCKING
#error "When using LWIP_TCPIP_CORE_LOCKING_INPUT, LWIP_TCPIP_CORE_LOCKING must be enabled, too"
#endif
#if LWIP_TCP && LWIP_NETIF_TX_SINGLE_PBUF && !TCP_OVERSIZE
#error "LWIP_NETIF_TX_SINGLE_PBUF needs TCP_OVERSIZE enabled to create single-pbuf TCP packets"
#endif
#if LWIP_NETCONN && LWIP_TCP
#if NETCONN_COPY != TCP_WRITE_FLAG_COPY
#error "NETCONN_COPY != TCP_WRITE_FLAG_COPY"
#endif
#if NETCONN_MORE != TCP_WRITE_FLAG_MORE
#error "NETCONN_MORE != TCP_WRITE_FLAG_MORE"
#endif
#endif /* LWIP_NETCONN && LWIP_TCP */

#if LWIP_DHCPS && !ETHARP_SUPPORT_STATIC_ENTRIES
  #error "ETHARP_SUPPORT_STATIC_ENTRIES needs to be turned on for LWIP_DHCPS"
#endif
#if LWIP_TFTP && !LWIP_UDP
  #error "LWIP_UDP needs to be turned on for LWIP_TFTP, as tftp runs over UDP socket"
#endif
#if LWIP_TFTP && !LWIP_SOCKET
  #error "LWIP_SOCKET needs to be turned on for LWIP_TFTP, as tftp runs over socket api"
#endif
#if LWIP_TFTP && !LWIP_SOCKET_SELECT
  #error "LWIP_SOCKET_SELECT needs to be turned on for LWIP_TFTP, as tftp runs over socket select api"
#endif

#if DRIVER_STATUS_CHECK && !LWIP_TCP
  #error "LWIP_TCP needs to be turned on for DRIVER_STATUS_CHECK, as DRIVER_STATUS_CHECK is base on tcp apis"
#endif
#if (DNS_TABLE_SIZE <= 0)
  #error "DNS_TABLE_SIZE should be configured as greater than zero"
#endif
#if (DNS_MAX_IPADDR <= 0)
  #error "DNS_MAX_IPADDR should be configured as greater than zero"
#endif
#if (DNS_MAX_SERVERS <= 0) || (DNS_MAX_SERVERS > 32)
  #error "DNS_MAX_SERVERS should be configured as greater than zero and less or equal to 32"
#endif

#if (DNS_MAX_NAME_LENGTH != 256)
  #error "DNS_MAX_NAME_LENGTH should be 256 as per RFC"
#endif

#if !LWIP_LITEOS_COMPAT && !LWIP_FREERTOS_COMPAT
#if LWIP_SOCKET
/* Check that the SO_* socket options and SOF_* lwIP-internal flags match */
#if SO_REUSEADDR != SOF_REUSEADDR
  #error "WARNING: SO_REUSEADDR != SOF_REUSEADDR"
#endif
#if SO_KEEPALIVE != SOF_KEEPALIVE
  #error "WARNING: SO_KEEPALIVE != SOF_KEEPALIVE"
#endif
#if SO_BROADCAST != SOF_BROADCAST
  #error "WARNING: SO_BROADCAST != SOF_BROADCAST"
#endif
#endif /* LWIP_SOCKET */
#endif /* LWIP_LITEOS_COMPAT */

/* Compile-time checks for deprecated options.
 */
#ifdef MEMP_NUM_TCPIP_MSG
#error "MEMP_NUM_TCPIP_MSG option is deprecated. Remove it from your lwipopts.h."
#endif
#ifdef TCP_REXMIT_DEBUG
#error "TCP_REXMIT_DEBUG option is deprecated. Remove it from your lwipopts.h."
#endif
#ifdef RAW_STATS
#error "RAW_STATS option is deprecated. Remove it from your lwipopts.h."
#endif
#ifdef ETHARP_QUEUE_FIRST
#error "ETHARP_QUEUE_FIRST option is deprecated. Remove it from your lwipopts.h."
#endif
#ifdef ETHARP_ALWAYS_INSERT
#error "ETHARP_ALWAYS_INSERT option is deprecated. Remove it from your lwipopts.h."
#endif
#if !NO_SYS && LWIP_TCPIP_CORE_LOCKING && LWIP_COMPAT_MUTEX && !defined(LWIP_COMPAT_MUTEX_ALLOWED)
#error "LWIP_COMPAT_MUTEX cannot prevent priority inversion. It is recommended to implement priority-aware mutexes. (Define LWIP_COMPAT_MUTEX_ALLOWED to disable this error.)"
#endif

#if (LWIP_ND6_NUM_NEIGHBORS < (LWIP_ND6_NUM_ROUTERS * 2))
  #error "neighbor cache entries must be greater than two times of router entries"
#endif

#ifndef LWIP_DISABLE_TCP_SANITY_CHECKS
#define LWIP_DISABLE_TCP_SANITY_CHECKS  0
#endif

/* TCP sanity checks */
#if !LWIP_DISABLE_TCP_SANITY_CHECKS
#if LWIP_TCP

#if TCP_MSS >= ((16 * 1024) - 1)
#error "lwip_sanity_check: WARNING: TCP_MSS must be <= 16382 to prevent u16_t underflow in TCP_SNDLOWAT calculation!"
#endif

#if TCP_SND_BUF < (2 * TCP_MSS)
#error "lwip_sanity_check: WARNING: TCP_SND_BUF must be at least as much as (2 * TCP_MSS) for things to work smoothly. If you know what you are doing, define LWIP_DISABLE_TCP_SANITY_CHECKS to 1 to disable this error."
#endif
#if !MEMP_MEM_MALLOC && PBUF_POOL_SIZE && (PBUF_POOL_BUFSIZE <= (PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN))
#error "lwip_sanity_check: WARNING: PBUF_POOL_BUFSIZE does not provide enough space for protocol headers. If you know what you are doing, define LWIP_DISABLE_TCP_SANITY_CHECKS to 1 to disable this error."
#endif
#if !TCP_PBUF_MALLOC && !MEMP_MEM_MALLOC && PBUF_POOL_SIZE && (TCP_WND > (PBUF_POOL_SIZE * (PBUF_POOL_BUFSIZE - (PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN))))
#error "lwip_sanity_check: WARNING: TCP_WND is larger than space provided by PBUF_POOL_SIZE * (PBUF_POOL_BUFSIZE - protocol headers). If you know what you are doing, define LWIP_DISABLE_TCP_SANITY_CHECKS to 1 to disable this error."
#endif
#if TCP_WND < TCP_MSS
#error "lwip_sanity_check: WARNING: TCP_WND is smaller than MSS. If you know what you are doing, define LWIP_DISABLE_TCP_SANITY_CHECKS to 1 to disable this error."
#endif
#if TCP_MAX_RTO_TICKS > 0x7FFF
  #error "MAX RTO TICKS can not be larger than s16"
#endif

#endif /* LWIP_TCP */
#endif /* !LWIP_DISABLE_TCP_SANITY_CHECKS */

/**
 * @ingroup lwip_nosys
 * Initialize all modules.
 * Use this in NO_SYS mode. Use tcpip_init() otherwise.
 */
void
lwip_init(void)
{
#ifndef LWIP_SKIP_CONST_CHECK
  int a = 0;
  LWIP_UNUSED_ARG(a);
  LWIP_ASSERT("LWIP_CONST_CAST not implemented correctly. Check your lwIP port.", LWIP_CONST_CAST(void *, &a) == &a);
#endif
#ifndef LWIP_SKIP_PACKING_CHECK
  LWIP_ASSERT("Struct packing not implemented correctly. Check your lwIP port.", sizeof(struct packed_struct_test) == PACKED_STRUCT_TEST_EXPECTED_SIZE);
#endif

  /* Modules initialization */
  stats_init();
#if !NO_SYS
  sys_init();
#endif /* !NO_SYS */
  mem_init();
  memp_init();
  pbuf_init();
  netif_init();
  (void)sock_init();
#if LWIP_IPV4
  ip_init();
#if LWIP_ARP
  etharp_init();
#endif /* LWIP_ARP */
#endif /* LWIP_IPV4 */
#if LWIP_RAW
  raw_init();
#endif /* LWIP_RAW */
#if LWIP_UDP
  udp_init();
#endif /* LWIP_UDP */
#if LWIP_TCP
  tcp_init();
#endif /* LWIP_TCP */
#if LWIP_IGMP
  igmp_init();
#endif /* LWIP_IGMP */
#if LWIP_DNS
  dns_init();
#endif /* LWIP_DNS */

#if LWIP_TIMERS
  sys_timeouts_init();
#endif /* LWIP_TIMERS */
#if LWIP_RIPPLE
  lwip_rpl_init();
#endif
#ifdef LWIP_EPOLL
  EpollEventPtrInit();
#endif
}
