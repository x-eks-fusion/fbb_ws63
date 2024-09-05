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

#ifndef __HW_IOMUX_DEF_H__
#define __HW_IOMUX_DEF_H__

/* ==================== [Includes] ========================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

/**
 * @brief 以下内容不要移动到 Typedefs.
 */

/**
 * @brief ws63 复用功能的功能编号。
 * Alternate Function
 */
typedef enum {
    MUX_DIAG = 0, /*!< == MUX_ADC */
    MUX_GPIO,
    MUX_I2C,
    MUX_I2S,
    MUX_PWM,
    MUX_SPI,
    MUX_UART,

    /* 其他不知道有没有用的 */
    // MUX_JTAG,
    MUX_SSI,
    // MUX_SFC, /*!< 内部，用户用不了 */
    // ...

    MUX_RESV, /*!< MUX_RESERVE，有复用但是未标出的用此枚举 */

    MUX_MAX, /*!< 无复用 */
} ws63_mux_enum_t;

typedef enum {
    MUX_SPI0_SCK = 0,
    MUX_SPI0_CS0_N,
    MUX_SPI0_CS1_N,
    MUX_SPI0_IN,
    MUX_SPI0_OUT,
    MUX_SPI1_SCK,
    MUX_SPI1_CSN,
    MUX_SPI1_IO0,
    MUX_SPI1_IO1,
    MUX_SPI1_IO2,
    MUX_SPI1_IO3,
    MUX_SPI_MAX,
} ws63_mux_spi_enum_t;

typedef enum {
    MUX_SSI_CLK = 0,
    MUX_SSI_DATA,
    MUX_SSI_MAX,
} ws63_mux_ssi_enum_t;

typedef enum {
    MUX_UART_RXD = 0,
    MUX_UART_TXD,
    MUX_UART_RTS,
    MUX_UART_CTS,
    MUX_UART_MAX,
} ws63_mux_uart_enum_t;

typedef enum {
    MUX_I2S_MCLK = 0,
    MUX_I2S_SCLK,
    MUX_I2S_LRCLK,
    MUX_I2S_DI,
    MUX_I2S_DO,
    MUX_I2S_MAX,
} ws63_mux_i2s_enum_t;

typedef enum {
    MUX_I2C_SCL = 0,
    MUX_I2C_SDA,
    MUX_I2C_MAX,
} ws63_mux_i2c_enum_t;

/* IO MUX 数据大小 */
#define map_iomux_dsize_t uint16_t

/*
    复用信息的位定义：
        功能(FUNC)                  功能编号(FUNCn)                 功能子编号(FUNCsn)
        bit15 ~ bit12  (4 bits)     bit11 ~ bit8  (4 bits)          bit7 ~ bit0 (8 bits)
    e.g.
        MUX_UART                     0(串口号0)                      TX/RX/CTS/RTS的编号
        MUX_GPIO                     0(无用)                         0/1/2/.../18编号
        MUX_SPI                      1(SPI号1)                       CS/DI/DO/CLK编号
*/

/* 功能子编号 */
#define _FUNCsn_OFS             (0) /*!< 偏移 */
#define _FUNCsn_BITS            (8) /*!< 所占位数 */
/* 主功能编号 */
#define _FUNCn_OFS              (_FUNCsn_BITS + _FUNCsn_OFS)
#define _FUNCn_BITS             (4)
/* 主功能 */
#define _FUNC_OFS               (_FUNCn_BITS + _FUNCn_OFS)
#define _FUNC_BITS              (4)

// STATIC_ASSERT(_FUNC_OFS + _FUNC_BITS <= (sizeof(map_iomux_dsize_t) * 8));

/* 临时宏，为了与 xf_def.h 解耦 */
#define __BIT_(n)               (1UL << (n))
#define __BITS_MASK_(n)         (((n) < 32) ? (__BIT_(n) - 1) : (~0UL))
// #include "xf_def.h" // 不需要开

