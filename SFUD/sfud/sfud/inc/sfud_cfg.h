/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2016-04-23
 */

#ifndef _SFUD_CFG_H_
#define _SFUD_CFG_H_

/*==============================================================================
 * 调试配置
 *============================================================================*/

/* 调试模式（通过串口打印日志）*/
#define SFUD_DEBUG_MODE

/*==============================================================================
 * Flash 参数获取方式配置
 *============================================================================*/

/* 使用 SFDP 自动读取 Flash 参数（推荐开启，W25Q256 支持 SFDP）*/
#define SFUD_USING_SFDP

/* Flash 信息表（SFDP 失败时的备选方案）*/
#define SFUD_USING_FLASH_INFO_TABLE

/*==============================================================================
 * 读取模式配置
 *============================================================================*/

/* 快速读取模式（使用 0x0B 命令，需要 dummy byte）
 * 注意：如果 SPI 时钟较高（>50MHz），建议开启
 * 我们先使用普通读取模式，调通后再考虑开启 */
// #define SFUD_USING_FAST_READ

/* QSPI 模式（我们使用普通 SPI，不需要开启）*/
// #define SFUD_USING_QSPI

/*==============================================================================
 * Flash 设备表配置
 *============================================================================*/

/* Flash 设备索引枚举 */
enum {
    SFUD_W25Q256_DEVICE_INDEX = 0,
};

/* Flash 设备信息表
 * 格式：{.name = "设备名称", .spi.name = "SPI名称"}
 * 这里的 name 仅用于调试打印，可以自定义
 */
#define SFUD_FLASH_DEVICE_TABLE                                                \
{                                                                              \
    [SFUD_W25Q256_DEVICE_INDEX] = {.name = "W25Q256", .spi.name = "SPI5"},     \
}

#endif /* _SFUD_CFG_H_ */
