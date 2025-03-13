#ifndef __PORT_SLE_SSAP_H__
#define __PORT_SLE_SSAP_H__

/* ==================== [Includes] ========================================== */

#include "xfconfig.h"

#if (CONFIG_XF_SLE_ENABLE)

#include "xf_utils.h"
#include "sle_ssap_stru.h"
#include "xf_sle_ssap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== [Defines] =========================================== */

#define INTERVAL_MS_CHECK_ATTR_ADD   (20)
#define TIMEOUT_CNT_CHECK_ATTR_ADD   (500)

/**
 * @brief 定义 ws63 结构的 uuid，
 *  并将 xf_sle_uuid_info 对应填充至对应位置
 * @param ws63_uuid_name 要定义的 ws63 结构的 uuid 的名字
 * @param ptr_xf_sle_uuid_info 指向 xf_sle_uuid_info 的指针
 */
#define PORT_SLE_DEF_WS63_UUID_FROM_XF_UUID(ws63_uuid_name, ptr_xf_sle_uuid_info) \
sle_uuid_t ws63_uuid_name =                                     \
{                                                               \
    .len = (ptr_xf_sle_uuid_info)->type,                      \
};                                                              \
memcpy(ws63_uuid_name.uuid, (ptr_xf_sle_uuid_info)->uuid128,      \
    (ptr_xf_sle_uuid_info)->type)                             \

/**
 * @brief 从 xf_sle_uuid_info 转换填充对应信息至 ws63 结构的 uuid，
 * @param ptr_ws63_uuid 要设置的 ws63 结构的 uuid 指针
 * @param ptr_xf_sle_uuid_info 指向 xf_sle_uuid_info 的指针
 */
#define PORT_SLE_SET_WS63_UUID_FROM_XF_UUID(ptr_ws63_uuid, ptr_xf_sle_uuid_info)    \
    (ptr_ws63_uuid)->len = (ptr_xf_sle_uuid_info)->type;                        \
    memcpy((ptr_ws63_uuid)->uuid, (ptr_xf_sle_uuid_info)->uuid128,                  \
            (ptr_xf_sle_uuid_info)->type)      

#define PORT_SLE_SET_XF_UUID_FROM_WS63_UUID(ptr_xf_sle_uuid_info, ptr_ws63_uuid)    \
    (ptr_xf_sle_uuid_info)->type = (ptr_ws63_uuid)->len;                        \
    memcpy((ptr_xf_sle_uuid_info)->uuid128, (ptr_ws63_uuid)->uuid,                  \
            (ptr_ws63_uuid)->len)  

/* ==================== [Typedefs] ========================================== */

/* ==================== [Global Prototypes] ================================= */

/* ==================== [Macros] ============================================ */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CONFIG_XF_SLE_ENABLE */

#endif /* __PORT_SLE_SSAP_H__ */
