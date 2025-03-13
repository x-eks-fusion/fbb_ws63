/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_SLE_ENABLE)

#include "xf_utils.h"
#include "xf_init.h"
#include "string.h"

#include "common_def.h"
#include "soc_osal.h"
#include "sle_transmition_manager.h"
#include "port_sle_transmition_manager.h"

/* ==================== [Defines] =========================================== */

#define TAG "port_sle_conn_mgr"

#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
#define PORT_SLE_LINK_MAX   8
#endif
/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
static void sle_send_data_cbk(uint16_t conn_id, sle_link_qos_state_t link_state);
static void sle_transmission_register_cbks(void);
#else
extern uint8_t gle_tx_acb_data_num_get(void);
#endif

/* ==================== [Static Variables] ================================== */

#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
static sle_link_qos_state_t s_map_sle_link_state[PORT_SLE_LINK_MAX] = {0};  /* sle link state */
#endif

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

void port_sle_flow_ctrl_init(void)
{
#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
    sle_transmission_callbacks_t trans_cbk = {0};
    trans_cbk.send_data_cb = sle_send_data_cbk;
    sle_transmission_register_callbacks(&trans_cbk);
#endif
}

uint8_t port_sle_flow_ctrl_state_get(uint16_t conn_id)
{
#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
    return (s_map_sle_link_state[conn_id] != SLE_QOS_BUSY) ? 1 : 0;
#elif (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_EXTERN_API)
    unused(conn_id);
    return gle_tx_acb_data_num_get();
#else
    return 1;
#endif
}

/* ==================== [Static Functions] ================================== */

#if (PORT_SLE_QOS_FLOWCTRL_MODE == PORT_SLE_QOS_FLOWCTRL_BY_CALLBACK)
static void sle_send_data_cbk(uint16_t conn_id, sle_link_qos_state_t link_state)
{
    s_map_sle_link_state[conn_id] = link_state;
    // printf("%s enter, gle_tx_acb_data_num_get:%d.\n", __FUNCTION__, link_state);
}

#endif

#endif  // CONFIG_XF_SLE_ENABLE