#define _IO_MUX_FUNC(n)         (((n) & __BITS_MASK_(_FUNC_BITS)) << _FUNC_OFS)
#define _IO_MUX_FUNCn(n)        (((n) & __BITS_MASK_(_FUNCn_BITS)) << _FUNCn_OFS)
#define _IO_MUX_FUNCsn(n)       (((n) & __BITS_MASK_(_FUNCsn_BITS)) << _FUNCsn_OFS)

#define _IO_MUX_GET_FUNC(n)     (((n) >> _FUNC_OFS) & __BITS_MASK_(_FUNC_BITS))
#define _IO_MUX_GET_FUNCn(n)    (((n) >> _FUNCn_OFS) & __BITS_MASK_(_FUNCn_BITS))
#define _IO_MUX_GET_FUNCsn(n)   (((n) >> _FUNCsn_OFS) & __BITS_MASK_(_FUNCsn_BITS))

/* IO 号范围 GPIO_00 ~ PIN_NONE(0~25), 用 8 bits 容纳, 范围 bit7~0 */
#define _IO_NUM_OFS             (0)
#define _IO_NUM_BITS            (8)
/* IO 复用模式范围 PIN_MODE_0 ~ PIN_MODE_MAX(0~8), 用 4 bits 容纳, 范围 bit11~8 */
#define _IO_MODE_OFS            (_IO_NUM_BITS + _IO_NUM_OFS)
#define _IO_MODE_BITS           (4)
/* IO 复用模式信息范围 uint16_t, 用 16 bits 容纳, 范围 bit27~12 */
#define _MUX_INFO_OFS           (_IO_MODE_BITS + _IO_MODE_BITS)
#define _MUX_INFO_BITS          (_FUNC_OFS + _FUNC_BITS)
/* 带复用信息的 IO 标识用 4 bits 容纳, 范围 bit31~28 */
#define _IO_MUX_ID_OFS          (_MUX_INFO_OFS + _MUX_INFO_BITS)
#define _IO_MUX_ID_BITS         (4)
/* 带复用信息的 IO 标识数字 */
#define _IO_MUX_ID_MAGIC_NUM    (0x06)

/**
 * @brief 拼接复用信息、复用模式号、引脚号。
 *
 * bit7~0:      gpio 号。           如: GPIO_06.
 * bit11~8:     io 复用模式号。     如: PIN_MODE_3.
 * bit27~12:    含主功能、主功能编号、子功能编号的复用信息。
 *              如: _IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_RTS),
 * bit31~28:    _IO_MUX 标识。即 _IO_MUX_ID_MAGIC_NUM, 此处是 0x06.
 *
 * @note 该宏要求使用 32 位数存储。
 */
#define _IO_MUX(mux_info, io_mode, io_num) \
    (uint32_t)0 \
        | (((_IO_MUX_ID_MAGIC_NUM) & __BITS_MASK_(_IO_MUX_ID_BITS)) << _IO_MUX_ID_OFS) \
        | (((mux_info) & __BITS_MASK_(_MUX_INFO_BITS)) << _MUX_INFO_OFS) \
        | (((io_mode) & __BITS_MASK_(_IO_MODE_BITS)) << _IO_MODE_OFS) \
        | (((io_num) & __BITS_MASK_(_IO_NUM_BITS)) << _IO_NUM_OFS)

/**
 * @brief 从 io_mux 获取标识数字。
 *
 * @param io_mux 可能带有复用信息的 IO.
 * @return io_mux 的 bit31~28 移动到 bit0.
 */
#define _IO_MUX_GET_ID(io_mux) \
    (((io_mux) >> _IO_MUX_ID_OFS) & __BITS_MASK_(_IO_MUX_ID_BITS))

/**
 * @brief 检查 io_mux 是否带有复用信息（以 _IO_MUX_ID_MAGIC_NUM 为标识）。
 *
 * @param io_mux 可能带有复用信息的 IO.
 * @return true: 带有复用信息; false: 不带有复用信息。
 */
