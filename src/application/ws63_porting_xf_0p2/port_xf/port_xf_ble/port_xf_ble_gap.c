/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_BLE_ENABLE)

#include "xf_utils.h"
#include "xf_ble_gap.h"
#include "xf_init.h"
#include "xf_heap.h"
#include "xf_ble_port_utils.h"

#include "port_utils.h"

#include "common_def.h"
#include "soc_osal.h"

#include "bts_le_gap.h"

/* ==================== [Defines] =========================================== */

#define TAG "port_ble_gap"

#define DEFAULT_BLE_ADV_MAX_CNT     (8)

/* ==================== [Typedefs] ========================================== */

typedef struct _port_ble_adv_t
{
    xf_ble_adv_id_t adv_id;
    gap_ble_adv_params_t param;
} port_ble_adv_t;

/* ==================== [Static Prototypes] ================================= */

static void port_ble_enable_cb(errcode_t status);
static void ble_gap_terminate_adv_cb(
    uint8_t adv_id, adv_status_t status);
static void port_ble_gap_connect_change_cb(uint16_t conn_id, bd_addr_t *addr,
        gap_ble_conn_state_t conn_state,
        gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason);
static void port_ble_paired_complete_cb(
    uint16_t conn_id, const bd_addr_t *addr, errcode_t status);
static void port_ble_gap_connect_param_update_cb(uint16_t conn_id, errcode_t status,
        const gap_ble_conn_param_update_t *param);
static void port_ble_gap_adv_enable_cb(uint8_t adv_id, adv_status_t status);
static void port_ble_gap_adv_disable_cb(uint8_t adv_id, adv_status_t status);
static void port_ble_gap_scan_result_cb(gap_scan_result_data_t *scan_result_data);
static xf_ble_evt_res_t _port_ble_gap_evt_cb_default(
    xf_ble_gap_evt_t event, xf_ble_gap_evt_cb_param_t *param);

static port_ble_adv_t *_port_ble_adv_alloc(void);
static xf_err_t *_port_ble_adv_free(port_ble_adv_t *adv);
static xf_err_t *_port_ble_adv_free_by_id(xf_ble_adv_id_t adv_id);
static port_ble_adv_t *_port_ble_adv_get_by_id(xf_ble_adv_id_t adv_id);
static inline errcode_t _port_ble_gap_set_adv_data(
    uint8_t adv_id, const xf_ble_gap_adv_data_t *data);

/* ==================== [Static Variables] ================================== */

static xf_ble_gap_evt_cb_t s_ble_gap_evt_cb = _port_ble_gap_evt_cb_default;
static xf_ble_appearance_t s_appearance = 0XFFFF;
static port_ble_adv_t *(s_list_adv[DEFAULT_BLE_ADV_MAX_CNT]) = {0};

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

// 使能 BLE 协议栈
xf_err_t xf_ble_enable(void)
{
    osal_msleep(1000);
    errcode_t ret = enable_ble();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "enable_ble failed!:%#X", ret);
    return XF_OK;
}

// 失能 BLE 协议栈
xf_err_t xf_ble_disable(void)
{
    errcode_t ret = disable_ble();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "disable_ble failed!:%#X", ret);
    return XF_OK;
}

