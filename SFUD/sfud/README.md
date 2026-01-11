# SFUD 学习笔记

> Serial Flash Universal Driver 移植与原理分析  
> 平台：STM32F429IGT6 + W25Q256 + HAL库

---

## 一、SFUD 简介

SFUD（Serial Flash Universal Driver）是一个开源的串行 Flash 通用驱动库，由 RT-Thread 社区的 armink 开发维护。其核心价值在于通过 SFDP 标准实现 Flash 芯片的自动识别和配置。

### 1.1 核心特点

- **通用性强**：支持几乎所有常见 SPI Flash（W25Qxx、GD25Qxx、MX25Lxx 等）
- **SFDP 支持**：利用 JEDEC 标准自动读取 Flash 参数，实现即插即用
- **轻量级**：代码精简，适合资源受限的 MCU 环境
- **接口统一**：提供统一的读、写、擦除 API

### 1.2 主要 API

| 函数 | 功能描述 |
|------|----------|
| `sfud_init()` | 初始化 SFUD 库，识别所有 Flash 设备 |
| `sfud_get_device(index)` | 获取指定索引的 Flash 设备 |
| `sfud_read(flash, addr, size, data)` | 读取数据 |
| `sfud_write(flash, addr, size, data)` | 写入数据（不擦除） |
| `sfud_erase(flash, addr, size)` | 擦除指定区域 |
| `sfud_erase_write(flash, addr, size, data)` | 擦除后写入 |

---

## 二、移植要点

### 2.1 文件结构

| 文件 | 说明 | 是否需要修改 |
|------|------|--------------|
| sfud.c / sfud.h | 核心驱动实现 | 否 |
| sfud_sfdp.c | SFDP 参数解析 | 否 |
| sfud_def.h | 数据结构定义 | 否 |
| sfud_cfg.h | 配置文件 | 是（配置设备表） |
| sfud_port.c | 移植接口 | 是（实现 SPI 操作） |

### 2.2 移植接口实现

移植的核心是实现 **sfud_port.c** 中的几个关键函数：

#### spi_write_read() - SPI 读写函数

这是最关键的函数，每次调用代表一个完整的 CS 周期：

1. CS 拉低（选中 Flash）
2. 发送命令/地址数据（write_buf）
3. 接收返回数据（read_buf）
4. CS 拉高（释放 Flash）

#### lock/unlock - SPI 总线互斥

> ⚠️ **重要**：lock/unlock 是 SPI 总线互斥锁，用于 RTOS 多任务保护，**不是** CS 引脚控制！

| 环境 | lock/unlock 实现 |
|------|------------------|
| 裸机单任务 | 留空即可，无需互斥保护 |
| RTOS 多任务 | 使用互斥量（Mutex）保护 SPI 总线 |
| 裸机需要保护 | 可使用 `__disable_irq()` / `__enable_irq()` |

#### 常见移植错误

> ⚠️ **陷阱**：在 `lock()` 中使用 `__disable_irq()` 关中断后，`retry.delay()` 调用 `HAL_Delay()` 会死锁！因为 HAL_Delay 依赖 SysTick 中断。

**解决方案**：裸机环境下 lock/unlock 留空，或使用忙等待延时替代 HAL_Delay。

---

## 三、SFDP 机制详解

### 3.1 什么是 SFDP

SFDP（Serial Flash Discoverable Parameters）是 JEDEC 制定的标准（JESD216），规定了 Flash 芯片内部必须存储一张**只读参数表**，描述自身特性。

这实现了 Flash 的「自描述」能力——驱动程序无需预先知道芯片型号，只需读取 SFDP 表即可获取所有必要参数。

### 3.2 SFDP 数据结构

Flash 内部存储布局：

| 区域 | 地址 | 内容 |
|------|------|------|
| 用户数据区 | 0x000000 起 | 可读写擦除的存储空间 |
| SFDP Header | SFDP 0x00 | 签名 "SFDP" + 版本号 |
| Parameter Header | SFDP 0x08 | 参数表位置和长度 |
| Basic Parameter Table | SFDP 0x80（通常） | 容量、擦除指令、地址模式等 |

### 3.3 参数表包含的信息

Basic Flash Parameter Table（9 个 DWORD = 36 字节）包含：

- **容量信息**：Flash 总容量（字节数）
- **擦除类型**：支持的擦除大小（4KB/32KB/64KB）及对应命令
- **写入粒度**：页编程大小（通常 256 字节）
- **地址模式**：支持 3 字节 / 4 字节地址
- **状态寄存器**：易失性/非易失性特性

### 3.4 SFDP 读取流程