#define _IO_MUX_CHECK_ID(io_mux) \
    (!(_IO_MUX_GET_ID(io_mux) ^ _IO_MUX_ID_MAGIC_NUM))

/**
 * @brief 从 io_mux 获取复用信息。
 *
 * @param io_mux 带有复用信息的 IO.
 * @return io_mux 的复用信息。
 *
 * @note 可以通过 _IO_MUX_GET_FUNC/_IO_MUX_GET_FUNCn/_IO_MUX_GET_FUNCsn 获取详细信息。
 */
#define _IO_MUX_GET_MUX_INFO(io_mux) \
    (((io_mux) >> _MUX_INFO_OFS) & __BITS_MASK_(_MUX_INFO_BITS))

/**
 * @brief 从 io_mux 获取 io 模式（也就是第几个复用信号）。
 *
 * @param io_mux 带有复用信息的 IO.
 * @return io 的复用模式（如: PIN_MODE_3）。
 *
 * @note 见 @ref pin_mode_t.
 */
#define _IO_MUX_GET_IO_MODE(io_mux) \
    (((io_mux) >> _IO_MODE_OFS) & __BITS_MASK_(_IO_MODE_BITS))

/**
 * @brief 从 io_mux 获取 io 引脚号。
 *
 * @param io_mux 带有复用信息的 IO.
 * @return io 的引脚号（如: GPIO_14）。
 *
 * @note 见 @ref pin_t.
 */
#define _IO_MUX_GET_IO_NUM(io_mux) \
    (((io_mux) >> _IO_NUM_OFS) & __BITS_MASK_(_IO_NUM_BITS))

/**
 * @brief 移除引脚复用信息。
 */
#define _IO_MUX_GET_INFO_REMOVED(io) \
    (_IO_MUX_CHECK_ID(io) ? _IO_MUX_GET_IO_NUM(io) : ((pin_t)io))

/**
 * @brief 带有复用信息的 gpio 宏定义。
 */

#define IO0_MUX0_GPIO_00        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  0, 0)
#define IO0_MUX1_PWM0           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  1, 0)
#define IO0_MUX2_DIAG0          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 0)
#define IO0_MUX3_SPI1_CSN       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_CSN),       3, 0)
#define IO0_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 0)
#define IO0_MUX5_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 0)
#define IO0_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 0)
#define IO0_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 0)

#define IO1_MUX0_GPIO_01        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),                  0, 1)
#define IO1_MUX1_PWM1           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),                  1, 1)
#define IO1_MUX2_DIAG1          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),                  2, 1)
#define IO1_MUX3_SPI1_IO0       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO0),       3, 1)
#define IO1_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 1)
#define IO1_MUX5_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 1)
#define IO1_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 1)
#define IO1_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 1)

#define IO2_MUX0_GPIO_02        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),                  0, 2)
#define IO2_MUX1_PWM2           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),                  1, 2)
#define IO2_MUX2_DIAG2          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),                  2, 2)
#define IO2_MUX3_SPI1_IO3       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO3),       3, 2)
#define IO2_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 2)
#define IO2_MUX5_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 2)
#define IO2_MUX6_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 2)
#define IO2_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 2)

#define IO3_MUX0_GPIO_03        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),                  0, 3)
#define IO3_MUX1_PWM3           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),                  1, 3)
#define IO3_MUX2_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 3)
#define IO3_MUX3_SPI1_IO1       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO1),       3, 3)
#define IO3_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 3)
#define IO3_MUX5_DIAG3          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),                  5, 3)
#define IO3_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 3)
#define IO3_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 3)

#define IO4_MUX0_SSI_CLK        _IO_MUX(_IO_MUX_FUNC(MUX_SSI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SSI_CLK),        0, 4)
#define IO4_MUX1_PWM4           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),                  1, 4)
#define IO4_MUX2_GPIO_04        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),                  2, 4)
#define IO4_MUX3_SPI1_IO1       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO1),       3, 4)
#define IO4_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 4)
#define IO4_MUX5_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 4)
#define IO4_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 4)
#define IO4_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 4)

