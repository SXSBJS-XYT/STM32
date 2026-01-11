/*
 * FAL (Flash Abstraction Layer) - SFUD 对接层
 * 
 * 基于官方 fal_flash_sfud_port.c 修改
 * 适配裸机环境（无 RT-Thread）
 * 
 * 平台：STM32F429IGT6 + W25Q256
 */

#include <fal.h>
#include <sfud.h>

#ifdef FAL_USING_SFUD_PORT

#ifndef FAL_USING_NOR_FLASH_DEV_NAME
#define FAL_USING_NOR_FLASH_DEV_NAME    "W25Q256"
#endif

/*==============================================================================
 * 私有变量
 *============================================================================*/

static sfud_flash_t sfud_dev = NULL;

/*==============================================================================
 * 操作函数声明
 *============================================================================*/

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

/*==============================================================================
 * Flash 设备对象定义
 *============================================================================*/

/*
 * W25Q256 Flash 设备
 * 
 * 注意：len 和 blk_size 会在 init() 中通过 SFUD 自动更新
 * 这里的值是初始默认值
 */
struct fal_flash_dev nor_flash0 =
{
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 32 * 1024 * 1024,   /* 32MB，会被 init 更新 */
    .blk_size   = 4096,                /* 4KB，会被 init 更新 */
    .ops        = {init, read, write, erase},
    .write_gran = 1                    /* NOR Flash 写粒度：1 bit */
};

/*==============================================================================
 * 操作函数实现
 *============================================================================*/

/**
 * 初始化 Flash 设备
 * 
 * 注意：调用此函数前，SFUD 必须已经初始化完成（sfud_init() 已调用）
 */
static int init(void)
{
    /* 
     * 裸机环境：通过设备索引获取 SFUD 设备
     * SFUD_W25Q256_DEVICE_INDEX 定义在 sfud_cfg.h 中
     */
    sfud_dev = sfud_get_device(SFUD_W25Q256_DEVICE_INDEX);
    
    if (NULL == sfud_dev)
    {
        return -1;
    }
    
    if (!sfud_dev->init_ok)
    {
        return -1;
    }

    /* 从 SFUD 获取实际的 Flash 参数，更新设备信息 */
    nor_flash0.blk_size = sfud_dev->chip.erase_gran;
    nor_flash0.len = sfud_dev->chip.capacity;

    return 0;
}

/**
 * 读取数据
 * 
 * @param offset 相对于 Flash 设备起始地址的偏移量
 * @param buf    读取缓冲区
 * @param size   读取大小
 * @return       实际读取的字节数，失败返回负数
 */
static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    
    if (sfud_read(sfud_dev, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

/**
 * 写入数据
 * 
 * @param offset 相对于 Flash 设备起始地址的偏移量
 * @param buf    写入数据缓冲区
 * @param size   写入大小
 * @return       实际写入的字节数，失败返回负数
 * 
 * 注意：写入前需确保目标区域已擦除
 */
static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    
    if (sfud_write(sfud_dev, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

/**
 * 擦除数据
 * 
 * @param offset 相对于 Flash 设备起始地址的偏移量
 * @param size   擦除大小
 * @return       实际擦除的字节数，失败返回负数
 * 
 * 注意：offset 和 size 应与擦除块大小对齐
 */
static int erase(long offset, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    
    if (sfud_erase(sfud_dev, nor_flash0.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

#endif /* FAL_USING_SFUD_PORT */
