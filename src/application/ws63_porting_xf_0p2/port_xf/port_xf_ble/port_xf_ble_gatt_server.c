/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_BLE_ENABLE)

#include "xf_utils.h"
#include "xf_ble_gatt_server.h"

#include "xf_init.h"
#include "xf_heap.h"

#include "port_utils.h"

#include "common_def.h"
#include "soc_osal.h"

#include "bts_gatt_server.h"
#include "bts_le_gap.h"

/* ==================== [Defines] =========================================== */

#define TAG "port_ble_gatts"

#define INTERVAL_MS_CHECK_ATTR_ADD   (5)
#define TIMEOUT_CNT_CHECK_ATTR_ADD   (20)

/* ==================== [Typedefs] ========================================== */

typedef struct _port_service_added_node_t {
    xf_list_t link;
    uint8_t app_id;
    xf_ble_gatts_service_t *service;
} port_service_added_node_t;

/* ==================== [Static Prototypes] ================================= */

void port_ble_gatts_service_del_cb(uint8_t app_id, errcode_t status);
static void port_ble_gatts_recv_read_req_cb(
    uint8_t app_id, uint16_t conn_id, 
    gatts_req_read_cb_t *read_cb_para, errcode_t status);
static void port_ble_gatts_recv_write_req_cb(
    uint8_t app_id, uint16_t conn_id, 
    gatts_req_write_cb_t *write_cb_para, errcode_t status);
static void port_ble_gatts_mtu_changed_cb(
    uint8_t app_id, uint16_t conn_id, 
    uint16_t mtu_size, errcode_t status);
static xf_ble_evt_res_t _port_ble_gatts_evt_cb_default(
    xf_ble_gatts_evt_t event, xf_ble_gatts_evt_cb_param_t *param);

/* ==================== [Static Variables] ================================== */

static xf_ble_gatts_evt_cb_t s_ble_gatts_evt_cb = _port_ble_gatts_evt_cb_default;

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