#define IO5_MUX0_SSI_DATA       _IO_MUX(_IO_MUX_FUNC(MUX_SSI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SSI_DATA),       0, 5)
#define IO5_MUX1_PWM5           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),                  1, 5)
#define IO5_MUX2_UART2_CTS      _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_CTS),       2, 5)
#define IO5_MUX3_SPI1_IO2       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_IO2),       3, 5)
#define IO5_MUX4_GPIO_05        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),                  4, 5)
#define IO5_MUX5_SPI0_IN        _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_IN),        5, 5)
#define IO5_MUX6_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 5)
#define IO5_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 5)

#define IO6_MUX0_GPIO_06        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),                  0, 6)
#define IO6_MUX1_PWM6           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),                  1, 6)
#define IO6_MUX2_UART2_RTS      _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_RTS),       2, 6)
#define IO6_MUX3_SPI1_SCK       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_SPI1_SCK),       3, 6)
#define IO6_MUX4_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 6)
#define IO6_MUX5_DIAG4          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),                  5, 6)
#define IO6_MUX6_SPI0_OUT       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_OUT),       6, 6)
#define IO6_MUX7_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 6)

#define IO7_MUX0_GPIO_07        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),                  0, 7)
#define IO7_MUX1_PWM7           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),                  1, 7)
#define IO7_MUX2_UART2_RXD      _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_RXD),       2, 7)
#define IO7_MUX3_SPI0_SCK       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_SCK),       3, 7)
#define IO7_MUX4_I2S0_MCLK      _IO_MUX(_IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_MCLK),       4, 7)
#define IO7_MUX5_DIAG5          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(5),                  5, 7)
#define IO7_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 7)
#define IO7_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 7)

#define IO8_MUX0_GPIO_08        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(8),                  0, 8)
#define IO8_MUX1_PWM0           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  1, 8)
#define IO8_MUX2_UART2_TXD      _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(2) | _IO_MUX_FUNCsn(MUX_UART_TXD),       2, 8)
#define IO8_MUX3_SPI0_CS1_N     _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_CS1_N),     3, 8)
#define IO8_MUX4_DIAG6          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(6),                  4, 8)
#define IO8_MUX5_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 8)
#define IO8_MUX6_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 8)
#define IO8_MUX7_INVALID        _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 8)

#define IO9_MUX0_GPIO_09        _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(9),                  0, 9)
#define IO9_MUX1_PWM1           _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),                  1, 9)
#define IO9_MUX2_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 9)
#define IO9_MUX3_SPI0_OUT       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_OUT),       3, 9)
#define IO9_MUX4_I2S0_DO        _IO_MUX(_IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_DO),         4, 9)
#define IO9_MUX5_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 9)
#define IO9_MUX6_DIAG7          _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),                  6, 9)
#define IO9_MUX7_RESV           _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 9)

#define IO10_MUX0_GPIO_10       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(10),                 0, 10)
#define IO10_MUX1_PWM2          _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(2),                  1, 10)
#define IO10_MUX2_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 10)
#define IO10_MUX3_SPI0_CS0_N    _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_CS0_N),     3, 10)
#define IO10_MUX4_I2S0_SCLK     _IO_MUX(_IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_SCLK),       4, 10)
#define IO10_MUX5_DIAG0         _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 10)
#define IO10_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 10)
#define IO10_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 10)

#define IO11_MUX0_GPIO_11       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(11),                 0, 11)
#define IO11_MUX1_PWM3          _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(3),                  1, 11)
#define IO11_MUX2_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 11)
#define IO11_MUX3_SPI0_IN       _IO_MUX(_IO_MUX_FUNC(MUX_SPI ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_SPI0_IN),        3, 11)
#define IO11_MUX4_I2S0_LRCLK    _IO_MUX(_IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_LRCLK),      4, 11)
#define IO11_MUX5_DIAG1         _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(1),                  5, 11)
#define IO11_MUX6_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 11)
#define IO11_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 11)

