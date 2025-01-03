/**
 * @file port_xf_ota_client.c
 * @author catcatBlue (catcatblue@qq.com)
 * @brief
 * @version 1.0
 * @date 2024-12-19
 *
 * Copyright (c) 2024, CorAL. All rights reserved.
 *
 */

/* ==================== [Includes] ========================================== */

#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"

#include "securec.h"
#include "sfc.h"
#include "partition.h"
#include "upg_porting.h"
#include "upg_common_porting.h"
#include "partition_resource_id.h"
#include "watchdog.h"

#include "xf_ota_client.h"

/* ==================== [Defines] =========================================== */

/* ==================== [Typedefs] ========================================== */

typedef struct port_xf_ota_client_ctx_s {
    uint8_t     b_inited;
    uint32_t    update_partition_size;
    uint32_t    update_package_size;
    uint32_t    write_len;
} port_xf_ota_client_ctx_t;

typedef errcode_t pf_err_t;

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

static const char *const TAG = "port_xf_ota_client";

static port_xf_ota_client_ctx_t s_port_xf_ota_client_ctx = {0};
static port_xf_ota_client_ctx_t *sp_port_xf_ota_client_ctx = &s_port_xf_ota_client_ctx;
#define ctx() sp_port_xf_ota_client_ctx

/* ==================== [Macros] ============================================ */

#if !defined(min)
#   define min(x, y)                (((x) < (y)) ? (x) : (y))
#endif

/* ==================== [Global Functions] ================================== */

xf_err_t xf_ota_init(void)
{
    pf_err_t pf_ret;
    pf_ret = uapi_partition_init();
    if (pf_ret != ERRCODE_SUCC) {
        /* TODO 是否需要检查 uapi_partition_init */
    }
    ctx()->update_partition_size = uapi_upg_get_storage_size();
    return XF_OK;
}

xf_err_t xf_ota_deinit(void)
{
    return XF_OK;
}

