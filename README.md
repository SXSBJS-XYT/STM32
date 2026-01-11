# STM32 深度理解系列
> A thorough understanding of the STM32 microcontroller

本仓库包含我的**复盘笔记**(每个工程下的README文件)和**示例工程**，主要基于 STM32F429IGT6 开发板，使用 CubeMX 6.15.0 + Keil MDK 开发环境。

---

## 📈 更新计划

- [√] 高级Timer
- [√] ADC (模数转换器)
- [ ] DAC (数模转换器)
- [ ] CAN
- [√] LowPower (低功耗模式)
- [√] SFUD (串行Flash通用驱动库)
- [√] FAL (Flash Abstraction Layer - Flash 抽象层)
- [ ] Bootloader
---

## 📁 仓库结构

```

STM32/
├── 高级定时器/          # 高级定时器相关工程
│   ├── ComplementaryOutputDeadZoneBraking/    # 输出互补 + 死区 + 刹车
│   ├── OutputCompare/        # 输出比较
│   ├── PulseOutput/          # 单脉冲输出
|   └── PwmInput/             # PWM输入
│
├── ADC/                # ADC相关工程
│   ├── AdcPoll/              # 轮询模式
│   ├── AdcInterrupt/         # 中断模式
│   ├── AdcDma/               # DMA模式
│   ├── AdcMultichannel/      # 多通道扫描
│   ├── AdcTimerTrigger/      # 定时器触发采样
│   ├── AdcInjected/          # 注入通道
│   └── AdcWatchdog/          # 模拟看门狗
|
├── LowPower/           # 低功耗相关工程
│   ├── SleepMode/            # 睡眠模式
│   └── StopMode/             # stop模式
|
├── SFUD/               # 串行Flash通用驱动库工程
│   └── sfud/                 # sfud 裸机Demo工程
|
├── FAL/                # FAL抽象层工程
│   └── fal/                  # fal 裸机Demo工程
|
└── README.md           # 本文件

```
---

## ⏱️ 高级定时器

STM32F4的高级定时器(TIM1/TIM8)功能强大，支持PWM生成、输入捕获、输出比较、死区控制、刹车保护等功能。

| 工程                                                                                              | 功能                   | 关键知识点                       |
| ------------------------------------------------------------------------------------------------- | ---------------------- | -------------------------------- |
| [PwmInput](Advanced-Control-Timers/PwmInput/)                                                     | PWM输入捕获            | 同时测量PWM频率和占空比          |
| [OutputCompare](Advanced-Control-Timers/OutputCompare/)                                           | 输出比较模式           | 精确时间控制，翻转/置位/复位模式 |
| [PulseOutput](Advanced-Control-Timers/PulseOutput/)                                               | 单脉冲输出             | OPM模式，精确脉冲宽度控制        |
| [ComplementaryOutputDeadZoneBraking](Advanced-Control-Timers/ComplementaryOutputDeadZoneBraking/) | 输出互补 + 死区 + 刹车 | 主要应用在电机控制               |

---

## 📊 ADC (模数转换器)

STM32F4的ADC支持12位分辨率、多通道扫描、DMA传输、注入通道、模拟看门狗等高级功能。

| 工程                                    | 功能       | 关键知识点                 |
| --------------------------------------- | ---------- | -------------------------- |
| [AdcPoll](ADC/AdcPoll/)                 | 轮询模式   | ADC基础，手动读取转换结果  |
| [AdcInterrupt](ADC/AdcInterrupt/)       | 中断模式   | EOC中断，转换完成通知      |
| [AdcDma](ADC/AdcDma/)                   | DMA模式    | 零CPU干预，高效数据传输    |
| [AdcMultichannel](ADC/AdcMultichannel/) | 多通道扫描 | 扫描模式，自动切换通道     |
| [AdcTimerTrigger](ADC/AdcTimerTrigger/) | 定时器触发 | 精确采样率，硬件同步       |
| [AdcInjected](ADC/AdcInjected/)         | 注入通道   | 高优先级采样，打断规则通道 |
| [AdcWatchdog](ADC/AdcWatchdog/)         | 模拟看门狗 | 电压超限自动报警           |


### ADC模式选择指南

| 场景              | 推荐模式   | 原因           |
| ----------------- | ---------- | -------------- |
| 偶尔读取单个值    | 轮询       | 简单直接       |
| 需要及时响应      | 中断       | 事件驱动       |
| 高速连续采集      | DMA        | CPU零负担      |
| 多传感器采集      | DMA+多通道 | 自动扫描       |
| 精确采样率要求    | 定时器触发 | 硬件保证时序   |
| 紧急/高优先级采样 | 注入通道   | 可打断规则通道 |
| 过压/欠压保护     | 模拟看门狗 | 硬件自动检测   |

---
## 🪫LowPower (低功耗)

| 模式 | 功耗 | CPU | 时钟 | RAM | 唤醒源 | 唤醒后 |
|------|------|-----|------|-----|--------|--------|
| Sleep | mA | 停 | 运行 | 保持 | 任意中断 | 直接继续 |
| Stop | μA | 停 | **停** | 保持 | **EXTI** | **重配时钟** |
| Standby | 最低 | 停 | 停 | **丢失** | WKUP/RTC | **复位重启** |
---

## 📂SFUD
- 通用性强：支持几乎所有常见 SPI Flash（W25Qxx、GD25Qxx、MX25Lxx 等）
- SFDP 支持：利用 JEDEC 标准自动读取 Flash 参数，实现即插即用
- 轻量级：代码精简，适合资源受限的 MCU 环境
- 接口统一：提供统一的读、写、擦除 API
---

 ## 💽FAL
 FAL（Flash Abstraction Layer）是 Flash 抽象层，由 armink 开发，作用是：

- 分区管理：将一整块 Flash 划分为多个逻辑分区，按名称访问
- 统一接口：屏蔽不同 Flash 的差异（片内/片外），提供统一 API
- 安全隔离：各分区互不干扰，防止误操作
  
**FAL 的价值体现在哪里？**

单独使用 FAL 时，价值确实不明显。FAL 只是一层"中间件"，它的真正价值在于：

| 场景 | 没有 FAL | 有 FAL |
| ----------------- | -------------- | -------------- |
| FlashDB | 需要自己管理 Flash 地址，容易冲突 | 直接指定分区名，自动隔离 |
| OTA 升级 | 手动计算 app/download 区地址 | 按分区名操作，代码清晰 |
| 多 Flash 设备 | 每个设备单独管理，接口不统一 | 统一 API，透明访问 |
| 固件迁移 | 换 Flash 需要改大量地址 | 只改分区表，应用层不变 |
| 调试测试 | 需要自己写测试代码 | FAL 自带 Shell 测试命令（RT-Thread）|

**类比理解：**

- SFUD 相当于"硬盘驱动"
- FAL 相当于"分区表"（类似 Windows 的 C/D/E 盘）
- FlashDB 相当于"数据库软件"

单独看分区表没意义，但有了分区表，上层软件才能安全地共存。

---
## 🛠️ 开发环境

- **MCU**: STM32F429IGT6
- **IDE**: Keil MDK V5.x
- **配置工具**: STM32CubeMX--V6.15.0
- **HAL库版本**: STM32Cube_FW_F4--V1.28.3

---

## 📚 参考资料

- [STM32F4xx参考手册 (RM0090)](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F4xx HAL库用户手册](https://www.st.com/resource/en/user_manual/um1725-description-of-stm32f4-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
- [Cortex-M4权威指南](https://developer.arm.com/documentation/100166/0001)

---

## 📄 License

[MIT License](LICENSE) - 随意使用，欢迎Star⭐
