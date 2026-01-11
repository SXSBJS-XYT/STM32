/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 * 
 * 修改说明：适配 STM32F429 裸机环境
 */

#ifndef _FAL_DEF_H_
#define _FAL_DEF_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAL_SW_VERSION                 "0.5.99"

/*==============================================================================
 * 平台适配
 *============================================================================*/

#ifdef __RTTHREAD__ /* for RT-Thread platform */
#include <rtthread.h>
#define FAL_PRINTF      rt_kprintf
#define FAL_MALLOC      rt_malloc
#define FAL_CALLOC      rt_calloc
#define FAL_REALLOC     rt_realloc
#define FAL_FREE        rt_free
#else /* 裸机平台 */
#define FAL_PRINTF      printf
#define FAL_MALLOC      malloc
#define FAL_CALLOC      calloc
#define FAL_REALLOC     realloc
#define FAL_FREE        free
#endif

/*==============================================================================
 * 调试配置
 *============================================================================*/

#ifndef FAL_DEBUG
#define FAL_DEBUG                      1    /* 默认开启调试输出 */
#endif

#if FAL_DEBUG

/* assert 断言 */
#ifdef assert
#undef assert
#endif
#define assert(EXPR)                                                           \
if (!(EXPR))                                                                   \
{                                                                              \
    FAL_PRINTF("(%s) has assert failed at %s.\r\n", #EXPR, __func__);          \
    while (1);                                                                 \
}

/* debug 级别日志 */
#ifdef log_d
#undef log_d
#endif
#define log_d(...)                     FAL_PRINTF("[D/FAL] ");FAL_PRINTF(__VA_ARGS__);FAL_PRINTF("\r\n")

#else /* !FAL_DEBUG */

#ifdef assert
#undef assert
#endif
#define assert(EXPR)                   ((void)0)

#ifdef log_d
#undef log_d
#endif
#define log_d(...)

#endif /* FAL_DEBUG */

/* error 级别日志（始终开启）*/
#ifdef log_e
#undef log_e
#endif
#define log_e(...)                     FAL_PRINTF("[E/FAL] ");FAL_PRINTF(__VA_ARGS__);FAL_PRINTF("\r\n")

/* info 级别日志（始终开启）*/
#ifdef log_i
#undef log_i
#endif
#define log_i(...)                     FAL_PRINTF("[I/FAL] ");FAL_PRINTF(__VA_ARGS__);FAL_PRINTF("\r\n")

/*==============================================================================
 * 数据结构定义
 *============================================================================*/

/* FAL flash and partition device name max length */
#ifndef FAL_DEV_NAME_MAX
#define FAL_DEV_NAME_MAX 24
#endif

/**
 * Flash 设备结构体
 */
struct fal_flash_dev
{
    char name[FAL_DEV_NAME_MAX];

    /* flash device start address and len  */
    uint32_t addr;
    size_t len;
    /* the block size in the flash for erase minimum granularity */
    size_t blk_size;

    struct
    {
        int (*init)(void);
        int (*read)(long offset, uint8_t *buf, size_t size);
        int (*write)(long offset, const uint8_t *buf, size_t size);
        int (*erase)(long offset, size_t size);
    } ops;

    /* write minimum granularity, unit: bit. 
       1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1)/ 64(stm32l4)
       0 will not take effect. */
    size_t write_gran;
};
typedef struct fal_flash_dev *fal_flash_dev_t;

/**
 * FAL 分区结构体
 */
struct fal_partition
{
    uint32_t magic_word;

    /* partition name */
    char name[FAL_DEV_NAME_MAX];
    /* flash device name for partition */
    char flash_name[FAL_DEV_NAME_MAX];

    /* partition offset address on flash device */
    long offset;
    size_t len;

    uint32_t reserved;
};
typedef struct fal_partition *fal_partition_t;

/* 分区表魔数 */
#define FAL_PART_MAGIC_WORD         0x45503130

#endif /* _FAL_DEF_H_ */
