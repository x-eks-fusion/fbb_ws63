/**
 * @file port_utils.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-10-23
 *
 * Copyright (c) 2023, CorAL. All rights reserved.
 *
 */

#ifndef __PORT_UTILS_H__
#define __PORT_UTILS_H__

/* ==================== [Includes] ========================================== */

#include "xf_utils.h"
#include "platform_def.h"

#include "pinctrl.h"
#include "platform_core_rom.h"

#include "soc_osal.h"
#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

#define INVALID_VALUE       (0xAAAAAAAAul)

/* ==================== [Typedefs] ========================================== */

/**
 * @brief 引脚及复用号结构体。
 * 对齐 1 字节用于节省空间。
 */
typedef __aligned(1) struct {
    uint8_t pin; /*!< @ref pin_t */
    uint8_t mode; /*!< @ref pin_mode_t */
} port_pin_mux_t;

/**
 * @brief 引脚查找结果
 * 对齐 1 字节用于节省空间。
 */
typedef __aligned(1) struct {
    uint8_t num; /*!< 有效结果个数 */
#define PIN_MUX_CNT_MAX (4) /*!< 同一个功能最多被复用到不同引脚的次数 */
    port_pin_mux_t pin_mux[PIN_MUX_CNT_MAX]; /*!< 查找结果 */
} port_pin_search_results_t;

typedef errcode_t pf_err_t;

/* ==================== [Global Prototypes] ================================= */

/**
 * @brief 给定引脚号及功能，获取引脚模式复用号。
 *
 * @param pin 引脚。取值如: GPIO_00/GPIO_01 等。见 @ref pin_t.
 * @param func 主功能。取值如: MUX_GPIO/MUX_I2C 等。见 @ref port_mux_enum_t.
 * @param func_num 主功能编号。
 *      取值如: 0/1/2 等。如: UART0/SPI1.
 *      对于 GPIO/DIAG/PWM/SSI 等无特殊编号的统一填 0.
 * @param func_sub_num 功能子编号。
 *      对于 GPIO/DIAG 取值如: 0/1/2 等。
 *      对于 I2S 取值如: MUX_I2S_MCLK/MUX_I2S_SCLK/MUX_I2S_LRCLK 等。见 @ref port_mux_i2s_enum_t.
 *      根据具体功能寻找 port_mux_xxx_enum_t.
 *
 * @return pin_mode_t
 *      - PIN_MODE_MAX      该引脚不支持给定功能，或参数无效
 *      - (OTHER)           引脚复用号
 *
 * @code{c}
 * pin_mode_t mode;
 * mode = port_utils_get_pin_mode_mux_num(GPIO_08, MUX_GPIO, 0, 8);
 * mode = port_utils_get_pin_mode_mux_num(GPIO_15, MUX_I2C, 1, MUX_I2C_SDA);
 * @endcode
 */
pin_mode_t port_utils_get_pin_mode_mux_num(
    pin_t pin, uint8_t func, uint8_t func_num, uint8_t func_sub_num);

/**
 * @brief 给定功能，查找能够实现该功能的引脚及对应的复用号。
 *
 * @param func 主功能。取值如: MUX_GPIO/MUX_I2C 等。见 @ref port_mux_enum_t.
 * @param func_num 主功能编号。
 *      取值如: 0/1/2 等。如: UART0/SPI1.
 *      对于 GPIO/DIAG/PWM/SSI 等无特殊编号的统一填 0.
 * @param func_sub_num 功能子编号。
 *      对于 GPIO/DIAG 取值如: 0/1/2 等。
 *      对于 I2S 取值如: MUX_I2S_MCLK/MUX_I2S_SCLK/MUX_I2S_LRCLK 等。见 @ref port_mux_i2s_enum_t.
 *      根据具体功能寻找 port_mux_xxx_enum_t.
 * @param[out] p_res 传出结果。见 @ref port_pin_search_results_t.
 *
 * @return pin_mode_t
 *      - XF_ERR_INVALID_ARG    无效参数
 *      - XF_OK                 成功
 *
 * @code{c}
 * port_pin_search_results_t res;
 * port_utils_get_pin_from_mux(MUX_SPI, 0, MUX_SPI0_IN, &res);
 * xf_printf("The number found: %u\r\n", res.num);
 * for (uint8_t i = 0; i < res.num; i++) {
 *      xf_printf("Num: %u, Pin: %u, Mode: %u\r\n",
 *                i, res.pin_mux[i].pin, res.pin_mux[i].mode);
 * }
 * @endcode
 */
xf_err_t port_utils_get_pin_from_mux(
    uint8_t func, uint8_t func_num, uint8_t func_sub_num,
    port_pin_search_results_t *p_res);

/**
 * @brief 将数组中的值转换为索引。
 *
 * @note 对于重复元素，只能查找找到的第一个元素。
 * @param map 数组中的值与索引的映射表。map 中的值除了无效值不要重复。
 * @param map_size 映射表的字节大小，通过 sizeof 获取。
 * @param element_size 映射表中每个元素的字节大小，通过 sizeof(map[0]) 获取。
 * @param val map 中的值。如 map[0]
 * @param idx 转换回的索引。
 * @return xf_err_t
 *      - XF_OK                 成功
 *      - XF_ERR_INVALID_ARG    参数错误
 *      - XF_FAIL               失败
 */
xf_err_t port_utils_value_to_idx(
    const void *map,
    uint16_t map_size, uint16_t element_size,
    uint32_t val, uint32_t *idx);

xf_err_t port_convert_pf2xf_err(pf_err_t pf_err);

/* ==================== [Macros] ============================================ */

/**
 * @brief 从表中的值找到对应的索引。
 */
#define PORT_UTILS_VALUE_2_IDX(map, val, p_idx) \
    port_utils_value_to_idx((map), sizeof(map), sizeof(map[0]), (val), (p_idx))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PORT_UTILS_H__ */
