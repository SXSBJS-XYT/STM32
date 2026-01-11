/*
 * FAL (Flash Abstraction Layer) 配置文件
 * 
 * 平台：STM32F429IGT6 + W25Q256 (裸机)
 * 修改自官方示例
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <stdint.h>
#include <stddef.h>

/*==============================================================================
 * 基础配置
 *============================================================================*/

/* 启用静态分区表配置（在本文件中定义分区表）*/
#define FAL_PART_HAS_TABLE_CFG

/* 启用 SFUD 驱动对接 */
#define FAL_USING_SFUD_PORT

/*==============================================================================
 * Flash 设备配置
 *============================================================================*/

/* 外部 NOR Flash 设备名称 */
#define NOR_FLASH_DEV_NAME              "W25Q256"
#define FAL_USING_NOR_FLASH_DEV_NAME    NOR_FLASH_DEV_NAME

/* ===================== Flash device Configuration ========================= */
/* 声明 Flash 设备（在 fal_flash_sfud_port.c 中定义）*/
extern struct fal_flash_dev nor_flash0;

/* Flash 设备表 */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &nor_flash0,                                                     \
}

/*==============================================================================
 * 分区表配置
 *============================================================================*/

/*
 * W25Q256 分区规划（32MB 总空间）
 * 
 * |   分区名    |  起始地址  |   大小   |         用途           |
 * |-------------|-----------|----------|------------------------|
 * | bootloader  | 0x000000  |   64KB   | 引导程序（预留）         |
 * | app         | 0x010000  |  960KB   | 应用程序固件（预留）      |
 * | easyflash   | 0x100000  |    1MB   | FlashDB / EasyFlash 区  |
 * | download    | 0x200000  |    1MB   | OTA 下载缓存区          |
 * | filesys     | 0x300000  |   29MB   | 文件系统区              |
 * 
 * 总计：64KB + 960KB + 1MB + 1MB + 29MB = 32MB
 */

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG

/* 分区表定义 
 * 格式：{FAL_PART_MAGIC_WORD, "分区名", "Flash设备名", 偏移地址, 分区大小, 0}
 */
#define FAL_PART_TABLE                                                              \
{                                                                                   \
    {FAL_PART_MAGIC_WORD, "bootloader", NOR_FLASH_DEV_NAME, 0x00000000,   64*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "app",        NOR_FLASH_DEV_NAME, 0x00010000,  960*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "easyflash",  NOR_FLASH_DEV_NAME, 0x00100000, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "download",   NOR_FLASH_DEV_NAME, 0x00200000, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "filesys",    NOR_FLASH_DEV_NAME, 0x00300000, 29*1024*1024, 0}, \
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