SFUD 通过发送 0x5A 命令读取 SFDP 数据，源码位于 **sfud_sfdp.c**：

1. **read_sfdp_header()**：验证 "SFDP" 签名，获取版本号
2. **read_basic_header()**：获取参数表的位置指针和长度
3. **read_basic_table()**：解析参数表，提取容量、擦除命令等

---

## 四、Flash 物理特性

### 4.1 存储原理

SPI Flash 内部使用**浮栅晶体管（Floating Gate）**存储数据，通过控制浮栅中的电子数量来表示 0 或 1。

- **写入（编程）**：通过高压将电子注入浮栅，是物理隧穿过程
- **擦除**：通过更高电压将电子从浮栅释放，同样是物理过程
- **读取**：检测浮栅电荷状态，几乎瞬时完成

### 4.2 操作耗时对比

| 操作 | 物理过程 | 典型耗时 | 能否通过提高 SPI 速度加快 |
|------|----------|----------|---------------------------|
| 读取 | 检测电荷 | 纳秒级 | ✅ 是（数据传输更快） |
| 页写入 | 电子隧穿注入 | 0.4 ~ 3 ms | ❌ 否（物理过程决定） |
| 扇区擦除 (4KB) | 电子隧穿释放 | 50 ~ 400 ms | ❌ 否（物理过程决定） |
| 块擦除 (64KB) | 电子隧穿释放 | 150 ~ 2000 ms | ❌ 否（物理过程决定） |

> ✅ **结论**：提高 SPI 时钟速度只能加快读取操作和命令传输，无法加快擦除和写入。因为这些操作 99.99% 的时间是在等待 Flash 内部完成物理过程。

### 4.3 擦除等待机制

擦除命令发出后，MCU 需要轮询 Flash 状态寄存器等待完成：

1. 发送擦除命令（0x20 + 地址）—— 几微秒
2. Flash 内部执行擦除 —— 50~400 毫秒
3. MCU 轮询状态寄存器（0x05 命令）等待 BUSY 位清零

---

## 五、地址模式

### 5.1 3 字节 vs 4 字节地址

| 地址模式 | 地址范围 | 最大寻址容量 |
|----------|----------|--------------|
| 3 字节 | 0x000000 ~ 0xFFFFFF | 16 MB (2²⁴ 字节) |
| 4 字节 | 0x00000000 ~ 0xFFFFFFFF | 4 GB (2³² 字节) |

### 5.2 W25Q256 的地址问题

W25Q256 容量为 32MB，超过了 3 字节地址的 16MB 上限：

- 使用 3 字节地址：只能访问前 16MB，后 16MB 无法寻址
- 使用 4 字节地址：可以访问全部 32MB 空间

SFUD 检测到 W25Q256 后，会自动发送 **0xB7** 命令进入 4 字节地址模式。

---

## 六、调试信息解析

### 6.1 JEDEC ID

```
manufacturer ID is 0xEF, memory type ID is 0x40, capacity ID is 0x19
```

| 字段 | 值 | 含义 |
|------|-----|------|
| Manufacturer ID | 0xEF | Winbond（华邦） |
| Memory Type ID | 0x40 | SPI Flash 系列标识 |
| Capacity ID | 0x19 | 容量代码（2^25 = 32MB） |

### 6.2 SFDP 解析结果

| 参数 | 值 | 说明 |
|------|-----|------|
| 4KB Erase | 命令 0x20 | 扇区擦除 |
| 32KB Erase | 命令 0x52 | 小块擦除 |
| 64KB Erase | 命令 0xD8 | 大块擦除 |
| Write granularity | 64 bytes+ | 页编程粒度 |
| Capacity | 33554432 Bytes | 32MB |
| Addressing | 3- or 4-Byte | 支持两种地址模式 |

---

## 附录：常见问题

### Q1: 初始化成功但擦除卡死

**原因**：`lock()` 中使用 `__disable_irq()` 关闭中断，导致 `HAL_Delay()` 依赖的 SysTick 停止工作。

**解决**：裸机环境下 lock/unlock 函数留空。

### Q2: 读取的 JEDEC ID 全是 0xFF 或 0x00

**原因**：SPI 通信失败，可能是时钟过快、引脚配置错误、CS 控制问题。

**解决**：降低 SPI 分频系数，检查 GPIO 配置，确认 CS 在 spi_write_read 中正确控制。

### Q3: 容量超过 16MB 的 Flash 只能访问一半

**原因**：Flash 仍在 3 字节地址模式。

**解决**：确认 SFUD 是否成功进入 4 字节地址模式，检查日志中是否有 "Enter 4-Byte addressing mode success"。
