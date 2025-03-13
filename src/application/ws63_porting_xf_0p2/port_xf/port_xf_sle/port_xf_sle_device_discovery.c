/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_SLE_ENABLE)

#include "xf_utils.h"
#include "xf_init.h"
#include "string.h"
#include "xf_heap.h"
#include "xf_sle_port_utils.h"

#include "common_def.h"
#include "soc_osal.h"

#include "xf_sle_device_discovery.h"
#include "sle_device_discovery.h"

/* ==================== [Defines] =========================================== */

#define TAG "port_sle_ssaps"

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

xf_err_t xf_sle_enable(void)
{
    osal_msleep(800);
    errcode_t ret = enable_sle();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "enable_sle failed!:%#X", ret);
    return XF_OK;
}

xf_err_t xf_sle_disable(void)
{
    errcode_t ret = disable_sle();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "disable_sle failed!:%#X", ret);
    return XF_OK;
}

// 设置本地设备地址
xf_err_t xf_sle_set_local_addr(xf_sle_addr_t *addr)
{
    sle_addr_t own_addr = {.type = addr->type};
    memcpy(own_addr.addr, addr->addr, XF_SLE_ADDR_LEN);

    errcode_t ret = sle_set_local_addr(&own_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_set_local_addr failed!:%#X", ret);
    return XF_OK;
}

// 获取本地设备地址
xf_err_t xf_sle_get_local_addr(xf_sle_addr_t *addr)
{
    sle_addr_t own_addr = {0};

    errcode_t ret = sle_get_local_addr(&own_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_get_local_addr failed!:%#X", ret);

    addr->type = own_addr.type;
    memcpy(addr->addr, own_addr.addr, XF_SLE_ADDR_LEN);
    return XF_OK;
}

// 设置本地设备名称
xf_err_t xf_sle_set_local_name(const uint8_t *name, const uint8_t len)
{
    errcode_t ret = sle_set_local_name(name, len);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_set_local_name failed!:%#X", ret);
    return XF_OK;
}

// 获取本地设备名称
xf_err_t xf_sle_get_local_name(uint8_t *name, uint8_t *len)
{
    errcode_t ret = sle_get_local_name(name, len);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_get_local_name failed!:%#X", ret);
    return XF_OK;
}

// 设置设备公开（广播）数据
xf_err_t xf_sle_set_announce_data(
    uint8_t announce_id, const xf_sle_announce_data_t *data)
{
    /* 包含广播数据包及扫描响应数据包的信息 */
    sle_announce_data_t adv_data_info = {0};  // 先清空

    uint8_t data_packed_size = 0;
    uint8_t data_packed_size_temp = 0;

    /* 解析 adv_struct_set ，设置广播数据 (adv_data) */
    data_packed_size = xf_sle_adv_data_packed_size_get(data->announce_struct_set);
    XF_LOGD(TAG, "data_packed_size (adv_data):%d", data_packed_size);
    data_packed_size_temp = data_packed_size == 0?1:data_packed_size;   // 避免 无广播数据时数组长度为 0
    uint8_t data_packed_adv[data_packed_size_temp];
    /* 有广播数据 -> 解析广播数据单元集合，填充广播数据包 */
    if(data_packed_size > 0)
    {
        memset(data_packed_adv, 0, data_packed_size);
        xf_sle_adv_data_packed_by_adv_struct_set(data_packed_adv, data->announce_struct_set);
        adv_data_info.announce_data = data_packed_adv;
    }
    adv_data_info.announce_data_len = data_packed_size;
    /* FIXME 貌似是SDK问题，广播数据或扫描响应数据指向 NULL 或长度为0时，
        广播开启失败，报 0x8000 600C 错误）*/
    if(data_packed_size == 0)
    {
        adv_data_info.announce_data_len = 1;
        adv_data_info.announce_data = data_packed_adv;
    }

    /* 解析 seek_rsp_struct_set ，设置扫描响应数据 (seek_rsp_data) */
    data_packed_size = xf_sle_adv_data_packed_size_get(data->seek_rsp_struct_set);
    XF_LOGD(TAG, "data_packed_size (seek_rsp_data):%d", data_packed_size);
    data_packed_size_temp = data_packed_size == 0?1:data_packed_size;   // 避免 无扫描响应数据时数组长度为 0
    uint8_t data_packed_seek_rsp[data_packed_size_temp];
    /* 有扫描响应数据 -> 解析扫描响应数据单元集合，填充扫描响应数据包 */
    if(data_packed_size > 0)
    {
        memset(data_packed_seek_rsp, 0, data_packed_size);
        xf_sle_adv_data_packed_by_adv_struct_set(data_packed_seek_rsp, data->seek_rsp_struct_set);
        adv_data_info.seek_rsp_data = data_packed_seek_rsp;
    }
    adv_data_info.seek_rsp_data_len = data_packed_size;
    /* FIXME 貌似是SDK问题，广播数据或扫描响应数据指向 NULL 或长度为0时，
        广播开启失败，报 0x8000 600C 错误）*/
    if(data_packed_size == 0)
    {
        adv_data_info.seek_rsp_data_len = 1;
        adv_data_info.seek_rsp_data = data_packed_seek_rsp;
    }

    /* FIXME 貌似 扫描响应数据不能生效，扫描不到 */
    errcode_t ret = sle_set_announce_data(announce_id, &adv_data_info);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_set_announce_data failed!:%#X", ret);
    return XF_OK;
}

// 设置设备公开（广播）参数
xf_err_t xf_sle_set_announce_param(
    uint8_t announce_id, const xf_sle_announce_param_t *param)
{
    sle_announce_param_t announce_param = {
        .announce_channel_map = param->announce_channel_map,
        .announce_gt_role = param->announce_gt_role,
        .announce_handle = param->announce_handle,
        .announce_interval_min = param->announce_interval_min,
        .announce_interval_max = param->announce_interval_max,
        .announce_level = param->announce_level,
        .announce_mode = param->announce_type,
        .announce_tx_power = param->announce_tx_power,
        .conn_interval_min = param->conn_interval_min,
        .conn_interval_max = param->conn_interval_max,
        .conn_max_latency = param->conn_max_latency,
        .conn_supervision_timeout = param->conn_supervision_timeout,
        .ext_param = param->ext_param,

        .own_addr.type = param->own_addr.type,
        .peer_addr.type = param->peer_addr.type,
    };

    memcpy(announce_param.own_addr.addr, param->own_addr.addr, XF_SLE_ADDR_LEN);
    memcpy(announce_param.peer_addr.addr, param->peer_addr.addr, XF_SLE_ADDR_LEN);

    XF_LOGI(TAG, "adv param: own:"XF_SLE_ADDR_PRINT_FMT" peer:" XF_SLE_ADDR_PRINT_FMT "\r\n",
            XF_SLE_ADDR_EXPAND_TO_ARG(announce_param.own_addr.addr),
            XF_SLE_ADDR_EXPAND_TO_ARG(announce_param.peer_addr.addr));
    errcode_t ret = sle_set_announce_param(announce_id, &announce_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_set_announce_param failed!:%#X", ret);
    return XF_OK;
}

// 开始设备公开（广播）
xf_err_t xf_sle_start_announce(uint8_t announce_id)
{
    errcode_t ret = sle_start_announce(announce_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_start_announce failed!:%#X", ret);
    return XF_OK;
}

// 停止设备公开（广播）
xf_err_t xf_sle_stop_announce(uint8_t announce_id)
{
    errcode_t ret = sle_stop_announce(announce_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_stop_announce failed!:%#X", ret);
    return XF_OK;
}

// 设置设备公开扫描参数
xf_err_t xf_sle_set_seek_param(xf_sle_seek_param_t *param)
{
    sle_seek_param_t seek_param = {
        .filter_duplicates = param->filter_duplicates,
        .own_addr_type = param->own_addr_type,
        .seek_filter_policy = param->seek_filter_policy,
        .seek_phys = param->seek_phy,
    };

    for (uint8_t i = 0; i < XF_SLE_SEEK_PHY_NUM_MAX; i++) {
        seek_param.seek_type[i] = param->phy_param_set[i].seek_type;
        seek_param.seek_interval[i] = param->phy_param_set[i].seek_interval;
        seek_param.seek_window[i] = param->phy_param_set[i].seek_window;
    }

    errcode_t ret = sle_set_seek_param(&seek_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_set_seek_param failed!:%#X", ret);
    return XF_OK;
}

//  开始设备公开扫描
xf_err_t xf_sle_start_seek(void)
{
    errcode_t ret = sle_start_seek();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_start_seek failed!:%#X", ret);
    return XF_OK;
}

// 停止设备公开扫描
xf_err_t xf_sle_stop_seek(void)
{
    errcode_t ret = sle_stop_seek();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "sle_stop_seek failed!:%#X", ret);
    return XF_OK;
}

/* ==================== [Static Functions] ================================== */

#endif  // CONFIG_XF_SLE_ENABLE