xf_err_t xf_ble_gatts_app_register(
    xf_ble_uuid_info_t *app_uuid, uint8_t *app_id)
{
    bt_uuid_t app_uuid_param = {
        .uuid_len = app_uuid->type,
    };
    memcpy(app_uuid_param.uuid, app_uuid->uuid128, app_uuid->type);

    XF_LOGI(TAG, "register_app_profile: len:%d uuid(16):%X",
            app_uuid_param.uuid_len,
            *(uint16_t *)app_uuid_param.uuid
           );
    errcode_t ret = gatts_register_server(
                        &app_uuid_param, app_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_register_server failed!:%#X", ret);
    return XF_OK;
}

xf_err_t xf_ble_gatts_app_unregister(uint8_t app_id)
{
    errcode_t ret = gatts_unregister_server(app_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_unregister_server failed!:%#X", ret);
    return XF_OK;
}

void xf_ble_uuid_endian_convert(bt_uuid_t *uuid_info)
{
    uint8_t len =  uuid_info->uuid_len;
    uint8_t cnt = 0;
    for(; cnt < len; )
    {
        uint8_t temp = uuid_info->uuid[cnt]; 
        uuid_info->uuid[cnt] = uuid_info->uuid[cnt+1];
        uuid_info->uuid[cnt+1] = temp;
        cnt += 2;
    }
}

xf_err_t xf_ble_gatts_add_service(
    uint8_t app_id,
    xf_ble_gatts_service_t *service)
{
    xf_err_t xf_ret = XF_OK;
    errcode_t ret = ERRCODE_SUCC;
    XF_CHECK(service == NULL, XF_ERR_INVALID_ARG, TAG, "service == NULL");
    /* 特征声明集不为空即有效时，才创建服务添加任务 */
    XF_CHECK(service->chara_set == NULL, XF_ERR_INVALID_ARG, TAG, "service->chara_set == NULL");

    bt_uuid_t service_uuid_param = {
        .uuid_len = service->uuid->type,
    };
    
    memcpy(service_uuid_param.uuid, service->uuid->uuid128,
           service->uuid->type);
    xf_ble_uuid_endian_convert(&service_uuid_param);
    
    ret = gatts_add_service_sync(app_id, &service_uuid_param,
                    (service->type == XF_BLE_GATT_SERVICE_TYPE_PRIMARY) ?
                    true : false, &service->handle);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
            TAG, "gatts_add_service_sync failed!:%#X", ret);

    XF_LOGI(TAG, ">>> SVC:0x%02X%02X hdl:%d", 
            service_uuid_param.uuid[0], service_uuid_param.uuid[1], 
            service->handle);

    /**
     * 2、添加服务下的特征
        * 从头检查 chara_set 数组各项，逐个特征项添加，直至遇到遇到
        * uuid == XF_BLE_ATTR_SET_END_FLAG 的项结束
        */
    xf_ble_gatts_chara_t *chara_set = service->chara_set;
    uint16_t cnt_chara = 0;
    while (chara_set[cnt_chara].uuid != XF_BLE_ATTR_SET_END_FLAG) {
        gatts_add_chara_info_t chara = {
            /* 特征相关 */
            .chara_uuid.uuid_len = chara_set[cnt_chara].uuid->type,
            .properties = chara_set[cnt_chara].props,
            .permissions = chara_set[cnt_chara].perms,
            /* 特征值相关 */
            .value = chara_set[cnt_chara].value,
            .value_len = chara_set[cnt_chara].value_len,
        };

        memcpy(chara.chara_uuid.uuid, chara_set[cnt_chara].uuid->uuid128,
                chara_set[cnt_chara].uuid->type);
        
        xf_ble_uuid_endian_convert(&chara.chara_uuid);
        gatts_add_character_result_t result_chara = {0};
        ret = gatts_add_characteristic_sync(
            app_id, service->handle, &chara, &result_chara);
        XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
            TAG, "gatts_add_characteristic_sync failed!:%#X", ret);

        chara_set[cnt_chara].handle = result_chara.handle;
        chara_set[cnt_chara].value_handle = result_chara.value_handle;
        
        XF_LOGI(TAG, ">>> CHARA:0x%02X%02X hdl:%d val_hdl:%d", 
            chara.chara_uuid.uuid[0], chara.chara_uuid.uuid[1], 
            chara_set[cnt_chara].handle, chara_set[cnt_chara].value_handle);
        
        /**
         * 3、如果特征描述符集存在时，
            * 则从头检查 desc_set 数组各项，逐个特征项添加，直至遇到遇到
            * desc_uuid == XF_BLE_ATTR_SET_END_FLAG 的项结束
            */
        xf_ble_gatts_desc_t *desc_set = chara_set[cnt_chara].desc_set;
        if (desc_set == NULL) {
            ++cnt_chara;
            continue;
        }
        uint16_t cnt_desc = 0;
        while (desc_set[cnt_desc].uuid != XF_BLE_ATTR_SET_END_FLAG) {
            gatts_add_desc_info_t desc = {
                .desc_uuid.uuid_len = desc_set[cnt_desc].uuid->type,
                .permissions = desc_set[cnt_desc].perms,
                .value = desc_set[cnt_desc].value,
                .value_len = desc_set[cnt_desc].value_len
            };

            xf_memcpy(desc.desc_uuid.uuid, desc_set[cnt_desc].uuid->uuid128,
                    desc_set[cnt_desc].uuid->type);
            xf_ble_uuid_endian_convert(&desc.desc_uuid);
            ret = gatts_add_descriptor_sync(
                app_id, service->handle, &desc, &desc_set[cnt_desc].handle);
            XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
                TAG, "gatts_add_descriptor_sync failed!:%#X", ret);

            XF_LOGI(TAG, ">>> DESC:0x%02X%02X hdl:%d", 
                desc.desc_uuid.uuid[0], desc.desc_uuid.uuid[1], 
                desc_set[cnt_desc].handle);

            ++cnt_desc;
        }
        ++cnt_chara;
    }
    return XF_OK;
}

// 启动一个 GATT 服务
xf_err_t xf_ble_gatts_start_service(
    uint8_t app_id, uint16_t service_handle)
{
    errcode_t ret = gatts_start_service(
                        app_id, service_handle);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_start_service failed!:%#X", ret);
    return XF_OK;
}

// 停止一个 GATT 服务
xf_err_t xf_ble_gatts_stop_service(
    uint8_t app_id, uint16_t service_handle)
{
    errcode_t ret = gatts_stop_service(
                        app_id, service_handle);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_stop_service failed!:%#X", ret);
    return XF_OK;
}

// 删除所有 GATT 服务
xf_err_t xf_ble_gatts_del_services_all(uint8_t app_id)
{
    errcode_t ret = gatts_delete_all_services(app_id);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_delete_all_services failed!:%#X", ret);
    return XF_OK;
}

// 发送通知（向客户端）
xf_err_t xf_ble_gatts_send_notification(
    xf_ble_app_id_t app_id, xf_ble_conn_id_t conn_id,
    xf_ble_gatts_ntf_t *param)
{
    gatts_ntf_ind_t ntf_ind_param = {
        .attr_handle = param->handle,
        .value = param->value,
        .value_len = param->value_len
    };
    errcode_t ret = gatts_notify_indicate(
                        app_id, conn_id, &ntf_ind_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "ntf failed!:%#X", ret);
    return XF_OK;
}

// 发送指示（向客户端）
xf_err_t xf_ble_gatts_send_indication(
    xf_ble_app_id_t app_id, xf_ble_conn_id_t conn_id,
    xf_ble_gatts_ind_t *param)
{
    gatts_ntf_ind_t ntf_ind_param = {
        .attr_handle = param->handle,
        .value = param->value,
        .value_len = param->value_len
    };
    errcode_t ret = gatts_notify_indicate(
                        app_id, conn_id, &ntf_ind_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "ind failed!:%#X", ret);
    return XF_OK;
}

