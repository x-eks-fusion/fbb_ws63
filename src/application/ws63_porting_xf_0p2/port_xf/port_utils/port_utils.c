/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Tasks Sample Source. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-03, Create file. \n
 */

/* ==================== [Includes] ========================================== */

#include "port_utils.h"

#include "common_def.h"
#include "soc_osal.h"

#include "port_xf_check.h"

/* ==================== [Defines] =========================================== */

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

static const char *TAG = "port_utils";

#define ENABLE_PORT_UTILS_CONST 1
/* 获取 map_iomux */
#include "port_utils_const.c"
#undef ENABLE_PORT_UTILS_CONST

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

pin_mode_t port_utils_get_pin_mode_mux_num(
    pin_t pin, uint8_t func, uint8_t func_num, uint8_t func_sub_num)
{
    XF_CHECK(pin >= PIN_PROGRAMMABLE_MAX, PIN_MODE_MAX,
             TAG, "pin: %d invalid", pin);
    XF_CHECK(func >= MUX_MAX, PIN_MODE_MAX,
             TAG, "func: %d invalid", func);

    pin_mode_t mode = PIN_MODE_MAX;

    for (int i = 0; i < PIN_MODE_MAX; i++) {
        const map_iomux_dsize_t iomux = map_iomux[pin][i];
        if ((_IO_MUX_GET_FUNC(iomux) != func)
                || (_IO_MUX_GET_FUNCn(iomux) != func_num)
                || (_IO_MUX_GET_FUNCsn(iomux) != func_sub_num)) {
            continue;
        }
        mode = i;
        break;
    }

    return mode;
}

xf_err_t port_utils_get_pin_from_mux(
    uint8_t func, uint8_t func_num, uint8_t func_sub_num,
    port_pin_search_results_t *p_res)
{
    XF_CHECK(NULL == p_res, XF_ERR_INVALID_ARG,
             TAG, "p_res: NULL");
    XF_CHECK(func >= MUX_MAX, XF_ERR_INVALID_ARG,
             TAG, "func: %d invalid", func);

    *p_res = (port_pin_search_results_t) {
        0
    };

    for (uint32_t i = 0; i < ARRAY_SIZE(map_iomux); i++) {
        for (uint32_t j = 0; j < ARRAY_SIZE(map_iomux[0]); j++) {
            const map_iomux_dsize_t iomux = map_iomux[i][j];
            if ((_IO_MUX_GET_FUNC(iomux) != func)
                    || (_IO_MUX_GET_FUNCn(iomux) != func_num)
                    || (_IO_MUX_GET_FUNCsn(iomux) != func_sub_num)) {
                continue;
            }
            if (p_res->num >= ARRAY_SIZE(p_res->pin_mux)) {
                return XF_OK;
            }
            p_res->pin_mux[p_res->num].pin = i;
            p_res->pin_mux[p_res->num].mode = j;
            ++p_res->num;
        }
    }

    return XF_OK;
}

xf_err_t port_utils_value_to_idx(
    const void *map,
    uint16_t map_size, uint16_t element_size,
    uint32_t val, uint32_t *idx)
{
    if ((map == NULL) || (idx == NULL)
            || (map_size == 0) || (element_size == 0) || (element_size > 4)
            || (element_size > map_size)) {
        return XF_ERR_INVALID_ARG;
    }
    uint8_t equal_cnt = 0;
    for (uint16_t i = 0; i < map_size / element_size; i++) {
        equal_cnt = 0;
        for (uint16_t j = 0; j < element_size; j++) {
            /* 未使用 memcmp */
            if (*((char *)map + element_size * i + j) == *((char *)&val + j)) {
                equal_cnt++;
            } else {
                return XF_FAIL;
            }
        }
        if (equal_cnt == element_size) {
            *idx = i;
            return XF_OK;
        }
    }
    return XF_FAIL;
}

