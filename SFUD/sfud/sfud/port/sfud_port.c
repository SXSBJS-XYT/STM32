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
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include <stdio.h>
#include "spi.h"
#include "main.h"

static char log_buf[256];

/* SPI 超时时间 (ms) */
#define SPI_TIMEOUT     1000

void sfud_log_debug(const char *file, const long line, const char *format, ...);

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    //uint8_t send_data, read_data;

    /**
     * add your spi write and read code
     */
    HAL_StatusTypeDef hal_status;
    
    /* 获取 SPI 句柄 */
    SPI_HandleTypeDef *spi_handle = (SPI_HandleTypeDef *)spi->user_data;
    
    /* CS 拉低，选中 Flash */
    HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_RESET);
    
    /* 发送数据 */
    if (write_size > 0) {
        hal_status = HAL_SPI_Transmit(spi_handle, (uint8_t *)write_buf, write_size, SPI_TIMEOUT);
        if (hal_status != HAL_OK) {
						printf("[DBG] SPI Transmit failed: %d\r\n", hal_status);
            result = SFUD_ERR_WRITE;
            goto exit;
        }
    }
    
    /* 接收数据 */
    if (read_size > 0) {
        hal_status = HAL_SPI_Receive(spi_handle, read_buf, read_size, SPI_TIMEOUT);
        if (hal_status != HAL_OK) {
						printf("[DBG] SPI Receive failed: %d\r\n", hal_status); 
            result = SFUD_ERR_READ;
            goto exit;
        }
    }

exit:
    /* CS 拉高，释放 Flash */
    HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_SET);
    
    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

/**
 * SPI 总线锁定（用于 RTOS 多任务环境）
 * 裸机环境下留空
 */
static void spi_lock(const sfud_spi *spi) {
    /* 裸机环境下留空，无需关中断 */
    (void)spi;
}

static void spi_unlock(const sfud_spi *spi) {
    /* 裸机环境下留空 */
    (void)spi;
}

/**
 * 重试延时函数
 */
static void retry_delay(void) {
    HAL_Delay(1);
}

sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
		/*
     * 1. SPI 和 GPIO 已由 CubeMX 生成的代码初始化 (MX_SPI5_Init, MX_GPIO_Init)
     *    这里不需要再初始化硬件
     */
    
    /* 2. 确保 CS 引脚初始状态为高电平（不选中） */
    HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_SET);
    
    /* 3. 绑定 SPI 接口函数 */
    flash->spi.wr = spi_write_read;           /* 必须：SPI 读写函数 */
    flash->spi.lock = spi_lock;               /* 可选：SPI 总线锁定 */
    flash->spi.unlock = spi_unlock;           /* 可选：SPI 总线解锁 */
    flash->spi.user_data = &hspi5;            /* 传递 SPI 句柄，供 wr 函数使用 */
    
#ifdef SFUD_USING_QSPI
    flash->spi.qspi_read = qspi_read;         /* QSPI 模式时需要 */
#endif

    /* 4. 绑定重试参数 */
    flash->retry.delay = retry_delay;         /* 重试延时函数 */
    flash->retry.times = 10000;               /* 必须：重试次数 */
		
    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}
