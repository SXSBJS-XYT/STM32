/**
 * @file    adc_it.h
 * @brief   ADC中断方式采集
 * @note    硬件: STM32F429IGT6, PA0 (ADC1_IN0)
 */

#ifndef __ADC_IT_H
#define __ADC_IT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/**
 * @brief  ADC采集结果
 */
typedef struct {
    uint16_t raw;       /* 原始值 (0~4095) */
    float    voltage;   /* 电压值 (0~3.3V) */
} ADC_Result_t;

/**
 * @brief  初始化ADC中断采集
 * @param  hadc: ADC句柄
 */
void ADC_IT_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  触发一次ADC采集 (非阻塞)
 */
void ADC_IT_StartConversion(void);

/**
 * @brief  检查数据是否就绪
 * @retval true=有新数据
 */
bool ADC_IT_IsDataReady(void);

/**
 * @brief  获取采集结果 (读取后清除就绪标志)
 * @param  result: 结果输出
 * @retval true=成功
 */
bool ADC_IT_GetResult(ADC_Result_t *result);

/**
 * @brief  获取转换次数统计
 */
uint32_t ADC_IT_GetConversionCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_IT_H */
