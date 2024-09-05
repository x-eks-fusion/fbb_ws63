/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Tasks Sample Source. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-03, Create file. \n
 */

/* ==================== [Includes] ========================================== */

#include "xf_hal_port.h"


#include "xf_utils.h"
#include "xf_task.h"
#include "xf_init.h"
#include "xf_heap.h"

#include "xf_hw_rsrc_def.h"
#include "port_common.h"
#include "port_utils.h"
#include "port_xf_check.h"

#include "common_def.h"
#include "soc_osal.h"
#include "pinctrl.h"
#include "platform_core_rom.h"
#include "app_init.h"

#if (defined(CONFIG_XF_AT_APP_ENABLE)) && (CONFIG_XF_AT_APP_ENABLE)

#include "xf_at_port.h"

#include "version_porting.h"

#include "key_id.h"
#include "nv.h"
#include "nv_common_cfg.h"

/* ==================== [Defines] =========================================== */

#define TAG "port_xf_at"

#define DEFAULT_AT_UART_NUM        0
#define DEFAULT_AT_UART_BAUDRATE   115200
#define DEFAULT_AT_UART_TX_NUM     17
#define DEFAULT_AT_UART_RX_NUM     18

/* SDK 名称 */
// SDK名称描述分隔符
#define XF_SRC_SDK_NAME_DELIM      "_"
/* 官方名称部分 */
#define XF_SRC_SDK_NAME_PREFIX     "HI3863-AT-XXX-WS63"
#define XF_SRC_SDK_NAME_MAIN        XF_SRC_SDK_NAME_DELIM SDK_VERSION
//  自定义或更多的部分（需要增加时，请在前面加上分隔符；如无需则仅设为""即可）

// #define XF_SRC_SDK_NAME_SUFFIX_VERSION  0.07
#define XF_SRC_SDK_NAME_SUFFIX       "-V" //XF_SRC_SDK_NAME_DELIM "xfusion"
#define XF_SRC_SDK_NAME     XF_SRC_SDK_NAME_PREFIX XF_SRC_SDK_NAME_MAIN XF_SRC_SDK_NAME_SUFFIX

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

xf_err_t xf_at_port_cmd_exec(xf_at_port_cmd_exec_type_t type, xf_at_port_cmd_exec_param_t *param)
{
    switch (type)
    {
    case XF_AT_PORT_CMD_EXEC_RST:
    {
        uapi_watchdog_disable();
        /* Wait for 3000 us until the AT print is complete. */
        // call LOS_TaskDelay to wait
        hal_reboot_chip();
        return XF_OK;
    }break;
    case XF_AT_PORT_CMD_EXEC_RESTORE:
    {

    }break;
    case XF_AT_PORT_CMD_EXEC_GET_SRC_SDK_NAME:
    {
        XF_CHECK(param == NULL, XF_ERR_INVALID_ARG, TAG, "param == NULL!");
        param->get_src_sdk_name = (uint8_t *)(XF_SRC_SDK_NAME);
    }break;
    case XF_AT_PORT_CMD_EXEC_CFG_SLE_TX_PWR_LEVEL:
    {
        XF_CHECK(param == NULL, XF_ERR_INVALID_ARG, TAG, "param == NULL!");

        btc_power_type_t _btc_power_type = {
            .btc_max_txpower = param->sle_tx_pwr_level
        };
        int32_t nv_ret = uapi_nv_write(NV_ID_BTC_TXPOWER_CFG, (uint8_t *)&_btc_power_type, 
            sizeof(btc_power_type_t));
        XF_CHECK(nv_ret != ERRCODE_SUCC, XF_ERR_INVALID_ARG, TAG, "WRITE NV_ID_BTC_TXPOWER_CFG failed");
    }break;
    case XF_AT_PORT_CMD_EXEC_GET_AT_DATA_PORT_INFO_DEF:
    {
        XF_CHECK(param == NULL, XF_ERR_INVALID_ARG, TAG, "param == NULL!");
        param->at_data_port_uart.uart_num   = DEFAULT_AT_UART_NUM;
        param->at_data_port_uart.baudrate   = DEFAULT_AT_UART_BAUDRATE;
        param->at_data_port_uart.tx_num     = DEFAULT_AT_UART_TX_NUM;
        param->at_data_port_uart.rx_num     = DEFAULT_AT_UART_RX_NUM;
    }break;
    default:
        return XF_ERR_INVALID_ARG;
    }
    return XF_OK;
}

/* ==================== [Static Functions] ================================== */

#endif