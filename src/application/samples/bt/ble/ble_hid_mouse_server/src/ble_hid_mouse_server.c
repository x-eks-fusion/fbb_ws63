/*
 * Copyright (c) @CompanyNameMagicTag. 2022. All rights reserved.
 * Description: BLE Mouse HID Service Server SAMPLE.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "osal_addr.h"
#include "securec.h"
#include "errcode.h"
#include "test_suite_uart.h"
#include "bts_def.h"
#include "bts_gatt_stru.h"
#include "bts_gatt_server.h"
#include "bts_le_gap.h"
#include "ble_hid_mouse_server.h"


#if 1 //defined(CONFIG_SAMPLE_SUPPORT_BLE_HID_MOUSE_SERVER)

#include "soc_osal.h"
#include "app_init.h"
#include "boards.h"
#include "pinctrl.h"
#include "gpio.h"
#include "ble_server_adv.h"
#include "systick.h"

#define IS_ASYNC     0

/* HID information flag remote wakeup */
#define BLE_HID_INFO_FLAG_REMOTE_WAKE_UP_MSK 0x01
/* HID information flag normally connectable */
#define BLE_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK 0x02
/* HID information country code */
#define BLE_HID_INFO_COUNTRY_CODE 0x00
/* HID spec version 1.11 */
#define BLE_HID_VERSION  0x0101
/* HID input report id */
#define BLE_HID_REPORT_ID 2
/* HID input report type */
#define BLE_REPORT_REFERENCE_REPORT_TYPE_INPUT_REPORT 1
/* HID output report type */
#define BLE_REPORT_REFERENCE_REPORT_TYPE_OUTPUT_REPORT 2
/* HID gatt server id */
#define BLE_HID_SERVER_ID 1
/* HID ble connect id */
#define BLE_SINGLE_LINK_CONNECT_ID 0
/* octets of 16 bits uuid */
#define UUID16_LEN 2
/* invalid attribute handle */
#define INVALID_ATT_HDL 0
/* invalid server ID */
#define INVALID_SERVER_ID 0

enum {
    /* HID information characteristic properties */
    HID_INFORMATION_PROPERTIES   = GATT_CHARACTER_PROPERTY_BIT_READ,
    /* HID protocol mode characteristic properties */
    HID_PROTOCOL_MODE_PROPERTIES = GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP,
    /* HID report map characteristic properties */
    HID_REPORT_MAP_PROPERTIES    = GATT_CHARACTER_PROPERTY_BIT_READ,
    /* HID input report characteristic properties */
    HID_INPUT_REPORT_PROPERTIES  = GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY |
                                   GATT_CHARACTER_PROPERTY_BIT_WRITE,
    /* HID output report characteristic properties */
    HID_OUTPUT_REPORT_PROPERTIES = GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_WRITE |
                                   GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP,
    /* HID control point characteristic properties */
    HID_CONTROL_POINT_PROPERTIES = GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP,
};

#define uint16_to_byte(n) ((uint8_t)(n)), ((uint8_t)((n) >> 8))

uint16_t g_device_mouse_appearance_value = GAP_BLE_APPEARANCE_TYPE_MOUSE;
uint8_t g_device_mouse_name_value[32] = "WS63_BLE_MOUSE";//{'M', 'o', 'u', 's', 'e', '6', 'x', '\0'};
uint8_t g_device_mouse_name_len = sizeof("WS63_BLE_MOUSE");
/* HID information value for test */
static uint8_t hid_information_val[] = { uint16_to_byte(BLE_HID_VERSION), BLE_HID_INFO_COUNTRY_CODE,
    BLE_HID_INFO_FLAG_REMOTE_WAKE_UP_MSK | BLE_HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK };
