/**
 * @file platform_port_def.h
 * @author
 * @brief 平台移植相关定义。
 * @version 1.0
 * @date
 *
 * Copyright (c) 2023, CorAL. All rights reserved.
 *
 */

#ifndef __PLATFORM_PORT_DEF_H__
#define __PLATFORM_PORT_DEF_H__

/* ==================== [Includes] ========================================== */

#include "xf_hw_rsrc_def.h"

/* ==================== [Defines] =========================================== */

/* FIXME: 调试串口引脚与编号，与具体硬件相关，不应该放这里 */
#define XF_UART_DEBUG_NUM       XF_UART_0 /*!< UART_NUM_0 */
#define XF_UART_DEBUG_TX_PIN    IO1
#define XF_UART_DEBUG_RX_PIN    IO3

/* ==================== [Typedefs] ========================================== */

/**
 * @brief 定时器。
 */
/* 定时器 源 的索引 */
typedef enum _port_timer_src_t {
#if defined(XF_TIM_0)
    XF_TIM_SRC_HW_0 = XF_TIM_0,
#endif
#if defined(XF_TIM_1)
    XF_TIM_SRC_HW_1 = XF_TIM_1,
#endif
#if defined(XF_TIM_2)
    XF_TIM_SRC_HW_2 = XF_TIM_2,
#endif
#if defined(XF_TIM_3)
    XF_TIM_SRC_HW_3 = XF_TIM_3,
#endif
#if defined(XF_TIM_4)
    XF_TIM_SRC_HW_4 = XF_TIM_4,
#endif
#if defined(XF_TIM_5)
    XF_TIM_SRC_HW_5 = XF_TIM_5,
#endif
#if defined(XF_TIM_6)
    XF_TIM_SRC_HW_6 = XF_TIM_6,
#endif
#if defined(XF_TIM_7)
    XF_TIM_SRC_HW_7 = XF_TIM_7,
#endif
    XF_TIM_SRC_CUSTOM,
    XF_TIM_SRC_MAX = XF_TIM_SRC_CUSTOM + 1,
    XF_TIM_SRC_BASE_INDEX = XF_TIM_0,
} port_timer_src_t;

/**
 * @brief tick。
 */
typedef enum _xf_systime_t {
#if defined(XF_TIM_0)
    XF_SYSTIME_SRC_0 = XF_TIM_0,
#endif
#if defined(XF_TIM_1)
    XF_SYSTIME_SRC_1 = XF_TIM_1,
#endif
#if defined(XF_TIM_2)
    XF_SYSTIME_SRC_2 = XF_TIM_2,
#endif
#if defined(XF_TIM_3)
    XF_SYSTIME_SRC_3 = XF_TIM_3,
#endif
#if defined(XF_TIM_4)
    XF_SYSTIME_SRC_4 = XF_TIM_4,
#endif
#if defined(XF_TIM_5)
    XF_SYSTIME_SRC_5 = XF_TIM_5,
#endif
#if defined(XF_TIM_6)
    XF_SYSTIME_SRC_6 = XF_TIM_6,
#endif
#if defined(XF_TIM_7)
    XF_SYSTIME_SRC_7 = XF_TIM_7,
#endif
    XF_SYSTIME_SRC_CORE, /*!< 使用 systick */
    XF_SYSTIME_SRC_MAX = XF_SYSTIME_SRC_CORE + 1,
} port_tick_t;

/* ISR 类型 */
typedef enum _port_isr_type_t {
    PORT_ISR_TYPE_BASE_INDEX = 0,

    PORT_ISR_TYPE_SYSTICK = 0,

    PORT_ISR_TYPE_TIM1,
    PORT_ISR_TYPE_TIM2,
    PORT_ISR_TYPE_TIM3,
    PORT_ISR_TYPE_TIM4,

    PORT_ISR_TYPE_I2C1_ERR,
    PORT_ISR_TYPE_I2C1_EV,
    PORT_ISR_TYPE_I2C2_EV,
    PORT_ISR_TYPE_I2C2_ERR,

    PORT_ISR_TYPE_MAX,
} port_isr_type_t;

/* ==================== [Global Prototypes] ================================= */

/* ==================== [Macros] ============================================ */

#endif /* __PLATFORM_PORT_DEF_H__ */
