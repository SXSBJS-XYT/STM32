/**
 * @file    adc_watchdog.h
 * @brief   ADC模拟看门狗 - 电压超限自动报警
 * @note    硬件: STM32F429IGT6
 *          监测PA0电压，超出阈值时触发中断
 */

#ifndef __ADC_WATCHDOG_H
#define __ADC_WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/**
 * @brief  报警类型
 */
typedef enum {
    AWD_ALARM_NONE = 0,    /* 无报警 */
    AWD_ALARM_HIGH,        /* 超上限 */
    AWD_ALARM_LOW,         /* 超下限 */
} AWD_AlarmType_t;

/**
 * @brief  看门狗状态
 */
typedef struct {
    uint16_t high_threshold;   /* 上限阈值 (原始值) */
    uint16_t low_threshold;    /* 下限阈值 (原始值) */
    float    high_voltage;     /* 上限电压 */
    float    low_voltage;      /* 下限电压 */
    uint32_t alarm_count;      /* 报警计数 */
    AWD_AlarmType_t last_alarm;/* 最后一次报警类型 */
} AWD_Status_t;

/**
 * @brief  初始化模拟看门狗
 * @param  hadc: ADC句柄
 */
void AWD_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  设置监测阈值 (电压值)
 * @param  low_voltage: 下限电压 (V)
 * @param  high_voltage: 上限电压 (V)
 */
void AWD_SetThreshold(float low_voltage, float high_voltage);

/**
 * @brief  设置监测阈值 (原始值)
 * @param  low_raw: 下限原始值 (0~4095)
 * @param  high_raw: 上限原始值 (0~4095)
 */
void AWD_SetThresholdRaw(uint16_t low_raw, uint16_t high_raw);

/**
 * @brief  启动监测
 */
void AWD_Start(void);

/**
 * @brief  停止监测
 */
void AWD_Stop(void);

/**
 * @brief  检查是否有报警
 */
bool AWD_IsAlarm(void);

/**
 * @brief  获取报警类型并清除报警标志
 */
AWD_AlarmType_t AWD_GetAlarmType(void);

/**
 * @brief  获取当前ADC值
 */
uint16_t AWD_GetCurrentValue(void);

/**
 * @brief  获取当前电压
 */
float AWD_GetCurrentVoltage(void);

/**
 * @brief  获取看门狗状态
 */
void AWD_GetStatus(AWD_Status_t *status);

/**
 * @brief  清除报警计数
 */
void AWD_ClearAlarmCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_WATCHDOG_H */