/* HID control point value for test */
static uint8_t control_point_val[] = {0x00, 0x00};
/* HID client characteristic configuration value for test */
static uint8_t ccc_val[] = {0x00, 0x00};
/* HID input report reference value for test  [report id 1, input] */
static uint8_t report_reference_val_input[] = {BLE_HID_REPORT_ID, BLE_REPORT_REFERENCE_REPORT_TYPE_INPUT_REPORT};
/* HID output report reference value for test [report id 1, output] */
static uint8_t report_reference_val_output[] = {BLE_HID_REPORT_ID, BLE_REPORT_REFERENCE_REPORT_TYPE_OUTPUT_REPORT};
/* HID input report value  for test
 * input report format:
 * data0 | data1 | data2 | data3 | data4 | data5 | data6
 * E0~E7 | key   | key   | key   | key   | key   | key 
 */
static uint8_t input_report_value[] = {0x00};
/* HID output report value for test */
static uint8_t output_report_value[] = {0x00};
/* HID protocol mode value for test */
static uint8_t protocol_mode_val[] = {0x00, 0x00};
/* HID server app uuid for test */
static uint8_t server_app_uuid_for_test[] = {0x00, 0x00};
/* hid input report att handle */
static uint16_t g_hid_input_report_att_hdl = INVALID_ATT_HDL;
/* gatt server ID */
static uint8_t g_server_id = INVALID_SERVER_ID;

/* Hid Report Map (Descriptor) */
static uint8_t g_srv_hid_mouse_report_map[] = {
    0x05, 0x01, /* Usage Page (Generic Desktop) */
    0x09, 0x02, /* Usage (Mouse) */
    0xA1, 0x01, /* Collection (Application) */
    0x85, 0x02, /* Report Id (2) */
    0x09, 0x01, /* Usage (Pointer) */
    0xA1, 0x00, /* Collection (Physical) */
    0x95, 0x10, /* Report Count (16) */
    0x75, 0x01, /* Report Size (1) */
    0x15, 0x00, /* Logical minimum (0) */
    0x25, 0x01, /* Logical maximum (1) */
    0x05, 0x09, /* Usage Page (Button) */
    0x19, 0x01, /* Usage Minimum (Button 1) */
    0x29, 0x10, /* Usage Maximum (Button 16) */
    0x81, 0x02, /* Input (data,Value,Absolute,Bit Field) */
    0x05, 0x01, /* Usage Page (Generic Desktop) */
    0x16, 0x01, 0xF8, /* Logical minimum (-2047) */
    0x26, 0xFF, 0x07, /* Logical maximum (2047) */
    0x75, 0x0C, /* Report Size (12) */
    0x95, 0x02, /* Report Count (2) */
    0x09, 0x30, /* Usage (X) */
    0x09, 0x31, /* Usage (Y) */
    0x81, 0x06, /* Input (data,Value,Relative,Bit Field) */
    0x15, 0x81, /* Logical minimum (-127) */
    0x25, 0x7F, /* Logical maximum (127) */
    0x75, 0x08, /* Report Size (8) */
    0x95, 0x01, /* Report Count (1) */
    0x09, 0x38, /* Usage (Wheel) */
    0x81, 0x06, /* Input (data,Value,Relative,Bit Field) */
    0x95, 0x01, /* Report Count (1) */
    0x05, 0x0C, /* Usage Page (Consumer) */
    0x0A, 0x38, 0x02, /* Usage (AC Pan) */
    0x81, 0x06, /* Input (data,Value,Relative,Bit Field) */
    0xC0, /* End Collection */
    0xC0, /* End Collection */
};

/* 将uint16的uuid数字转化为bt_uuid_t */
static void bts_data_to_uuid_len2(uint16_t uuid_data, bt_uuid_t *out_uuid)
{
    out_uuid->uuid_len = UUID16_LEN;
    out_uuid->uuid[0] = (uint8_t)(uuid_data >> 8); /* 8: octet bit num */
    out_uuid->uuid[1] = (uint8_t)(uuid_data);
}

#if IS_ASYNC
/* 创建服务 */
static void ble_hid_add_service(void)
{
    bt_uuid_t hid_service_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_HUMAN_INTERFACE_DEVICE, &hid_service_uuid);
    gatts_add_service(BLE_HID_SERVER_ID, &hid_service_uuid, true);
}
#endif

