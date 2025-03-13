#ifndef __PORT_SLE_TRANSMITION_MANAGER_H__
#define __PORT_SLE_TRANSMITION_MANAGER_H__

/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_SLE_ENABLE)

#include "xf_utils.h"
#include "sle_ssap_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

#define PORT_SLE_QOS_FLOWCTRL_NONE              0
#define PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK       1
#define PORT_SLE_QOS_FLOWCTRL_BY_EXTERN_API     2
#define PORT_SLE_QOS_FLOWCTRL_MODE              PORT_SLE_QOS_FLOWCTRL_BY_EXTERN_API

/* ==================== [Typedefs] ========================================== */

/* ==================== [Global Prototypes] ================================= */

void port_sle_flow_ctrl_init(void);
/**
 * @brief 
 * 
 * @param conn_id 连接 ID
 * @return uint8_t 
 *      0: 当前该 ID 的链路正忙
 *      1: 表示该 ID 的链路可用（空闲或流控中）
 */
uint8_t port_sle_flow_ctrl_state_get(uint16_t conn_id);

/* ==================== [Macros] ============================================ */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CONFIG_XF_SLE_ENABLE */

#endif /* __PORT_SLE_TRANSMITION_MANAGER_H__ */
