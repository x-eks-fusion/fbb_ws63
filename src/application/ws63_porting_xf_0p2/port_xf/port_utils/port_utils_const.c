/**
 * @file port_utils_const.c
 * @author catcatBlue (catcatblue@qq.com)
 * @brief 
 * @version 1.0
 * @date 2024-04-18
 * 
 * Copyright (c) 2024, CorAL. All rights reserved.
 * 
 */

/* ==================== [Includes] ========================================== */

/* ==================== [Defines] =========================================== */

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

#if ENABLE_PORT_UTILS_CONST

// *INDENT-OFF*
#define PIN_PROGRAMMABLE_MAX (GPIO_18 + 1)
const map_iomux_dsize_t map_iomux[PIN_PROGRAMMABLE_MAX][PIN_MODE_MAX] = {
    [GPIO_00] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_CSN),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_01] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_02] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO3),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_03] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO1),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_04] = {
        _IO_MUX_FUNC(MUX_SSI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SSI_CLK),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO1),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_05] = {
        _IO_MUX_FUNC(MUX_SSI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SSI_DATA),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_CTS),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO2),
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_IN),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_06] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_RTS),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_SCK),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_OUT),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_07] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_RXD),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_SCK),
        _IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_MCLK),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_08] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(8),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_TXD),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_CS1_N),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_09] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(9),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_OUT),
        _IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_DO),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_10] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(10),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_CS0_N),
        _IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_SCLK),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_11] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(11),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_IN),
        _IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_LRCLK),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_12] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(12),
        _IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_DI),
        _IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_13] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(13),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_CTS),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_14] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(14),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_RTS),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_15] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(15),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_TXD),
        _IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_I2C_SDA),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_16] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(16),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_RXD),
        _IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_I2C_SCL),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_17] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(17),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_UART_TXD),
        _IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2C_SDA),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
    [GPIO_18] = {
        _IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(18),
        _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_UART_RXD),
        _IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2C_SCL),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
        _IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),
    },
};
// *INDENT-ON*

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

/* ==================== [Static Functions] ================================== */

#endif /* ENABLE_PORT_UTILS_CONST */