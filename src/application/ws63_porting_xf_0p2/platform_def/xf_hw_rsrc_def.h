/**
 * @file xf_hw_rsrc_def.h
 * @author
 * @brief 平台硬件资源定义与转换。
 * @version 1.0
 * @date
 *
 * Copyright (c) 2023, CorAL. All rights reserved.
 *
 */

/* HW_RSRC_DEF: hardware_resource_definition */

#ifndef __XF_HW_RSRC_DEF_H__
#define __XF_HW_RSRC_DEF_H__

/* ==================== [Includes] ========================================== */

/* TODO: 不依赖头文件检测当前选择哪个芯片 */
#include "xfconfig.h"

#if defined(CONFIG_PLATFORM_WS63)
#   include "hw_rsrc_def/hw_rsrc_def_ws63.h"
#else
#   error "Please select first the target device used in your application"
#endif

/* ==================== [Defines] =========================================== */

/**
 * @addtogroup GPIO_Resource_Index
 * @{
 */
/* 见： drivers/chips/ws63/include/platform_core_rom.h */
#define IO0             0
#define IO1             1
#define IO2             2
#define IO3             3
#define IO4             4
#define IO5             5
#define IO6             6
#define IO7             7
#define IO8             8
#define IO9             9
#define IO10            10
#define IO11            11
#define IO12            12
#define IO13            13
#define IO14            14
#define IO15            15
#define IO16            16
#define IO17            17
#define IO18            18
#define IO_SFC_CLK      19
#define IO_SFC_CSN      20
#define IO_SFC_IO0      21
#define IO_SFC_IO1      22
#define IO_SFC_IO2      23
#define IO_SFC_IO3      24

#define XF_GPIO_MAX     25

/**
 * @brief 此处获取带有复用信息的 IO 号
 * 
 * 如 IO0_MUX1_PWM0/IO2_MUX3_SPI1_IO3 等。
 */
#include "hw_iomux_def.h"
/**
 * @} // GPIO_Resource_Index
 */

/**
 * @addtogroup UART_Resource_Index
 * @{
 */
/* 见：drivers/chips/ws63/include/platform_core.h (uart_bus_t) */
#define XF_UART_0       (0U)
#define XF_UART_1       (1U)
#define XF_UART_2       (2U)

#define XF_UART_MAX     (3U)
/**
 * @} // UART_Resource_Index
 */

/**
 * @addtogroup SPI_Resource_Index
 * @{
 */
/* 见：drivers/chips/ws63/include/platform_core.h (spi_bus_t) */
#define XF_SPI_0        (0U)
#define XF_SPI_1        (1U)

#define XF_SPI_MAX      (2U)
/**
 * @} // SPI_Resource_Index
 */

/**
 * @addtogroup I2C_Resource_Index
 * @{
 */
/* 见：drivers/chips/ws63/include/platform_core.h (i2c_bus_t) */
#define XF_I2C_0        (0U)
#define XF_I2C_1        (1U)

#define XF_I2C_MAX      (2U)
/**
 * @} // I2C_Resource_Index
 */

/**
 * @addtogroup ADC_Resource_Index
 * @{
 */
/* 见：drivers/chips/ws63/porting/adc/adc_porting.h (adc_channel_t) */
#define XF_ADC_0        (0U)

#define XF_ADC_MAX      (1U)

#define XF_ADC_CH_0     (0U)
#define XF_ADC_CH_1     (1U)
#define XF_ADC_CH_2     (2U)
#define XF_ADC_CH_3     (3U)
#define XF_ADC_CH_4     (4U)
#define XF_ADC_CH_5     (5U)

#define XF_ADC_CH_MAX   (6U)
/**
 * @} // ADC_Resource_Index
 */

/**
 * @addtogroup TIM_Resource_Index
 * @{
 */
/* 见： drivers/chips/ws63/rom/drivers/chips/ws63/porting/timer/timer_porting.h(timer_index_t) */
#define XF_TIM_0        (0U)
#define XF_TIM_1        (1U)
#define XF_TIM_2        (2U)

#define XF_TIM_MAX      (3U)
/**
 * @} // TIM_Resource_Index
 */

/**
 * @addtogroup PWM_Resource_Index
 * @{
 */
/* 见： drivers/chips/ws63/porting/pwm/pwm_porting.h (pwm_channel_t, pwm_v151_group_t) */
#define XF_PWM_CH0              (0U)
#define XF_PWM_CH1              (1U)
#define XF_PWM_CH2              (2U)
#define XF_PWM_CH3              (3U)
#define XF_PWM_CH4              (4U)
#define XF_PWM_CH5              (5U)

#define XF_PWM_CH_MAX           (6U)

#define XF_PWM_GROUP0           (0U)
#define XF_PWM_GROUP1           (1U)
#define XF_PWM_GROUP2           (2U)
#define XF_PWM_GROUP3           (3U)
#define XF_PWM_GROUP4           (4U)
#define XF_PWM_GROUP5           (5U)
#define XF_PWM_GROUP6           (6U)
#define XF_PWM_GROUP7           (7U)
#define XF_PWM_GROUP8           (8U)
#define XF_PWM_GROUP9           (9U)
#define XF_PWM_GROUP10          (10U)
#define XF_PWM_GROUP11          (11U)
#define XF_PWM_GROUP12          (12U)
#define XF_PWM_GROUP13          (13U)
#define XF_PWM_GROUP14          (14U)
#define XF_PWM_GROUP15          (15U)

#define XF_PWM_GROUP_MAX        (16U)

#define _PWM_CH_OFS             (0) /*!< 偏移 */
#define _PWM_CH_BITS            (4) /*!< 所占位数 */
#define _PWM_GROUP_OFS          (_PWM_CH_OFS + _PWM_CH_BITS) /*!< 偏移 */
#define _PWM_GROUP_BITS         (4) /*!< 所占位数 */

#define XF_PWM_NUM(group, channel)  \
    ((((group) & __BITS_MASK_(_PWM_GROUP_BITS)) << _PWM_GROUP_OFS) \
        | ((channel) & __BITS_MASK_(_PWM_CH_BITS)))

#define _PWM_NUM_GET_CH(pwm) ((pwm) & __BITS_MASK_(_PWM_CH_BITS))
#define _PWM_NUM_GET_GROUP(pwm) (((pwm) >> _PWM_GROUP_OFS) & __BITS_MASK_(_PWM_GROUP_BITS))

#define XF_PWM_MAX              XF_PWM_NUM(XF_PWM_CH_MAX, XF_PWM_GROUP_MAX)
/**
 * @} // PWM_Resource_Index
 */

/* ==================== [Typedefs] ========================================== */

/* ==================== [Global Prototypes] ================================= */

/* ==================== [Macros] ============================================ */

#endif /* __XF_HW_RSRC_DEF_H__ */
