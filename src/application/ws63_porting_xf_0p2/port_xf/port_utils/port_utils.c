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

/* ==================== [Static Functions] ================================== */