#define IO12_MUX0_GPIO_12       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(12),                 0, 12)
#define IO12_MUX1_PWM4          _IO_MUX(_IO_MUX_FUNC(MUX_PWM ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(4),                  1, 12)
#define IO12_MUX2_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 12)
#define IO12_MUX3_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 12)
#define IO12_MUX4_I2S0_DI       _IO_MUX(_IO_MUX_FUNC(MUX_I2S ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2S_DI),         4, 12)
#define IO12_MUX5_DIAG7         _IO_MUX(_IO_MUX_FUNC(MUX_DIAG) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(7),                  5, 12)
#define IO12_MUX6_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 12)
#define IO12_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 12)

#define IO13_MUX0_GPIO_13       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(13),                 0, 13)
#define IO13_MUX1_UART1_CTS     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_CTS),       1, 13)
#define IO13_MUX2_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 13)
#define IO13_MUX3_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 13)
#define IO13_MUX4_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 13)
#define IO13_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 13)
#define IO13_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 13)
#define IO13_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 13)

#define IO14_MUX0_GPIO_14       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(14),                 0, 14)
#define IO14_MUX1_UART1_RTS     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_RTS),       1, 14)
#define IO14_MUX2_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  2, 14)
#define IO14_MUX3_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 14)
#define IO14_MUX4_RESV          _IO_MUX(_IO_MUX_FUNC(MUX_RESV) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 14)
#define IO14_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 14)
#define IO14_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 14)
#define IO14_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 14)

#define IO15_MUX0_GPIO_15       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(15),                 0, 15)
#define IO15_MUX1_UART1_TXD     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_TXD),       1, 15)
#define IO15_MUX2_I2C1_SDA      _IO_MUX(_IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_I2C_SDA),        2, 15)
#define IO15_MUX3_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 15)
#define IO15_MUX4_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 15)
#define IO15_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 15)
#define IO15_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 15)
#define IO15_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 15)

#define IO16_MUX0_GPIO_16       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(16),                 0, 16)
#define IO16_MUX1_UART1_RXD     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_UART_RXD),       1, 16)
#define IO16_MUX2_I2C1_SCL      _IO_MUX(_IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(1) | _IO_MUX_FUNCsn(MUX_I2C_SCL),        2, 16)
#define IO16_MUX3_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 16)
#define IO16_MUX4_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 16)
#define IO16_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 16)
#define IO16_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 16)
#define IO16_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 16)

#define IO17_MUX0_GPIO_17       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(17),                 0, 17)
#define IO17_MUX1_UART0_TXD     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_UART_TXD),       1, 17)
#define IO17_MUX2_I2C0_SDA      _IO_MUX(_IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2C_SDA),        2, 17)
#define IO17_MUX3_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 17)
#define IO17_MUX4_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 17)
#define IO17_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 17)
#define IO17_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 17)
#define IO17_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 17)

#define IO18_MUX0_GPIO_18       _IO_MUX(_IO_MUX_FUNC(MUX_GPIO) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(18),                 0, 18)
#define IO18_MUX1_UART0_RXD     _IO_MUX(_IO_MUX_FUNC(MUX_UART) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_UART_RXD),       1, 18)
#define IO18_MUX2_I2C0_SCL      _IO_MUX(_IO_MUX_FUNC(MUX_I2C ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(MUX_I2C_SCL),        2, 18)
#define IO18_MUX3_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  3, 18)
#define IO18_MUX4_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  4, 18)
#define IO18_MUX5_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  5, 18)
#define IO18_MUX6_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  6, 18)
#define IO18_MUX7_INVALID       _IO_MUX(_IO_MUX_FUNC(MUX_MAX ) | _IO_MUX_FUNCn(0) | _IO_MUX_FUNCsn(0),                  7, 18)

/* ==================== [Typedefs] ========================================== */

/* ==================== [Global Prototypes] ================================= */

/* ==================== [Macros] ============================================ */

// 不能 undef
// #undef __BIT_
// #undef __BITS_MASK_

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HW_IOMUX_DEF_H__ */
