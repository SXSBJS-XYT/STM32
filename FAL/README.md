# FAL 移植学习笔记

> Flash Abstraction Layer 移植与分析
> 平台：STM32F429IGT6 + W25Q256 + 裸机（无 RTOS）

---

## 一、FAL 是什么

FAL（Flash Abstraction Layer）是 Flash 抽象层，由 armink 开发，作用是：

1. **分区管理**：将一整块 Flash 划分为多个逻辑分区，按名称访问
2. **统一接口**：屏蔽不同 Flash 的差异（片内/片外），提供统一 API
3. **安全隔离**：各分区互不干扰，防止误操作

### 1.1 架构层次

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层                                  │
│            FlashDB / OTA / 文件系统 / 用户程序               │
├─────────────────────────────────────────────────────────────┤
│                      FAL 分区层                              │
│   fal_partition_read() / fal_partition_write() / erase()    │
│                                                             │
│   ┌──────────┬──────────┬──────────┬────────────────┐      │
│   │bootloader│   app    │easyflash │    filesys     │      │
│   │  64KB    │  960KB   │   1MB    │     29MB       │      │
│   └──────────┴──────────┴──────────┴────────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      FAL 设备层                              │
│              fal_flash_dev (nor_flash0)                     │
├─────────────────────────────────────────────────────────────┤
│                      SFUD 驱动层                             │
│         sfud_read() / sfud_write() / sfud_erase()           │
├─────────────────────────────────────────────────────────────┤
│                      SPI 硬件层                              │
│                    HAL_SPI_Transmit()                       │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 FAL 的价值体现在哪里？

**单独使用 FAL 时，价值确实不明显**。FAL 只是一层"中间件"，它的真正价值在于：

| 场景 | 没有 FAL | 有 FAL |
|------|----------|--------|
| **FlashDB** | 需要自己管理 Flash 地址，容易冲突 | 直接指定分区名，自动隔离 |
| **OTA 升级** | 手动计算 app/download 区地址 | 按分区名操作，代码清晰 |
| **多 Flash 设备** | 每个设备单独管理，接口不统一 | 统一 API，透明访问 |
| **固件迁移** | 换 Flash 需要改大量地址 | 只改分区表，应用层不变 |
| **调试测试** | 需要自己写测试代码 | FAL 自带 Shell 测试命令（RT-Thread） |

**类比理解**：
- SFUD 相当于"硬盘驱动"
- FAL 相当于"分区表"（类似 Windows 的 C/D/E 盘）
- FlashDB 相当于"数据库软件"

单独看分区表没意义，但有了分区表，上层软件才能安全地共存。

---

## 二、移植过程

### 2.1 文件来源

FAL 源码来自 FlashDB 项目：
```
https://github.com/armink/FlashDB
└── port/fal/
    ├── inc/
    │   ├── fal.h
    │   └── fal_def.h
    └── src/
        ├── fal.c
        ├── fal_flash.c
        └── fal_partition.c
```

### 2.2 工程文件结构

```
工程目录/
├── Core/
│   ├── Src/
│   │   ├── main.c
│   │   └── fal_test.c          ← 测试代码
│   └── Inc/
├── SFUD/                        ← 已有
│   └── ...
└── FAL/                         ← 新增
    ├── inc/
    │   ├── fal.h               ← 官方原文件
    │   ├── fal_def.h           ← 修改：裸机适配
    │   └── fal_cfg.h           ← 新建：分区配置
    ├── src/
    │   ├── fal.c               ← 官方原文件
    │   ├── fal_flash.c         ← 官方原文件
    │   └── fal_partition.c     ← 官方原文件
    └── port/
        └── fal_flash_sfud_port.c  ← 修改：SFUD 对接
```

### 2.3 需要修改的文件

| 文件 | 修改说明 |
|------|----------|
| `fal_cfg.h` | 新建，定义 Flash 设备表和分区表 |
| `fal_def.h` | 去掉 ANSI 颜色码，添加裸机 printf/malloc 适配 |
| `fal_flash_sfud_port.c` | 去掉 RT-Thread 依赖，改用 `sfud_get_device()` |

### 2.4 关键配置：fal_cfg.h

```c
/* 启用静态分区表 */
#define FAL_PART_HAS_TABLE_CFG

/* 启用 SFUD 对接 */
#define FAL_USING_SFUD_PORT

/* Flash 设备名称 */
#define NOR_FLASH_DEV_NAME  "W25Q256"

/* Flash 设备表 */
extern struct fal_flash_dev nor_flash0;
#define FAL_FLASH_DEV_TABLE  { &nor_flash0, }

/* 分区表（32MB 总空间）*/
#define FAL_PART_TABLE                                                              \
{                                                                                   \
    {FAL_PART_MAGIC_WORD, "bootloader", NOR_FLASH_DEV_NAME, 0x000000,   64*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "app",        NOR_FLASH_DEV_NAME, 0x010000,  960*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "easyflash",  NOR_FLASH_DEV_NAME, 0x100000, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "download",   NOR_FLASH_DEV_NAME, 0x200000, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "filesys",    NOR_FLASH_DEV_NAME, 0x300000, 29*1024*1024, 0}, \
}
```

### 2.5 分区规划图

```
W25Q256 (32MB) 地址空间：

地址        大小      分区名        用途
────────────────────────────────────────────────────────
0x000000    64KB     bootloader   引导程序（OTA 预留）
0x010000   960KB     app          应用程序固件
0x100000     1MB     easyflash    FlashDB 键值存储
0x200000     1MB     download     OTA 下载缓存
0x300000    29MB     filesys      文件系统（LittleFS）
────────────────────────────────────────────────────────
                     合计 32MB
```