/* 添加特征：HID information */
static void ble_hid_add_character_hid_information(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_information_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_HID_INFORMATION, &hid_information_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_information_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    character.properties = HID_INFORMATION_PROPERTIES;
    character.value_len = sizeof(hid_information_val);
    character.value = hid_information_val;

#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
#endif
}

/* 添加特征：HID report map */
static void ble_hid_add_character_report_map(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_report_map_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_REPORT_MAP, &hid_report_map_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_report_map_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_AUTHENTICATION_NEED;
    character.properties = HID_REPORT_MAP_PROPERTIES;
    character.value_len = sizeof(g_srv_hid_mouse_report_map);
    character.value = g_srv_hid_mouse_report_map;
    
#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
#endif
}

/* 添加特征：HID control point */
static void ble_hid_add_character_hid_control_point(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_control_point_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_HID_CONTROL_POINT, &hid_control_point_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_control_point_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    character.properties = HID_CONTROL_POINT_PROPERTIES;
    character.value_len = sizeof(control_point_val);
    character.value = control_point_val;
    
#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
#endif
}

/* 添加描述符：客户端特性配置 */
static void ble_hid_add_descriptor_ccc(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t ccc_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_CLIENT_CHARACTERISTIC_CONFIGURATION, &ccc_uuid);
    gatts_add_desc_info_t descriptor;
    descriptor.desc_uuid = ccc_uuid;
    descriptor.permissions = GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE;
    descriptor.value_len = sizeof(ccc_val);
    descriptor.value = ccc_val;

#if IS_ASYNC
    gatts_add_descriptor(server_id, srvc_handle, &descriptor);
#else
    uint16_t result;
    errcode_t ret = gatts_add_descriptor_sync(server_id, srvc_handle, &descriptor, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add desc %d\r\n", __LINE__);
    }
#endif
}

/* 添加描述符：HID report reference */
static void ble_hid_add_descriptor_report_reference(uint8_t server_id, uint16_t srvc_handle, bool is_input_flag)
{
    bt_uuid_t hid_report_reference_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_REPORT_REFERENCE, &hid_report_reference_uuid);
    gatts_add_desc_info_t descriptor;
    descriptor.desc_uuid = hid_report_reference_uuid;
    descriptor.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    if (is_input_flag) {
        descriptor.value = report_reference_val_input;
        descriptor.value_len = sizeof(report_reference_val_input);
    } else {
        descriptor.value = report_reference_val_output;
        descriptor.value_len = sizeof(report_reference_val_output);
    }

#if IS_ASYNC
    gatts_add_descriptor(server_id, srvc_handle, &descriptor);
#else
    uint16_t result;
    errcode_t ret = gatts_add_descriptor_sync(server_id, srvc_handle, &descriptor, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add desc %d\r\n", __LINE__);
    }
#endif
}

/* 添加特征：HID input report(device to host) */
static void ble_hid_add_character_input_report(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_report_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_REPORT, &hid_report_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_report_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    character.properties = HID_INPUT_REPORT_PROPERTIES;
    character.value_len = sizeof(input_report_value);
    character.value = input_report_value;
    
#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
    g_hid_input_report_att_hdl = result.value_handle;
#endif

    ble_hid_add_descriptor_ccc(server_id, srvc_handle);
    ble_hid_add_descriptor_report_reference(server_id, srvc_handle, true);
}

/* 添加特征：HID output report(host to device) */
static void ble_hid_add_character_output_report(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_report_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_REPORT, &hid_report_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_report_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    character.properties = HID_OUTPUT_REPORT_PROPERTIES;
    character.value_len = sizeof(output_report_value);
    character.value = output_report_value;
    
#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
#endif

    ble_hid_add_descriptor_report_reference(server_id, srvc_handle, false);
}

