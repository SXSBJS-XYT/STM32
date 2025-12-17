/**
 * @file    adc_injected.h
 * @brief   ADC注入通道 - 高优先级采样
 * @note    硬件: STM32F429IGT6
 *          规则通道: PA0 (连续采集)
 *          注入通道: PA1 (高优先级，可打断规则通道)
 */

#ifndef __ADC_INJECTED_H
#define __ADC_INJECTED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/**
 * @brief  ADC结果
 */
typedef struct {
    uint16_t regular_raw;      /* 规则通道原始值 */
    float    regular_voltage;  /* 规则通道电压 */
    uint16_t injected_raw;     /* 注入通道原始值 */
    float    injected_voltage; /* 注入通道电压 */
} ADC_Injected_Result_t;

/**
 * @brief  初始化
 * @param  hadc: ADC句柄
 */
void ADC_Inj_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  启动规则通道连续转换
 */
void ADC_Inj_StartRegular(void);

/**
 * @brief  触发注入通道转换 (高优先级，打断规则通道)
 */
void ADC_Inj_TriggerInjected(void);

/**
 * @brief  停止所有转换
 */
void ADC_Inj_Stop(void);

/**
 * @brief  检查注入转换是否完成
 */
bool ADC_Inj_IsInjectedReady(void);

/**
 * @brief  获取结果
 * @param  result: 结果输出
 */
bool ADC_Inj_GetResult(ADC_Injected_Result_t *result);

/**
 * @brief  获取注入通道触发次数
 */
uint32_t ADC_Inj_GetTriggerCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_INJECTED_H */