// 设置本地设备地址
xf_err_t xf_ble_gap_set_local_addr(xf_ble_addr_t *addr)
{
    XF_CHECK(addr == NULL, XF_ERR_INVALID_ARG,
             TAG, "xf_ble_gap_set_local_addr failed!:addr == NULL");

    bd_addr_t dev_addr = {.type = addr->type};
    memcpy(dev_addr.addr, addr->addr, XF_BLE_ADDR_LEN);

    errcode_t ret = gap_ble_set_local_addr(&dev_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_set_local_addr failed!:%#X", ret);
    return XF_OK;
}

// 获取本地设备地址
xf_err_t xf_ble_gap_get_local_addr(xf_ble_addr_t *addr)
{
    XF_CHECK(addr == NULL, XF_ERR_INVALID_ARG,
             TAG, "xf_ble_gap_get_local_addr failed!:addr == NULL");

    bd_addr_t dev_addr = {0};
    errcode_t ret = gap_ble_get_local_addr(&dev_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_get_local_addr failed!:%#X", ret);

    memcpy(addr->addr, dev_addr.addr, XF_BLE_ADDR_LEN);
    addr->type = dev_addr.type;
    return XF_OK;
}

xf_err_t xf_ble_gap_set_local_appearance(
    xf_ble_appearance_t appearance)
{
    errcode_t ret = gap_ble_set_local_appearance(appearance);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_set_local_appearance failed!:%#X", ret);
    s_appearance = appearance;
    return XF_OK;
}

// 获取本地设备的外观
xf_err_t xf_ble_gap_get_local_appearance(xf_ble_appearance_t *appearance)
{
    *appearance = s_appearance;
    return XF_OK;
}

// 设置本地设备名称 （ IDF 只有 name 参数，然后内部 strlen 获取长度）
xf_err_t xf_ble_gap_set_local_name(
    const uint8_t *name, const uint8_t len)
{
    errcode_t ret = gap_ble_set_local_name(name, len);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_set_local_name failed!:%#X", ret);
    return XF_OK;
}

// 获取本地设备名称
xf_err_t xf_ble_gap_get_local_name(
    uint8_t *name, uint8_t *len)
{
    errcode_t ret = gap_ble_get_local_name(name, len);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_get_local_name failed!:%#X", ret);
    return XF_OK;
}

/* 创建广播 */
xf_err_t xf_ble_gap_create_adv(
    xf_ble_adv_id_t *adv_id,
    const xf_ble_gap_adv_param_t *param,
    const xf_ble_gap_adv_data_t *data)
{
    port_ble_adv_t *adv = _port_ble_adv_alloc();
    XF_CHECK(adv == NULL, XF_ERR_RESOURCE, TAG, "adv alloc failed");
    
    errcode_t ret = ERRCODE_SUCC;

    /* 设置广播数据 */
    ret = _port_ble_gap_set_adv_data(adv->adv_id, data);
    XF_CHECK_GOTO(ret != ERRCODE_SUCC, _TAG_ADV_EXEC_ERROR,
             TAG, "set adv data failed!:%#X", ret);

    /* 缓存广播参数 */
    adv->param = (gap_ble_adv_params_t){
        .min_interval = param->min_interval,
        .max_interval = param->max_interval,
        .adv_type = param->type,
        .own_addr.type = param->own_addr.type,
        .peer_addr.type =  param->peer_addr.type,
        .adv_filter_policy = param->filter_policy,
        .channel_map = param->channel_map,
        .tx_power = param->tx_power,
        .duration = 0,
    };

    memcpy(adv->param.own_addr.addr, param->own_addr.addr, XF_BLE_ADDR_LEN);
    memcpy(adv->param.peer_addr.addr, param->peer_addr.addr, XF_BLE_ADDR_LEN);

    *adv_id = adv->adv_id;
    return XF_OK;

_TAG_ADV_EXEC_ERROR:;
    _port_ble_adv_free(adv);
    return ret;            
}

/* 广播销毁 */
xf_err_t xf_ble_gap_delete_adv(xf_ble_adv_id_t adv_id)
{
    XF_ASSERT(adv_id != XF_BLE_ADV_ID_INVALID, XF_ERR_INVALID_ARG, 
        TAG, "adv id is invalid");
    XF_ASSERT(adv_id < DEFAULT_BLE_ADV_MAX_CNT, XF_ERR_INVALID_ARG, 
        TAG, "adv cnt(%d) >= max(%d)", adv_id, DEFAULT_BLE_ADV_MAX_CNT);

    port_ble_adv_t *adv = _port_ble_adv_get_by_id(adv_id);
    XF_CHECK(adv == NULL, XF_ERR_INVALID_ARG,
        TAG, "adv(id:%d) not found", adv_id);
    
    return _port_ble_adv_free_by_id(adv_id);
}

/* 开启广播 */
xf_err_t xf_ble_gap_start_adv(xf_ble_adv_id_t adv_id, uint16_t duration)
{
    XF_ASSERT(adv_id != XF_BLE_ADV_ID_INVALID, XF_ERR_INVALID_ARG, 
        TAG, "adv id is invalid");
    XF_ASSERT(adv_id < DEFAULT_BLE_ADV_MAX_CNT, XF_ERR_INVALID_ARG, 
        TAG, "adv cnt(%d) >= max(%d)", adv_id, DEFAULT_BLE_ADV_MAX_CNT);
    port_ble_adv_t *adv = _port_ble_adv_get_by_id(adv_id);
    XF_CHECK(adv == NULL, XF_ERR_INVALID_ARG,
        TAG, "adv(id:%d) not found", adv_id);

    errcode_t ret = ERRCODE_SUCC;

    adv->param.duration = duration;
    /* 设置广播数据 */
    ret = gap_ble_set_adv_param(adv->adv_id, &adv->param);
    XF_CHECK(ret != ERRCODE_SUCC, XF_ERR_INVALID_STATE,
             TAG, "set adv param failed!:%#X", ret);
             
    /* 开广播 */
    ret = gap_ble_start_adv(adv->adv_id);
    XF_CHECK(ret != ERRCODE_SUCC, XF_ERR_INVALID_STATE,
             TAG, "start adv failed!:%#X", ret);
    return XF_OK;
}

/* 停止广播 */
xf_err_t xf_ble_gap_stop_adv(xf_ble_adv_id_t adv_id)
{
    XF_ASSERT(adv_id != XF_BLE_ADV_ID_INVALID, XF_ERR_INVALID_ARG, 
        TAG, "adv id is invalid");
    XF_ASSERT(adv_id < DEFAULT_BLE_ADV_MAX_CNT, XF_ERR_INVALID_ARG, 
        TAG, "adv cnt(%d) >= max(%d)", adv_id, DEFAULT_BLE_ADV_MAX_CNT);

    port_ble_adv_t *adv = _port_ble_adv_get_by_id(adv_id);
    XF_CHECK(adv == NULL, XF_ERR_INVALID_ARG,
        TAG, "adv(id:%d) not found", adv_id);

     errcode_t ret = gap_ble_stop_adv(adv->adv_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
            TAG, "stop adv failed!:%#X", ret);
    return XF_OK;
}

/* 设置广播数据 */
xf_err_t xf_ble_gap_set_adv_data(
    xf_ble_adv_id_t adv_id, const xf_ble_gap_adv_data_t *data)
{
    XF_ASSERT(adv_id != XF_BLE_ADV_ID_INVALID, XF_ERR_INVALID_ARG, 
        TAG, "adv id is invalid");
    XF_ASSERT(adv_id < DEFAULT_BLE_ADV_MAX_CNT, XF_ERR_INVALID_ARG, 
        TAG, "adv cnt(%d) >= max(%d)", adv_id, DEFAULT_BLE_ADV_MAX_CNT);

    port_ble_adv_t *adv = _port_ble_adv_get_by_id(adv_id);
    XF_CHECK(adv == NULL, XF_ERR_INVALID_ARG,
        TAG, "adv(id:%d) not found", adv_id);

    errcode_t ret = _port_ble_gap_set_adv_data(adv->adv_id, data);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
            TAG, "set adv data failed!:%#X", ret);
    return XF_OK;
}

/* 开启扫描 */
xf_err_t xf_ble_gap_start_scan(const xf_ble_gap_scan_param_t *param)
{
    gap_ble_scan_params_t scan_param = {
        .scan_type = param->type,
        .scan_interval = param->interval,
        .scan_window = param->window,
        .scan_phy = param->phy,
        .scan_filter_policy = param->filter_policy,
    };
    errcode_t ret = gap_ble_set_scan_parameters(&scan_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_set_scan_parameters failed!:%#X", ret);
    
    ret = gap_ble_start_scan();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_start_scan failed!:%#X", ret);
    return XF_OK;
}

/* 停止扫描 */
xf_err_t xf_ble_gap_stop_scan(void)
{
    errcode_t ret = gap_ble_stop_scan();
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_stop_scan failed!:%#X", ret);
    return XF_OK;
}

/* 更新连接参数 */
xf_err_t xf_ble_gap_update_conn_param(
    xf_ble_conn_id_t conn_id, xf_ble_gap_conn_param_update_t *param)
{
    gap_conn_param_update_t update_conn_param = {
        .conn_handle = conn_id,
        .interval_min = param->min_interval,
        .interval_max = param->max_interval,
        .slave_latency = param->latency,
        .timeout_multiplier = param->timeout,
    };
    errcode_t ret = gap_ble_connect_param_update(&update_conn_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_connect_param_update failed!:%#X", ret);
    return XF_OK;
}

/* 与设备建立（ACL）连接 */
xf_err_t xf_ble_gap_connect(
    const xf_ble_addr_t *addr)
{
    bd_addr_t dev_addr = {.type = addr->type};
    memcpy(dev_addr.addr, addr->addr, XF_BLE_ADDR_LEN);

    errcode_t ret = gap_ble_connect_remote_device(&dev_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_connect_remote_device failed!:%#X", ret);
    return XF_OK;
}

/* 与设备断开连接 */
xf_err_t xf_ble_gap_disconnect(
    const xf_ble_addr_t *addr)
{
    bd_addr_t dev_addr = {.type = addr->type};
    memcpy(dev_addr.addr, addr->addr, XF_BLE_ADDR_LEN);

    errcode_t ret = gap_ble_disconnect_remote_device(&dev_addr);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_disconnect_remote_device failed!:%#X", ret);
    return XF_OK;
}

/* 配对 */
// 发起/启动配对
xf_err_t xf_ble_gap_add_pair(const xf_ble_addr_t *addr)
{
    bd_addr_t dev_addr = {.type = addr->type};
    memcpy(dev_addr.addr, addr->addr, XF_BLE_ADDR_LEN);

    errcode_t ret = gap_ble_pair_remote_device(&dev_addr);
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_pair_remote_device failed!:%#X", ret);
    return XF_OK;
}

// 取消指定设备的配对
xf_err_t xf_ble_gap_del_pair(const xf_ble_addr_t *addr)
{
    bd_addr_t dev_addr = {.type = addr->type};
    memcpy(dev_addr.addr, addr->addr, XF_BLE_ADDR_LEN);

    errcode_t ret = gap_ble_remove_pair(&dev_addr);
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_remove_pair failed!:%#X", ret);
    return XF_OK;
}

// 取消所有设备的配对
xf_err_t xf_ble_gap_del_pair_all(void)
{
    errcode_t ret = gap_ble_remove_all_pairs();
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_remove_all_pairs failed!:%#X", ret);
    return XF_OK;
}

// 获取已配对的设备
xf_err_t xf_ble_gap_get_pair_list(
    uint16_t *max_num, xf_ble_addr_t *dev_list)
{
    bd_addr_t *ws63_dev_list = (bd_addr_t *)xf_malloc(sizeof(bd_addr_t)*(*max_num));
    XF_CHECK( ws63_dev_list == NULL, XF_ERR_NO_MEM, 
        TAG, "ws63_dev_list xf_malloc failed!");
    
    errcode_t ret = gap_ble_get_paired_devices(ws63_dev_list, max_num);
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_get_paired_devices failed!:%#X", ret);
    uint16_t num_get = *max_num;
    while (num_get--)
    {
        dev_list[num_get].type = ws63_dev_list[num_get].type;
        memcpy(dev_list[num_get].addr,
            ws63_dev_list[num_get].addr, XF_BLE_ADDR_LEN);
    }
    xf_free(ws63_dev_list);
    return XF_OK;
}

// 获取已绑定的设备
xf_err_t xf_ble_gap_get_bond_list(
    int *max_num, xf_ble_addr_t *dev_list)
{
    bd_addr_t *ws63_dev_list = (bd_addr_t *)xf_malloc(sizeof(bd_addr_t)*(*max_num));
    XF_CHECK( ws63_dev_list == NULL, XF_ERR_NO_MEM, 
        TAG, "ws63_dev_list xf_malloc failed!");

    errcode_t ret = gap_ble_get_bonded_devices(ws63_dev_list, max_num);
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_get_bonded_devices failed!:%#X", ret);
    uint16_t num_get = *max_num;
    while (num_get--)
    {
        dev_list[num_get].type = ws63_dev_list[num_get].type;
        memcpy(dev_list[num_get].addr,
            ws63_dev_list[num_get].addr, XF_BLE_ADDR_LEN);
    }
    xf_free(ws63_dev_list);
    return XF_OK;
}

xf_err_t xf_ble_gap_set_pair_feature(
    xf_ble_conn_id_t conn_id, xf_ble_sm_pair_feature_t type,
    xf_ble_sm_pair_feature_param_t feature)
{
    union _xf_ble_sm_pair_feature_param_t feat = {._untype = feature};
    /*因为没有对接的获取函数，且多类参数只有一个 api ，所以需要缓存着 */
    static gap_ble_sec_params_t s_params = {0};
    switch (type)
    {
    case XF_BLE_SM_PAIR_FEATURE_AUTHEN_RQMT:
    {
        if((feat.authen_rqmt & XF_BLE_SM_AUTHEN_RQMT_BOND)
            == true)
        {
            s_params.bondable = true;
        }
        if((feat.authen_rqmt & XF_BLE_SM_AUTHEN_RQMT_SC)
            == true)
        {
            s_params.sc_enable = true;
        }
    }
    break;
    case XF_BLE_SM_PAIR_FEATURE_IO_CAP:
    {
        /* 此处两边都是按标准的进行定义，所以直接设置 */
        s_params.io_capability = feat.io_cap;
    }
    break;
    case XF_BLE_SM_PAIR_FEATURE_SECTY_RQMT:
    {
        s_params.sc_mode = feat.secty_rqmt;
    }
    break;
    default:
        break;
    }
    errcode_t ret = gap_ble_set_sec_param(&s_params);
    XF_CHECK( ret != ERRCODE_SUCC, (xf_err_t)ret, 
        TAG, "gap_ble_set_sec_param failed!:%#X", ret);
    return XF_OK;
}

xf_err_t xf_ble_gap_respond_pair(xf_ble_conn_id_t conn_id)
{
    /* FIXME 本平台上貌似无对应的操作 */
    return XF_OK;
}

xf_err_t xf_ble_gap_pair_exchange_passkey(
    xf_ble_conn_id_t conn_id, uint32_t pin_code)
{
    /* FIXME 本平台上貌似无对应的操作 */
    return XF_OK;
}

// GAP 回调注册
xf_err_t xf_ble_gap_event_cb_register(
    xf_ble_gap_evt_cb_t evt_cb,
    xf_ble_gap_evt_t events)
{
    unused(events);
    errcode_t ret = ERRCODE_SUCC;
    s_ble_gap_evt_cb = evt_cb;

    gap_ble_callbacks_t gap_cb_set = {
        .ble_enable_cb = port_ble_enable_cb,
        .conn_state_change_cb = port_ble_gap_connect_change_cb,
        .pair_result_cb = port_ble_paired_complete_cb,
        .conn_param_update_cb = port_ble_gap_connect_param_update_cb,
        .start_adv_cb = port_ble_gap_adv_enable_cb,
        .stop_adv_cb = port_ble_gap_adv_disable_cb,
        .terminate_adv_cb = ble_gap_terminate_adv_cb,
        .scan_result_cb = port_ble_gap_scan_result_cb,
    };
    ret = gap_ble_register_callbacks(&gap_cb_set);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gap_ble_register_callbacks failed!:%#X", ret);
    return XF_OK;
}

/* ==================== [Static Functions] ================================== */

static void port_ble_enable_cb(errcode_t status)
{
    XF_LOGD(TAG, "enable cb:status: %d\n", status);
}

static void ble_gap_terminate_adv_cb(
    uint8_t adv_id, adv_status_t status)
{
    unused(adv_id);
    XF_LOGD(TAG, "ble_gap_terminate_adv_cb status: %d\n", status);
}

static void port_ble_gap_connect_change_cb(uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
        gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason)
{
    XF_LOGD(TAG, "connect state change cb:conn_id: %d,"
            "status: %d,pair_status:%d,addr %x,disc_reason:%x\n",
            conn_id, conn_state, pair_state, addr[0], disc_reason);
    
    xf_ble_addr_t xf_addr = {.type = addr->type};
    memcpy(xf_addr.addr, addr->addr, BD_ADDR_LEN);
    
    if (conn_state == GAP_BLE_STATE_CONNECTED) {
        xf_ble_gap_evt_cb_param_t param = {
            .connect =
            {
                .conn_id = conn_id,
                .addr = &xf_addr,
            }
        };
        s_ble_gap_evt_cb(XF_BLE_GAP_EVT_CONNECT, &param);
    } else {
        xf_ble_gap_evt_cb_param_t param = {
            .disconnect =
            {
                .conn_id = conn_id,
                .addr = &xf_addr,
                .reason = disc_reason
            }
        };
        s_ble_gap_evt_cb(XF_BLE_GAP_EVT_DISCONNECT, &param);
    }
}

static void port_ble_paired_complete_cb(
    uint16_t conn_id, const bd_addr_t *addr, errcode_t status)
{
    unused(status);
    XF_LOGD(TAG, "port_ble_paired_complete_cb:conn_id:%d,status:%d\n",
            conn_id, status);

    xf_ble_addr_t xf_addr = {.type = addr->type};
    memcpy(xf_addr.addr, addr->addr, BD_ADDR_LEN);

    xf_ble_gap_evt_cb_param_t evt_param = {
        .pair_end =
        {
            .conn_id = conn_id,
            .addr = &xf_addr,
        }
    };
    s_ble_gap_evt_cb(XF_BLE_GAP_EVT_PAIR_END, &evt_param);
}

static void port_ble_gap_connect_param_update_cb(uint16_t conn_id, errcode_t status,
        const gap_ble_conn_param_update_t *param)
{
    unused(status);
    XF_LOGD(TAG, "ble_gap_connect_param_update_cb:conn_id:%d,status:%d,"
            "interval:%d,latency:%d,timeout:%d\n",
            conn_id, status, param->interval, param->latency, param->timeout);
    
    xf_ble_gap_evt_cb_param_t evt_param = {
        .conn_param_upd =
        {
            .conn_id = conn_id,
            .interval = param->interval,
            .latency = param->latency,
            .timeout = param->timeout,
        }
    };
    s_ble_gap_evt_cb(XF_BLE_GAP_EVT_CONN_PARAM_UPDATE, &evt_param);
}

static void port_ble_gap_adv_enable_cb(uint8_t adv_id, adv_status_t status)
{
    XF_LOGD(TAG, "adv enable adv_id: %d, status:%d\n", adv_id, status);
}

static void port_ble_gap_adv_disable_cb(uint8_t adv_id, adv_status_t status)
{
    XF_LOGD(TAG, "adv disable adv_id: %d, status:%d\n",
            adv_id, status);
}

static void port_ble_gap_scan_result_cb(gap_scan_result_data_t *scan_result_data)
{
    xf_ble_addr_t xf_addr = {.type = scan_result_data->addr.type};
    xf_memcpy(xf_addr.addr, scan_result_data->addr.addr, BD_ADDR_LEN);

    xf_ble_gap_evt_cb_param_t param = {
        .scan_result =
        {
            .rssi = scan_result_data->rssi,
            .adv_data_len = scan_result_data->adv_len,
            .adv_data = scan_result_data->adv_data,
            .addr = &xf_addr,
        }
    };

    typedef struct _scanned_adv_type_port_to_xf_t
    {
        uint8_t port_val;
        uint8_t xf_val;
    } scanned_adv_type_port_to_xf_t;
    
    static scanned_adv_type_port_to_xf_t s_map_scanned_adv_type_port_to_xf[] = 
    {
        {GAP_BLE_EVT_CONNECTABLE,                   XF_BLE_GAP_SCANNED_ADV_TYPE_CONN},
        {GAP_BLE_EVT_LEGACY_CONNECTABLE,            XF_BLE_GAP_SCANNED_ADV_TYPE_CONN},

        {GAP_BLE_EVT_CONNECTABLE_DIRECTED,          XF_BLE_GAP_SCANNED_ADV_TYPE_CONN_DIR},
        {GAP_BLE_EVT_LEGACY_CONNECTABLE_DIRECTED,   XF_BLE_GAP_SCANNED_ADV_TYPE_CONN_DIR},
        {GAP_BLE_EVT_SCANNABLE_DIRECTED,            XF_BLE_GAP_SCANNED_ADV_TYPE_CONN_DIR},

        {GAP_BLE_EVT_SCANNABLE,                     XF_BLE_GAP_SCANNED_ADV_TYPE_SCAN},
        {GAP_BLE_EVT_LEGACY_SCANNABLE,              XF_BLE_GAP_SCANNED_ADV_TYPE_SCAN},

        {GAP_BLE_EVT_LEGACY_NON_CONNECTABLE,        XF_BLE_GAP_SCANNED_ADV_TYPE_NONCONN},

        {GAP_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV_SCAN,   XF_BLE_GAP_SCANNED_ADV_TYPE_SCAN_RSP},
        {GAP_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV,        XF_BLE_GAP_SCANNED_ADV_TYPE_SCAN_RSP},
    };
    /* TODO 待优化成表  */
    uint8_t xf_scanned_adv_type = 0;
    uint8_t index = ARRAY_SIZE(s_map_scanned_adv_type_port_to_xf);
    while (index--)
    {
        if(s_map_scanned_adv_type_port_to_xf[index].port_val == scan_result_data->event_type)
        {
            param.scan_result.type = s_map_scanned_adv_type_port_to_xf[index].xf_val;
            break;
        }
    }
    /* 不在 xf_ble 定义的 扫描到的广播的类型值范围内 */
    if(xf_scanned_adv_type == 0)
    {
        param.scan_result.type = scan_result_data->event_type;
    }
    
    s_ble_gap_evt_cb(XF_BLE_GAP_EVT_SCAN_RESULT, &param);
}

static xf_ble_evt_res_t _port_ble_gap_evt_cb_default(
    xf_ble_gap_evt_t event, xf_ble_gap_evt_cb_param_t *param)
{
    UNUSED(event);
    UNUSED(param);
    return XF_BLE_EVT_RES_NOT_HANDLED;
}


static port_ble_adv_t *_port_ble_adv_alloc(void)
{
    uint8_t num_adv = 0;
    for(; num_adv < DEFAULT_BLE_ADV_MAX_CNT; num_adv++)
    {
        if(s_list_adv[num_adv] == NULL)
        {
            s_list_adv[num_adv] = xf_malloc(sizeof(port_ble_adv_t));
            xf_memset(s_list_adv[num_adv], 0, sizeof(port_ble_adv_t));
            // 因为 sdk 侧 adv_id 非传出获取的，是由用户自行设置的，所以索引号当作 ADV ID
            s_list_adv[num_adv]->adv_id = num_adv;
            return s_list_adv[num_adv];
        }
    }
    return NULL;
}

static xf_err_t *_port_ble_adv_free(port_ble_adv_t *adv)
{
    uint8_t num_adv = 0;
    for(; num_adv < DEFAULT_BLE_ADV_MAX_CNT; num_adv++)
    {
        if( (s_list_adv[num_adv] != NULL)
        &&  (s_list_adv[num_adv] == adv))
        {
            xf_free(s_list_adv[num_adv]);
            s_list_adv[num_adv] == NULL;
            return XF_OK;
        }
    }
    return XF_ERR_NOT_FOUND;
}

static xf_err_t *_port_ble_adv_free_by_id(xf_ble_adv_id_t adv_id)
{
    uint8_t num_adv = 0;
    for(; num_adv < DEFAULT_BLE_ADV_MAX_CNT; num_adv++)
    {
        if( (s_list_adv[num_adv] != NULL)
        &&  (s_list_adv[num_adv]->adv_id == adv_id)
        )
        {
            xf_free(s_list_adv[num_adv]);
            s_list_adv[num_adv] == NULL;
            return XF_OK;
        }
    }
    return XF_ERR_NOT_FOUND;
}

static port_ble_adv_t *_port_ble_adv_get_by_id(xf_ble_adv_id_t adv_id)
{
    uint8_t num_adv = 0;
    for(; num_adv < DEFAULT_BLE_ADV_MAX_CNT; num_adv++)
    {
        if( (s_list_adv[num_adv] != NULL)
        &&  (s_list_adv[num_adv]->adv_id == adv_id)
        )
        {
            return s_list_adv[num_adv];
        }
    }
    return NULL;
}

static inline errcode_t _port_ble_gap_set_adv_data(uint8_t adv_id, const xf_ble_gap_adv_data_t *data)
{
    errcode_t ret = ERRCODE_SUCC;
    /* 包含广播数据包及扫描响应数据包的信息 */
    gap_ble_config_adv_data_t adv_data_info = {0};  // 先清空

    uint8_t data_packed_size = 0;
    uint8_t data_packed_size_temp = 0;

    /* 解析 adv_struct_set ，设置广播数据 (adv_data) */
    data_packed_size = xf_ble_gap_adv_data_packed_size_get(data->adv_struct_set);
    XF_LOGD(TAG, "data_packed_size (adv_data):%d", data_packed_size);
    uint8_t data_packed_adv[XF_BLE_GAP_ADV_STRUCT_DATA_MAX_SIZE]={0};
    /* 有广播数据 -> 解析广播数据单元集合，填充广播数据包 */
    if(data_packed_size > 0)
    {
        memset(data_packed_adv, 0, data_packed_size);
        xf_ble_gap_adv_data_packed_by_adv_struct_set(data_packed_adv, data->adv_struct_set);
        adv_data_info.adv_data = data_packed_adv;
    }
    adv_data_info.adv_length = data_packed_size;

    /* 解析 scan_rsp_struct_set ，设置扫描响应数据 (scan_rsp_data) */
    data_packed_size = xf_ble_gap_adv_data_packed_size_get(data->scan_rsp_struct_set);
    XF_LOGD(TAG, "data_packed_size (scan_rsp_data):%d", data_packed_size);
    uint8_t data_packed_scan_rsp[XF_BLE_GAP_ADV_STRUCT_DATA_MAX_SIZE]={0};
    /* 有扫描响应数据 -> 解析扫描响应数据单元集合，填充扫描响应数据包 */
    if(data_packed_size > 0)
    {
        memset(data_packed_scan_rsp, 0, data_packed_size);
        xf_ble_gap_adv_data_packed_by_adv_struct_set(data_packed_scan_rsp, data->scan_rsp_struct_set);
        adv_data_info.scan_rsp_data = data_packed_scan_rsp;
    }
    adv_data_info.scan_rsp_length = data_packed_size;
    
    ret = gap_ble_set_adv_data(adv_id, &adv_data_info);
    XF_CHECK(ret != ERRCODE_SUCC, ret,
        TAG, "gap_ble_set_adv_data failed!:%#X", ret);

    return ERRCODE_SUCC;
}

#endif // #if (CONFIG_XF_BLE_ENABLE)