/* 添加特征：HID protocol mode */
static void ble_hid_add_character_protocol_mode(uint8_t server_id, uint16_t srvc_handle)
{
    bt_uuid_t hid_protocol_mode_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_PROTOCOL_MODE, &hid_protocol_mode_uuid);
    gatts_add_chara_info_t character;
    character.chara_uuid = hid_protocol_mode_uuid;
    character.permissions = GATT_ATTRIBUTE_PERMISSION_READ;
    character.properties = HID_PROTOCOL_MODE_PROPERTIES;
    character.value_len = sizeof(protocol_mode_val);
    character.value = protocol_mode_val;
    
    
#if IS_ASYNC
    gatts_add_characteristic(server_id, srvc_handle, &character);
#else
    gatts_add_character_result_t result;
    errcode_t ret = gatts_add_characteristic_sync(server_id, srvc_handle, &character, &result);
    if(ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add chara %d\r\n", __LINE__);
    }
    g_hid_input_report_att_hdl = result.value_handle;
#endif
}

/* 添加HID服务的所有特征和描述符 */
static void ble_hid_add_characters_and_descriptors(uint8_t server_id, uint16_t srvc_handle)
{
    /* HID Information */
    ble_hid_add_character_hid_information(server_id, srvc_handle);
    /* Report Map */
    ble_hid_add_character_report_map(server_id, srvc_handle);
    /* Protocol Mode */
    ble_hid_add_character_protocol_mode(server_id, srvc_handle);
    /* Input Report */
    ble_hid_add_character_input_report(server_id, srvc_handle);
    /* Output Report */
    ble_hid_add_character_output_report(server_id, srvc_handle);
    /* HID Control Point */
    ble_hid_add_character_hid_control_point(server_id, srvc_handle);
}

bool bts_compare_mouse_uuid(bt_uuid_t *uuid1, bt_uuid_t *uuid2)
{
    if (uuid1->uuid_len != uuid2->uuid_len) {
        return false;
    }
    if (memcmp(uuid1->uuid, uuid2->uuid, uuid1->uuid_len) != 0) {
        return false;
    }
    return true;
}

#if IS_ASYNC
/* 服务添加回调 */
static void ble_hid_server_service_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t handle, errcode_t status)
{
    int8_t i = 0;
    test_suite_uart_sendf("[hid]ServiceAddCallback server: %d srv_handle: %d uuid_len: %d\n",
        server_id, handle, uuid->uuid_len);
    test_suite_uart_sendf("uuid:");
    for (i = 0; i < uuid->uuid_len ; i++) {
        test_suite_uart_sendf("%02x", uuid->uuid[i]);
    }
    test_suite_uart_sendf("\n");
    test_suite_uart_sendf("status:%d\n", status);
    test_suite_uart_sendf("[hid][INFO]beginning add characters and descriptors\r\n");
    ble_hid_add_characters_and_descriptors(server_id, handle);
    test_suite_uart_sendf("[hid][INFO]beginning start service\r\n");
    gatts_start_service(server_id, handle);
}

/* 特征添加回调 */
static void  ble_hid_server_characteristic_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
    gatts_add_character_result_t *result, errcode_t status)
{
    int8_t i = 0;
    bt_uuid_t report_uuid = {0};
    bts_data_to_uuid_len2(BLE_UUID_REPORT, &report_uuid);
    test_suite_uart_sendf("[hid]CharacteristicAddCallback server: %d srvc_hdl: %d "
        "char_hdl: %d char_val_hdl: %d uuid_len: %d \n",
        server_id, service_handle, result->handle, result->value_handle, uuid->uuid_len);
    test_suite_uart_sendf("uuid:");
    for (i = 0; i < uuid->uuid_len ; i++) {
        test_suite_uart_sendf("%02x", uuid->uuid[i]);
    }
    if ((g_hid_input_report_att_hdl == INVALID_ATT_HDL) && (bts_compare_mouse_uuid(uuid, &report_uuid))) {
        g_hid_input_report_att_hdl = result->value_handle;
    }
    test_suite_uart_sendf("\n");
    test_suite_uart_sendf("status:%d\n", status);
}

