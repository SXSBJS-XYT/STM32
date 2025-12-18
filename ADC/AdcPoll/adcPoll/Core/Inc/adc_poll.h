/**
 * @file    adc_poll.h
 * @brief   ADC单通道轮询采集
 * @note    硬件: STM32F429IGT6, PA0 (ADC1_IN0)
 */

#ifndef __ADC_POLL_H
#define __ADC_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @brief  ADC采集结果
 */
typedef struct {
    uint16_t raw;       /* 原始值 (0~4095) */
    float    voltage;   /* 电压值 (0~3.3V) */
} ADC_Result_t;

/**
 * @brief  单次ADC采集 (轮询方式)
 * @param  hadc: ADC句柄
 * @param  result: 结果输出
 * @retval HAL_OK=成功
 */
HAL_StatusTypeDef ADC_Poll_Read(ADC_HandleTypeDef *hadc, ADC_Result_t *result);

/**
 * @brief  多次采集取平均 (滤波)
 * @param  hadc: ADC句柄
 * @param  result: 结果输出
 * @param  samples: 采样次数 (建议8/16/32)
 * @retval HAL_OK=成功
 */
HAL_StatusTypeDef ADC_Poll_ReadAverage(ADC_HandleTypeDef *hadc, ADC_Result_t *result, uint8_t samples);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_POLL_H */
