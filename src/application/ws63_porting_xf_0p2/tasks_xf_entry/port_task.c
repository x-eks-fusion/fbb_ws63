/**
 * @file port_task.c
 * @author dotc (dotchan@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */

/* ==================== [Includes] ========================================== */

#include "xf_task.h"
#include "xf_init.h"
#include "tcxo.h"
#include "osal_task.h"
#include "soc_osal.h"

/* ==================== [Defines] =========================================== */

#define CLOCK_PER_SEC  1000UL

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

/* ==================== [Static Functions] ================================== */

static xf_task_time_t xf_task_clock(void)
{
    return (xf_task_time_t)uapi_tcxo_get_ms();
}

static void xf_task_on_idle(unsigned long int max_idle_ms)
{
    // uapi_watchdog_kick();
    osal_msleep(max_idle_ms);
}

static int port_task_init(void)
{
    xf_task_tick_init(CLOCK_PER_SEC, xf_task_clock);
    xf_task_manager_default_init(xf_task_on_idle);

    return 0;
}

XF_INIT_EXPORT_BOARD(port_task_init);