/* 描述符添加回调 */
static void  ble_hid_server_descriptor_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
    uint16_t handle, errcode_t status)
{
    int8_t i = 0;
    test_suite_uart_sendf("[hid]DescriptorAddCallback server: %d srv_hdl: %d desc_hdl: %d "
        "uuid_len:%d\n", server_id, service_handle, handle, uuid->uuid_len);
    test_suite_uart_sendf("uuid:");
    for (i = 0; i < uuid->uuid_len ; i++) {
        test_suite_uart_sendf("%02x", (uint8_t)uuid->uuid[i]);
    }
    test_suite_uart_sendf("\n");
    test_suite_uart_sendf("status:%d\n", status);
}

/* 开始服务回调 */
static void ble_hid_server_service_start_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    test_suite_uart_sendf("[hid]ServiceStartCallback server: %d srv_hdl: %d status: %d\n",
        server_id, handle, status);
}

#endif

static void ble_hid_receive_write_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_write_cb_t *write_cb_para,
    errcode_t status)
{
    test_suite_uart_sendf("[hid]ReceiveWriteReqCallback--server_id:%d conn_id:%d\n", server_id, conn_id);
    test_suite_uart_sendf("request_id:%d att_handle:%d offset:%d need_rsp:%d need_authorize:%d is_prep:%d\n",
        write_cb_para->request_id, write_cb_para->handle, write_cb_para->offset, write_cb_para->need_rsp,
        write_cb_para->need_authorize, write_cb_para->is_prep);
    test_suite_uart_sendf("data_len:%d data:\n", write_cb_para->length);
    for (uint8_t i = 0; i < write_cb_para->length; i++) {
        test_suite_uart_sendf("%02x ", write_cb_para->value[i]);
    }
    test_suite_uart_sendf("\n");
    test_suite_uart_sendf("status:%d\n", status);
}

static void ble_hid_receive_read_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_read_cb_t *read_cb_para,
    errcode_t status)
{
    test_suite_uart_sendf("[hid]ReceiveReadReq--server_id:%d conn_id:%d\n", server_id, conn_id);
    test_suite_uart_sendf("request_id:%d att_handle:%d offset:%d need_rsp:%d need_authorize:%d is_long:%d\n",
        read_cb_para->request_id, read_cb_para->handle, read_cb_para->offset, read_cb_para->need_rsp,
        read_cb_para->need_authorize, read_cb_para->is_long);
    test_suite_uart_sendf("status:%d\n", status);
}

static void ble_hid_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    test_suite_uart_sendf("[hid]MtuChanged--server_id:%d conn_id:%d\n", server_id, conn_id);
    test_suite_uart_sendf("mtusize:%d\n", mtu_size);
    test_suite_uart_sendf("status:%d\n", status);
}

static errcode_t ble_hid_server_register_gatt_callbacks(void)
{
    errcode_t ret = ERRCODE_BT_SUCCESS;
    gatts_callbacks_t cb = {0};
#if IS_ASYNC
    cb.add_service_cb = ble_hid_server_service_add_cbk;
    cb.add_characteristic_cb = ble_hid_server_characteristic_add_cbk;
    cb.add_descriptor_cb = ble_hid_server_descriptor_add_cbk;
    cb.start_service_cb = ble_hid_server_service_start_cbk;
#endif
    cb.read_request_cb = ble_hid_receive_read_req_cbk;
    cb.write_request_cb = ble_hid_receive_write_req_cbk;
    cb.mtu_changed_cb = ble_hid_mtu_changed_cbk;
    ret = gatts_register_callbacks(&cb);
    return ret;
}

/* 设置注册服务时的name */
void ble_hid_set_mouse_name_value(const uint8_t *name, const uint8_t len)
{
    size_t len_name = sizeof(g_device_mouse_name_value);
    if (memcpy_s(&g_device_mouse_name_value, len_name, name, len) != EOK) {
        test_suite_uart_sendf("[btsrv][ERROR] memcpy name fail\n");
    }
}

/* 设置注册服务时的appearance */
void ble_hid_set_mouse_appearance_value(uint16_t appearance)
{
    g_device_mouse_appearance_value = appearance;
}

