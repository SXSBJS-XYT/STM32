/*
 * FAL 测试代码
 * 
 * 在 main.c 中调用 fal_test() 进行测试
 */
#include "fal_test.h"
#include <fal.h>
#include <stdio.h>
#include <string.h>

/**
 * FAL 基础功能测试
 */
void fal_test(void)
{
    const struct fal_partition *part = NULL;
    uint8_t write_buf[256];
    uint8_t read_buf[256];
    int i;
    int result;
    
    printf("\r\n============================================================\r\n");
    printf("                     FAL Test Start\r\n");
    printf("============================================================\r\n");
    
    /*----------------------------------------------------------------------
     * 1. 初始化 FAL
     *--------------------------------------------------------------------*/
    printf("\r\n[1] Initializing FAL...\r\n");
    result = fal_init();
    if (result < 0) {
        printf("[FAL] Init failed!\r\n");
        return;
    }
    printf("[FAL] Init success, found %d partition(s).\r\n", result);
    
    /*----------------------------------------------------------------------
     * 2. 打印分区表（fal_init 内部已经打印，这里可选）
     *--------------------------------------------------------------------*/
    // fal_show_part_table();
    
    /*----------------------------------------------------------------------
     * 3. 对 easyflash 分区进行读写测试
     *--------------------------------------------------------------------*/
    printf("\r\n[2] Testing 'easyflash' partition...\r\n");
    
    /* 查找分区 */
    part = fal_partition_find("easyflash");
    if (part == NULL) {
        printf("[FAL] ERROR: Partition 'easyflash' not found!\r\n");
        return;
    }
    printf("    Found partition: %s\r\n", part->name);
    printf("    Flash device:    %s\r\n", part->flash_name);
    printf("    Offset:          0x%08lX\r\n", part->offset);
    printf("    Size:            %lu bytes (%lu KB)\r\n", part->len, part->len / 1024);
    
    /* 准备测试数据 */
    for (i = 0; i < 256; i++) {
        write_buf[i] = i;
    }
    memset(read_buf, 0, 256);
    
    /* 擦除 */
    printf("\r\n[3] Erasing 4KB at offset 0...\r\n");
    result = fal_partition_erase(part, 0, 4096);
    if (result < 0) {
        printf("[FAL] ERROR: Erase failed!\r\n");
        return;
    }
    printf("    Erase OK, erased %d bytes.\r\n", result);
    
    /* 验证擦除后数据为 0xFF */
    printf("\r\n[4] Verifying erase (should be 0xFF)...\r\n");
    fal_partition_read(part, 0, read_buf, 16);
    printf("    Data[0:15]: ");
    for (i = 0; i < 16; i++) {
        printf("%02X ", read_buf[i]);
    }
    printf("\r\n");
    
    /* 写入 */
    printf("\r\n[5] Writing 256 bytes at offset 0...\r\n");
    result = fal_partition_write(part, 0, write_buf, 256);
    if (result < 0) {
        printf("[FAL] ERROR: Write failed!\r\n");
        return;
    }
    printf("    Write OK, wrote %d bytes.\r\n", result);
    
    /* 读取 */
    printf("\r\n[6] Reading 256 bytes at offset 0...\r\n");
    memset(read_buf, 0, 256);
    result = fal_partition_read(part, 0, read_buf, 256);
    if (result < 0) {
        printf("[FAL] ERROR: Read failed!\r\n");
        return;
    }
    printf("    Read OK, read %d bytes.\r\n", result);
    
    /* 显示部分数据 */
    printf("    Data[0:15]:   ");
    for (i = 0; i < 16; i++) {
        printf("%02X ", read_buf[i]);
    }
    printf("\r\n");
    printf("    Data[240:255]: ");
    for (i = 240; i < 256; i++) {
        printf("%02X ", read_buf[i]);
    }
    printf("\r\n");
    
    /* 校验 */
    printf("\r\n[7] Verifying data...\r\n");
    if (memcmp(write_buf, read_buf, 256) == 0) {
        printf("\r\n");
        printf("    ********************************************\r\n");
        printf("    *          FAL Test PASSED!                *\r\n");
        printf("    ********************************************\r\n");
    } else {
        printf("\r\n");
        printf("    ############################################\r\n");
        printf("    #          FAL Test FAILED!                #\r\n");
        printf("    ############################################\r\n");
        printf("    Data mismatch!\r\n");
    }
    
    printf("\r\n============================================================\r\n");
    printf("                     FAL Test End\r\n");
    printf("============================================================\r\n");
}