xf_err_t xf_ble_gatts_send_read_rsp(
    xf_ble_app_id_t app_id, xf_ble_conn_id_t conn_id,
    xf_ble_gatts_response_t *param)
{
    gatts_send_rsp_t rsp_param = {
        .status = param->err,
        .value = param->value,
        .value_len = param->value_len,
        .offset = param->offset,
        .request_id = param->trans_id // ?
    };
    errcode_t ret = gatts_send_response(
                        app_id, conn_id, &rsp_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "send read rsp failed!:%#X", ret);
    return XF_OK;
}

xf_err_t xf_ble_gatts_send_write_rsp(
    xf_ble_app_id_t app_id, xf_ble_conn_id_t conn_id,
    xf_ble_gatts_response_t *param)
{
    gatts_send_rsp_t rsp_param = {
        .status = param->err,
        .value = param->value,
        .value_len = param->value_len,
        .offset = param->offset,
        .request_id = param->trans_id // ?
    };
    errcode_t ret = gatts_send_response(
                        app_id, conn_id, &rsp_param);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "send write rsp failed!:%#X", ret);
    return XF_OK;
}

// 事件回调注册
xf_err_t xf_ble_gatts_event_cb_register(
    xf_ble_gatts_evt_cb_t evt_cb,
    xf_ble_gatts_evt_t events)
{
    unused(events);
    errcode_t ret = ERRCODE_SUCC;
    s_ble_gatts_evt_cb = evt_cb;

    gatts_callbacks_t gatts_cb_set = {
        .read_request_cb = port_ble_gatts_recv_read_req_cb,
        .write_request_cb = port_ble_gatts_recv_write_req_cb,
        .mtu_changed_cb = port_ble_gatts_mtu_changed_cb,
    };
    ret = gatts_register_callbacks(&gatts_cb_set);
    XF_CHECK(ret != ERRCODE_SUCC, (xf_err_t)ret,
             TAG, "gatts_register_callbacks failed!:%#X", ret);
    return XF_OK;
}

/* ==================== [Static Functions] ================================== */

static xf_ble_evt_res_t _port_ble_gatts_evt_cb_default(
    xf_ble_gatts_evt_t event, xf_ble_gatts_evt_cb_param_t *param)
{
    UNUSED(event);
    UNUSED(param);
    return XF_BLE_EVT_RES_NOT_HANDLED;
}

/* 服务移除回调 */
void port_ble_gatts_service_del_cb(uint8_t app_id, errcode_t status)
{
    unused(app_id);
    unused(status);
}

/* 收到读请求的回调 */
static void port_ble_gatts_recv_read_req_cb(uint8_t app_id, uint16_t conn_id, gatts_req_read_cb_t *read_cb_para,
        errcode_t status)
{
    unused(read_cb_para);
    unused(status);

    xf_ble_gatts_evt_cb_param_t evt_param = {
        .read_req =
        {
            .app_id = app_id,
            .conn_id = conn_id,
            .handle = read_cb_para->handle,
            .is_long = read_cb_para->is_long,
            .need_author = read_cb_para->need_authorize,
            .need_rsp = read_cb_para->need_rsp,
            .offset = read_cb_para->offset,
            .trans_id = read_cb_para->request_id,   // FIXME？
        }
    };
    s_ble_gatts_evt_cb(XF_BLE_GATTS_EVT_READ_REQ, &evt_param);
}

/* 收到写请求的回调 */
static void port_ble_gatts_recv_write_req_cb(uint8_t app_id, uint16_t conn_id, gatts_req_write_cb_t *write_cb_para,
        errcode_t status)
{
    unused(write_cb_para);
    unused(status);

    xf_ble_gatts_evt_cb_param_t evt_param = {
        .write_req =
        {
            .app_id = app_id,
            .conn_id = conn_id,
            .handle = write_cb_para->handle,
            .is_prep = write_cb_para->is_prep,
            .value_len = write_cb_para->length,
            .need_author = write_cb_para->need_authorize,
            .need_rsp = write_cb_para->need_rsp,
            .offset = write_cb_para->offset,
            .trans_id = write_cb_para->request_id,   // FIXME？
            .value = write_cb_para->value,
        }
    };
    s_ble_gatts_evt_cb(XF_BLE_GATTS_EVT_WRITE_REQ, &evt_param);
}

/* 收到 MTU 协商的回调 */
static void port_ble_gatts_mtu_changed_cb(uint8_t app_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    unused(status);

    xf_ble_gatts_evt_cb_param_t evt_param = {
        .mtu =
        {
            .app_id = app_id,
            .conn_id = conn_id,
            .mtu_size = mtu_size,
        }
    };
    s_ble_gatts_evt_cb(XF_BLE_GATTS_EVT_EXCHANGE_MTU, &evt_param);
}

#endif // #if (CONFIG_XF_BLE_ENABLE)
