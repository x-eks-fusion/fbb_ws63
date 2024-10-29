/* ==================== [Includes] ========================================== */

#include "xfusion.h"

#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "xf_task.h"
#include "xf_init.h"
#include "osal_task.h"
#include "tcxo.h"

/* ==================== [Defines] =========================================== */

#define TASKS_XF_PREMAIN_DURATION_MS        1000
#define TASKS_XF_PREMAIN_PRIO               CONFIG_TASKS_XF_PREMAIN_PRIORITY
#define TASKS_XF_PREMAIN_STACK_SIZE         CONFIG_TASKS_XF_PREMAIN_STACK_SIZE
#define CLOCK_PER_SEC  1000*1000UL

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

#define TAG "tasks_xf_entry"

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

/* ==================== [Static Functions] ================================== */

static void *tasks_xf_premain(const char *arg)
{
    unused(arg);
    xfusion_init();
    while (1)
    {
        xfusion_run();
    }
    return NULL;
}

static void tasks_xf_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)tasks_xf_premain, 0, "TasksXFPremain",
                                      TASKS_XF_PREMAIN_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, TASKS_XF_PREMAIN_PRIO);
        osal_kfree(task_handle);
    }
    osal_kthread_unlock();
}
app_run(tasks_xf_entry); /*!< Run the tasks_xf_entry. */