/* 初始化HID device */
void ble_hid_mouse_server_init(void)
{
    ble_hid_server_register_gatt_callbacks();
    errcode_t ret = ERRCODE_BT_UNHANDLED;
    bt_uuid_t app_uuid = {0};
    app_uuid.uuid_len = sizeof(server_app_uuid_for_test);
    if (memcpy_s(app_uuid.uuid, app_uuid.uuid_len, server_app_uuid_for_test, sizeof(server_app_uuid_for_test)) != EOK) {
        test_suite_uart_sendf("[hid][ERROR]add server app uuid memcpy failed\r\n");
        return;
    }
    test_suite_uart_sendf("[hid][INFO]beginning add server\r\n");
    enable_ble();
    gap_ble_set_local_name(g_device_mouse_name_value, g_device_mouse_name_len);
    gap_ble_set_local_appearance(g_device_mouse_appearance_value);
    ret = gatts_register_server(&app_uuid, &g_server_id);
    if ((ret != ERRCODE_BT_SUCCESS) || (g_server_id == INVALID_SERVER_ID)) {
        test_suite_uart_sendf("[hid][ERROR]add server failed\r\n");
        return;
    }
    test_suite_uart_sendf("[hid][INFO]beginning add service\r\n");
#if IS_ASYNC
    ble_hid_add_service(); /* 添加HID服务 */
#else
    bt_uuid_t hid_service_uuid = {0};
    uint16_t hdl_svc = 0;
    ret = ERRCODE_SUCC;
    
    bts_data_to_uuid_len2(BLE_UUID_HUMAN_INTERFACE_DEVICE, &hid_service_uuid);
    ret = gatts_add_service_sync(BLE_HID_SERVER_ID, &hid_service_uuid, true, &hdl_svc);
    printf("[>>>>> svc hdl %d\r\n", hdl_svc);
    if (ret != ERRCODE_SUCC)
    {
        printf("[ERR]>> add svc %d\r\n", __LINE__);
    }
    
    ble_hid_add_characters_and_descriptors(BLE_HID_SERVER_ID, hdl_svc);

    gatts_start_service(BLE_HID_SERVER_ID, hdl_svc);
#endif
}

/* device向host发送数据：input report */
errcode_t ble_hid_mouse_server_send_input_report(const ble_hid_high_mouse_event_st *mouse_data)
{
    gatts_ntf_ind_t param = {0};
    param.attr_handle = g_hid_input_report_att_hdl;
    param.value_len = sizeof(ble_hid_high_mouse_event_st);
    param.value = osal_vmalloc(sizeof(ble_hid_high_mouse_event_st));
    if (param.value == NULL) {
        test_suite_uart_sendf("[hid][ERROR]send input report new fail\r\n");
        return ERRCODE_BT_MALLOC_FAIL;
    }
    if (memcpy_s(param.value, param.value_len, mouse_data, sizeof(ble_hid_high_mouse_event_st)) != EOK) {
        test_suite_uart_sendf("[hid][ERROR]send input report memcpy fail\r\n");
        osal_vfree(param.value);
        return ERRCODE_BT_FAIL;
    }
    gatts_notify_indicate(BLE_HID_SERVER_ID, BLE_SINGLE_LINK_CONNECT_ID, &param);
    osal_vfree(param.value);
    return ERRCODE_BT_SUCCESS;
}

/* device向host发送数据by uuid：input report */
errcode_t ble_hid_mouse_server_send_input_report_by_uuid(const ble_hid_high_mouse_event_st *mouse_data)
{
    gatts_ntf_ind_by_uuid_t param = {0};
    param.start_handle = g_hid_input_report_att_hdl;
    param.end_handle = g_hid_input_report_att_hdl;
    bts_data_to_uuid_len2(BLE_UUID_REPORT, &param.chara_uuid);
    param.value_len = sizeof(ble_hid_high_mouse_event_st);
    param.value = osal_vmalloc(sizeof(ble_hid_high_mouse_event_st));
    if (param.value == NULL) {
        test_suite_uart_sendf("[hid][ERROR]send input report new fail\r\n");
        return ERRCODE_BT_MALLOC_FAIL;
    }
    if (memcpy_s(param.value, param.value_len, mouse_data, sizeof(ble_hid_high_mouse_event_st)) != EOK) {
        test_suite_uart_sendf("[hid][ERROR]send input report memcpy fail\r\n");
        osal_vfree(param.value);
        return ERRCODE_BT_FAIL;
    }
    gatts_notify_indicate_by_uuid(BLE_HID_SERVER_ID, BLE_SINGLE_LINK_CONNECT_ID, &param);
    osal_vfree(param.value);
    return ERRCODE_BT_SUCCESS;
}