xf_err_t xf_ota_get_running_partition(xf_ota_partition_t *p_part_hdl)
{
    XF_CHECK(NULL == p_part_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_part_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    p_part_hdl->partition_id = XF_OTA_PARTITION_ID_OTA_0;
    p_part_hdl->is_xf_fal_part = false;
    return XF_OK;
}

xf_err_t xf_ota_get_next_update_partition(
    xf_ota_partition_t *p_start_part_hdl, xf_ota_partition_t *p_next_part_hdl)
{
    XF_CHECK(NULL == p_next_part_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_next_part_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    /* TODO 暂不支持双 OTA 分区情况 */
    UNUSED(p_start_part_hdl);
    p_next_part_hdl->partition_id = XF_OTA_PARTITION_ID_PACKAGE_STORAGE;
    p_next_part_hdl->is_xf_fal_part = false;
    return XF_OK;
}

xf_err_t xf_ota_get_partition_info(
    const xf_ota_partition_t *p_part_hdl, xf_ota_partition_info_t *p_info)
{
    xf_err_t xf_ret = XF_OK;
    XF_CHECK(NULL == p_part_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_part_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    XF_CHECK(XF_OTA_PARTITION_ID_INVALID == p_part_hdl->partition_id, XF_ERR_INVALID_ARG,
             TAG, "partition_id:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    XF_CHECK(NULL == p_info, XF_ERR_INVALID_ARG,
             TAG, "p_info:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    switch ((uintptr_t)p_part_hdl->partition_id) {
    case XF_OTA_PARTITION_ID_OTA_0: {
        xf_ret = XF_ERR_NOT_SUPPORTED;
    } break;
    case XF_OTA_PARTITION_ID_PACKAGE_STORAGE: {
        p_info->unit_size = 1;
        p_info->size = (size_t)ctx()->update_partition_size;
        xf_ret = XF_OK;
    } break;
    default:
        xf_ret = XF_FAIL;
        break;
    }

    return xf_ret;
}

size_t xf_ota_get_platform_app_desc_size(void)
{
    return 0;
}

xf_err_t xf_ota_get_platform_app_desc_block(
    xf_ota_partition_t *p_part_hdl, void *p_desc_out, size_t buff_size)
{
    UNUSED(p_part_hdl);
    UNUSED(p_desc_out);
    UNUSED(buff_size);
    return XF_ERR_NOT_SUPPORTED;
}

size_t xf_ota_get_platform_app_digest_size(void)
{
    return 0;
}

xf_err_t xf_ota_get_platform_app_digest_block(
    xf_ota_partition_t *p_part_hdl, void *p_digest_out, size_t buff_size)
{
    UNUSED(p_part_hdl);
    UNUSED(p_digest_out);
    UNUSED(buff_size);
    return XF_ERR_NOT_SUPPORTED;
}

// xf_err_t xf_ota_get_xf_app_desc(
//     xf_ota_partition_t *p_part_hdl, xf_app_desc_t *p_desc_out);

xf_err_t xf_ota_start(
    xf_ota_partition_t *p_part_hdl,
    uint32_t package_len, bool sequential_write, xf_ota_t *p_hdl)
{
    UNUSED(sequential_write);

    pf_err_t pf_ret;
    upg_prepare_info_t prepare_info = {0};
    size_t image_size = 0;

    XF_CHECK(NULL == p_part_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_part_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    XF_CHECK(0 == package_len, XF_ERR_INVALID_ARG,
             TAG, "package_len:%s", xf_err_to_name(XF_ERR_INVALID_ARG));
    XF_CHECK(NULL == p_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    ctx()->update_package_size  = package_len;
    ctx()->write_len            = 0;

    prepare_info.package_len = package_len;
    pf_ret = uapi_upg_prepare(&prepare_info);
    if (pf_ret != ERRCODE_SUCC) {
        XF_LOGD(TAG, "uapi_upg_prepare error = 0x%x", pf_ret);
        return XF_FAIL;
    }

    UNUSED(p_hdl->platform_data);

    return XF_OK;
}

xf_err_t xf_ota_abort(xf_ota_t *p_hdl)
{
    XF_CHECK(NULL == p_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    return XF_ERR_NOT_SUPPORTED;
}

xf_err_t xf_ota_end(xf_ota_t *p_hdl)
{
    XF_CHECK(NULL == p_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    return XF_OK;
}

xf_err_t xf_ota_upgrade(xf_ota_partition_t *p_part_hdl, bool reboot)
{
    pf_err_t pf_ret;
    UNUSED(p_part_hdl);
    /* TODO 暂不支持双 OTA 分区情况 */

    pf_ret = uapi_upg_request_upgrade(reboot);
    if (pf_ret != ERRCODE_SUCC) {
        XF_LOGE(TAG, "uapi_upg_request_upgrade error = 0x%x", pf_ret);
        return XF_FAIL;
    }

    return XF_OK;
}

xf_err_t xf_ota_write(xf_ota_t *p_hdl, const void *src, size_t size)
{
    pf_err_t pf_ret;

    XF_CHECK(NULL == p_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    if (ctx()->write_len + size > ctx()->update_partition_size) {
        XF_LOGE(TAG, "Too much data was written.");
        XF_LOGE(TAG, "Length already written: %d", (int)ctx()->write_len);
        XF_LOGE(TAG, "update_partition_size:  %d", (int)ctx()->update_partition_size);
        return XF_FAIL;
    }
    pf_ret = uapi_upg_write_package_sync(ctx()->write_len, (uint8_t *)src, size);
    if (pf_ret != ERRCODE_SUCC) {
        XF_LOGE(TAG, "uapi_upg_prepare error = 0x%x", pf_ret);
        return XF_FAIL;
    }
    ctx()->write_len += size;

    return XF_OK;
}

xf_err_t xf_ota_write_to(
    xf_ota_t *p_hdl, size_t dst_offset, const void *src, size_t size)

{
    XF_CHECK(NULL == p_hdl, XF_ERR_INVALID_ARG,
             TAG, "p_hdl:%s", xf_err_to_name(XF_ERR_INVALID_ARG));

    return XF_ERR_NOT_SUPPORTED;
}

/* ==================== [Static Functions] ================================== */