---

## 三、核心代码解析

### 3.1 FAL 初始化流程

```c
int fal_init(void)
{
    fal_flash_init();      // 初始化所有 Flash 设备
    fal_partition_init();  // 初始化分区表
}
```

### 3.2 分区操作 API

```c
/* 查找分区 */
const struct fal_partition *part = fal_partition_find("easyflash");

/* 读取数据（相对分区起始地址）*/
fal_partition_read(part, offset, buf, size);

/* 写入数据 */
fal_partition_write(part, offset, buf, size);

/* 擦除数据 */
fal_partition_erase(part, offset, size);

/* 擦除整个分区 */
fal_partition_erase_all(part);
```

### 3.3 地址转换

FAL 自动处理地址转换：

```
用户调用：fal_partition_write(easyflash_part, 0, data, 256)
                                              ↓
FAL 内部：offset = part->offset + addr = 0x100000 + 0 = 0x100000
                                              ↓
SFUD 调用：sfud_write(flash, 0x100000, 256, data)
```

用户只需关心分区内的相对地址，无需记忆物理地址。

---

## 四、SFUD 与 FAL 对接要点

### 4.1 设备获取方式

```c
/* 裸机环境 */
sfud_dev = sfud_get_device(SFUD_W25Q256_DEVICE_INDEX);

/* RT-Thread 环境 */
sfud_dev = rt_sfud_flash_find_by_dev_name("W25Q256");
```

### 4.2 参数自动更新

FAL 在初始化时会从 SFUD 获取实际的 Flash 参数：

```c
static int init(void)
{
    sfud_dev = sfud_get_device(SFUD_W25Q256_DEVICE_INDEX);

    /* 从 SFUD 更新实际参数 */
    nor_flash0.blk_size = sfud_dev->chip.erase_gran;  // 4096
    nor_flash0.len = sfud_dev->chip.capacity;          // 33554432

    return 0;
}
```

---

## 五、调用顺序

```c
int main(void)
{
    /* 硬件初始化 */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI5_Init();
    MX_USART1_UART_Init();

    /* 1. 先初始化 SFUD（底层驱动）*/
    sfud_init();

    /* 2. 再初始化 FAL（依赖 SFUD）*/
    fal_init();

    /* 3. 后续可以初始化 FlashDB（依赖 FAL）*/
    // fdb_kvdb_init(...);

    while (1) { }
}
```

**依赖关系**：`SPI → SFUD → FAL → FlashDB`

---

## 六、测试结果

```
========== System Start ==========
[SFUD] Find a Winbond flash chip. Size: 32MB
[SFUD] W25Q256 initialized successfully.
SFUD init OK.

[I/FAL] Flash Abstraction Layer (V0.5.99) initialize success.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name       | flash_dev |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | bootloader | W25Q256   | 0x00000000 | 0x00010000 |
[I/FAL] | app        | W25Q256   | 0x00010000 | 0x000f0000 |
[I/FAL] | easyflash  | W25Q256   | 0x00100000 | 0x00100000 |
[I/FAL] | download   | W25Q256   | 0x00200000 | 0x00100000 |
[I/FAL] | filesys    | W25Q256   | 0x00300000 | 0x01d00000 |
[I/FAL] =============================================================

[FAL] Testing 'easyflash' partition...
[FAL] Erase OK, Write OK, Read OK, Verify OK.

>>> FAL Test PASSED! <<<
```

---

## 七、后续学习路线

FAL 已就绪，接下来可以：

```
当前位置：SFUD + FAL ✓
    │
    ├─► FlashDB（推荐下一步）
    │   ├── KVDB：键值数据库，存储配置参数
    │   └── TSDB：时序数据库，存储传感器日志
    │
    ├─► LittleFS 文件系统
    │   └── 挂载到 filesys 分区，支持文件操作
    │
    └─► OTA 固件升级
        └── app + download 分区配合实现
```

---

## 八、常见问题

### Q1: FAL 初始化失败，提示找不到 Flash 设备

**原因**：SFUD 未初始化，或设备名不匹配。

**解决**：
1. 确保 `sfud_init()` 在 `fal_init()` 之前调用
2. 检查 `fal_cfg.h` 中的设备名与 `fal_flash_sfud_port.c` 一致

### Q2: 分区表显示正常，但读写失败

**原因**：分区偏移或大小超出 Flash 范围。

**解决**：检查分区表，确保所有分区在 Flash 容量范围内。

### Q3: 编译报错 `undefined reference to 'printf'`

**原因**：裸机环境未实现 printf 重定向。

**解决**：在 main.c 中添加：
```c
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

---

## 附录：移植文件清单

| 文件 | 来源 | 是否修改 |
|------|------|----------|
| fal.h | FlashDB/port/fal/inc/ | 否 |
| fal_def.h | FlashDB/port/fal/inc/ | 是（裸机适配） |
| fal_cfg.h | FlashDB/port/fal/samples/ | 是 |
| fal.c | FlashDB/port/fal/src/ | 否 |
| fal_flash.c | FlashDB/port/fal/src/ | 否 |
| fal_partition.c | FlashDB/port/fal/src/ | 否 |
| fal_flash_sfud_port.c | FlashDB/port/fal/samples/ | 是（裸机适配） |
| fal_test.c | 新建 | - |
