/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for wal_regdb.c.
 */

#ifndef __WAL_REGDB_H__
#define __WAL_REGDB_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    char *country_list; /* 国家列表 */
    struct ieee80211_reg_rule reg_rule; /* 管制域规则 */
} wal_country_reg_list_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
const oal_ieee80211_regdomain_stru* wal_regdb_find_db_etc(const osal_s8 *country_code);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of wal_regdb.h */