xf_err_t port_convert_pf2xf_err(errcode_t pf_err)
{
    switch (pf_err) {
    default:
    case ERRCODE_FAIL:                              return XF_FAIL;                 break;
    case ERRCODE_SUCC:                              return XF_OK;                   break;
    case ERRCODE_MALLOC:                            return XF_ERR_NO_MEM;           break;
    case ERRCODE_INVALID_PARAM:                     return XF_ERR_INVALID_ARG;      break;

    case ERRCODE_PARTITION_CONFIG_NOT_FOUND:
    case ERRCODE_IMAGE_CONFIG_NOT_FOUND:
    case ERRCODE_NV_KEY_NOT_FOUND:
    case ERRCODE_NV_PAGE_NOT_FOUND:
    case ERRCODE_DIAG_NOT_FOUND:
    case ERRCODE_DIAG_OBJ_NOT_FOUND:                return XF_ERR_NOT_FOUND;        break;

    case ERRCODE_NOT_SUPPORT:
    case ERRCODE_SFC_FLASH_NOT_SUPPORT:
    case ERRCODE_SFC_CMD_NOT_SUPPORT:
    case ERRCODE_PARTITION_NOT_SUPPORT:
    case ERRCODE_UPG_NOT_SUPPORTED:
    case ERRCODE_DIAG_NOT_SUPPORT:
    case ERRCODE_DIAG_KV_NOT_SUPPORT_ID:            return XF_ERR_NOT_SUPPORTED;    break;

    case ERRCODE_HASH_BUSY:
    case ERRCODE_DMA_CH_BUSY:
    case ERRCODE_KM_KEYSLOT_BUSY:
    case ERRCODE_CIPHER_BUSY:
    case ERRCODE_SPI_BUSY:
    case ERRCODE_SFC_DMA_BUSY:
    case ERROR_SECURITY_CHN_BUSY:
    case ERRCODE_AT_CHANNEL_BUSY:
    case ERRCODE_IPC_MAILBOX_STATUS_BUSY:
    case ERRCODE_DIAG_BUSY:                         return XF_ERR_BUSY;             break;

    case ERRCODE_HASH_TIMEOUT:
    case UART_DMA_TRANSFER_TIMEOUT:
    case ERRCODE_ADC_TIMEOUT:
    case ERRCODE_TRNG_TIMEOUT:
    case ERRCODE_PKE_TIMEOUT:
    case ERRCODE_KM_TIMEOUT:
    case ERRCODE_CIPHER_TIMEOUT:
    case ERRCODE_I2C_TIMEOUT:
    case ERRCODE_SPI_TIMEOUT:
    case ERRCODE_SFC_FLASH_TIMEOUT_WAIT_READY:
    case ERRCODE_PSRAM_RET_TIMEOUT:
    case ERRCODE_CAN_FD_SEND_TIMEOUT:
    case ERROR_SECURITY_GET_TRNG_TIMEOUT:
    case ERROR_SECURITY_HASH_CLEAR_CHN_TIMEOUT:
    case ERROR_SECURITY_HASH_CALC_TIMEOUT:
    case ERROR_SECURITY_SYMC_CLEAR_CHN_TIMEOUT:
    case ERROR_SECURITY_SYMC_CALC_TIMEOUT:
    case ERROR_SECURITY_SYMC_GET_TAG_TIMEOUT:
    case ERROR_SECURITY_PKE_LOCK_TIMEOUT:
    case ERROR_SECURITY_PKE_WAIT_DONE_TIMEOUT:
    case ERRCODE_FLASH_TIMEOUT:
    case ERRCODE_SDIO_ERR_INIT_CARD_RDY_TIMEOUT:
    case ERRCODE_SDIO_ERR_INIT_FUN1_RDY_TIMEOUT:
    case ERRCODE_EPMU_TIMEOUT:                      return XF_ERR_TIMEOUT;          break;

    case ERRCODE_PSRAM_RET_UNINIT:
    case ERRCODE_GPIO_NOT_INIT:
    case ERRCODE_CALENDAR_NOT_INIT:
    case ERRCODE_UART_NOT_INIT:
    case ERRCODE_PWM_NOT_INIT:
    case ERRCODE_DMA_NOT_INIT:
    case ERRCODE_SYSTICK_NOT_INIT:
    case ERRCODE_PIN_NOT_INIT:
    case ERRCODE_KEYSCAN_NOT_INIT:
    case ERRCODE_I2C_NOT_INIT:
    case ERRCODE_TIMER_NOT_INIT:
    case ERRCODE_SFC_NOT_INIT:
    case ERRCODE_MPU_NOT_INIT:
    case ERRCODE_TSENSOR_NOT_INIT:
    case ERRCODE_EFLASH_NOT_INIT:
    case ERRCODE_EFUSE_NOT_INIT:
    case ERRCODE_I2S_NOT_INIT:
    case ERRCODE_PCM_NOT_INIT:
    case ERRCODE_CAN_FD_NOT_INIT:
    case ERROR_SECURITY_NOT_INIT:
    case ERROR_SECURITY_PROCESS_NOT_INIT:
    case ERRCODE_UPG_NOT_INIT:
    case ERRCODE_NV_NOT_INITIALISED:
    case ERRCODE_IPC_NOT_INIT:
    case ERRCODE_SDIO_NOT_INIT:
    case ERRCODE_EDGE_NOT_INITED:
    case ERRCODE_RTC_NOT_INITED:
    case ERRCODE_UPG_FLAG_NOT_INITED:               return XF_ERR_UNINIT;           break;

    case ERRCODE_PSRAM_RET_INITED:
    case ERRCODE_CAN_FD_INITED:                     return XF_ERR_INITED;           break;
    }
}

/* ==================== [Static Functions] ================================== */