/************************************************
 * 
 *  以下为增加的对 sample 的调用
 * 
*/

#define TEST_BLE_TASK_PRIO          24
#define TEST_BLE_TASK_STACK_SIZE    0x1000
#define BUTTON_0 GPIO_03

#define TEST_BTN_LONGPRESS_MS   2000
static bool is_short_click = false, is_long_press_click = false;
static uint32_t time_press_start = 0;
#define BUTTON                  (pin_t)CONFIG_SAMPLE_BUTTON_PIN
#define DELAY_US                CONFIG_SAMPLE_BUTTON_DEBOUNCE_DELAY_US

static void gpio_callback_func(uint8_t pin_group, uintptr_t state)
{
    unused(pin_group);
    unused(state);
    osal_printk("Button EVT:");
    bool key_val = uapi_gpio_get_val(BUTTON_0);
    if (key_val == true) {
        time_press_start = uapi_systick_get_ms();
        osal_printk("Pressed:%d\r\n", time_press_start);
        
    } else {
        
        uint32_t press_time = uapi_systick_get_ms() - time_press_start;
        if( press_time >= TEST_BTN_LONGPRESS_MS )
        {
            osal_printk(">>Released: long press click:%d\r\n", press_time);
            is_long_press_click = true;
        }
        else
        {
            osal_printk(">>Released: short click:%d\r\n", press_time);
            is_short_click = true;
        }
    }
}

static void button_init(void)
{
    osal_printk(">>>>>>> button_init: IO%d\r\n", BUTTON_0);
    uapi_pin_set_mode(BUTTON_0, HAL_PIO_FUNC_GPIO);

    gpio_select_core(BUTTON_0, CORES_APPS_CORE);
    uapi_pin_set_pull(BUTTON_0, PIN_PULL_TYPE_DOWN);
    uapi_gpio_set_dir(BUTTON_0, GPIO_DIRECTION_INPUT);
    uapi_gpio_disable_interrupt(BUTTON_0);
    uapi_gpio_register_isr_func(BUTTON_0, GPIO_INTERRUPT_DEDGE, gpio_callback_func);
    uapi_gpio_enable_interrupt(BUTTON_0);
}

errcode_t test_ble_set_adv_data(uint8_t adv_id)
{
    errcode_t ret = 0;
    
    gap_ble_config_adv_data_t cfg_adv_data = 
    {
        .adv_data = 0,//(uint8_t *)"blemouse",  // 广播数据
        .adv_length = 0,//sizeof("blemouse"),
        .scan_rsp_data = 0,//(uint8_t *)"resp", // 扫描返回的数据（对端）
        .scan_rsp_length = 0//sizeof("resp"),
    };
    ret =  gap_ble_set_adv_data(adv_id, &cfg_adv_data);
    return ret;
}


errcode_t test_ble_set_adv_param(uint8_t adv_id)
{
    errcode_t ret = 0;
    gap_ble_adv_params_t adv_param =
    {
        /* 广播间隔 [N * 0.625ms] */
        .min_interval = BLE_ADV_MIN_INTERVAL,
        .max_interval = BLE_ADV_MAX_INTERVAL,
        
        .adv_type = BLE_ADV_TYPE_CONNECTABLE_UNDIRECTED, // 广播地址类型

        /**
         * ble地址类型有2大类：
         *  1、公共设备地址（类似USB VID，需申请）
         *  2、随机设备地址：设备启动后随机生成
         *      2.A、静态设备地址：在启动后地址不变，下次复位后地址可能会变（非强制要求）
         *      2.B、私有设备地址：地址会更新
         *          2.B.1、不可解析私有地址：地址定时更新
         *          2.B.2、可解析私有地址：地址加密生成
         */
        .own_addr = 
        {
            .type = BLE_PUBLIC_DEVICE_ADDRESS,  // 本端地址类型
            .addr = { 0x0 },//{0x11,0x22,0x33,0x44,0x55,0x66},   // 本端地址
        },
        .peer_addr = 
        {
            .type = BLE_PUBLIC_DEVICE_ADDRESS,  // 对端地址类型
            .addr = { 0x0 },   // 对端地址
        }, 
        /**
         * // 广播通道: [0x01, 0x07] 
         *      0x01----使用37通道 
         *      0x07----使用37/38/39三个通道
         */
        .channel_map = BLE_ADV_CHANNEL_MAP_CH_DEFAULT,   
        .adv_filter_policy = GAP_BLE_ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // 白名单过滤策略（无白名单过滤）
        .tx_power = 1,  // 发送功率,单位dbm,范围-127~20
        .duration = BTH_GAP_BLE_ADV_FOREVER_DURATION,  // 广播持续发送时长
    
    };
    ret = gap_ble_set_adv_param(adv_id, &adv_param);
    return ret;
}


static int ble_test_task(const char *arg)
{
    unused(arg);
    uint16_t cnt = 0;
    uint8_t adv_id = BTH_GAP_BLE_ADV_HANDLE_DEFAULT;
    while (1)
    {
        /* 开机后仅当有长按事件时才初始化 ble hid mouse  */
        if( is_long_press_click == true )
        {
            errcode_t ret = 0;
            /* ble 初始化 设置服务 */
            ble_hid_mouse_server_init();
            osal_msleep(500);
            
            /* 设置 ble 广播数据 */
            ret = test_ble_set_adv_data(adv_id);
            if( ret != ERRCODE_SUCC )
            {
                osal_printk(">>> test_ble_set_adv_data != ERRCODE_SUCC:%d\r\n", ret);
                return ret;
            }

            /* 设置 ble 广播参数 */
            ret = test_ble_set_adv_param(adv_id);
            if( ret != ERRCODE_SUCC )
            {
                osal_printk(">>> test_ble_set_adv_param != ERRCODE_SUCC:%d\r\n", ret);
                return ret;
            }

            /* ble 起广播 */
            ret = gap_ble_start_adv(adv_id);
            if( ret != ERRCODE_SUCC )
            {
                osal_printk(">>> gap_ble_start_adv != ERRCODE_SUCC:%d\r\n", ret);
                return ret;
            }

            is_long_press_click = false;
            break;
        }
        if(cnt++%8 == 0)
        {
            osal_printk(">>>btn(IO%d) long press to start test:%d\r\n", BUTTON_0);
        }
        osal_msleep(500);
    }
    
    ble_hid_high_mouse_event_st mouse_data = 
    {
        .ac_pan = 0,
        .button_mask = 0,
        .wheel = 0,
        .x = 60,
        .y = 0,
    };
    while (1) 
    {
        osal_msleep(500);
        
        /* 每次 短按键 触发将会上报鼠标数据，鼠标会往右平移 60 pix  */
        if( is_short_click == true )
        {
            errcode_t ret = 
                ble_hid_mouse_server_send_input_report(&mouse_data);
            osal_printk(">>> report mouse evt. ret:%d\r\n", ret);
            is_short_click = false;
        }
    }
    return 0;
}

static void test_ble_init(void)
{
    button_init();
    osal_task *task_handle = NULL;
    osal_kthread_lock();
    task_handle = osal_kthread_create((osal_kthread_handler)ble_test_task, 0, "ble_test_task", TEST_BLE_TASK_STACK_SIZE);
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, TEST_BLE_TASK_PRIO);
        osal_kfree(task_handle);
    }
    osal_kthread_unlock();
}

app_run(test_ble_init);

#endif
