/**
 * @file    adc_timer_trigger.h
 * @brief   定时器触发ADC采集 - 精确采样率控制
 * @note    硬件: STM32F429IGT6
 *          TIM3触发ADC1，DMA传输
 */

#ifndef __ADC_TIMER_TRIGGER_H
#define __ADC_TIMER_TRIGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* 缓冲区大小 */
#define ADC_TT_BUFFER_SIZE    256

/**
 * @brief  初始化定时器触发ADC
 * @param  hadc: ADC句柄
 * @param  htim: 定时器句柄 (触发源)
 */
void ADC_TT_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim);

/**
 * @brief  启动采集
 */
void ADC_TT_Start(void);

/**
 * @brief  停止采集
 */
void ADC_TT_Stop(void);

/**
 * @brief  设置采样率
 * @param  sample_rate: 采样率 (Hz)
 * @param  tim_clock: 定时器时钟频率 (Hz)
 * @note   需要在Start之前调用
 */
void ADC_TT_SetSampleRate(uint32_t sample_rate, uint32_t tim_clock);

/**
 * @brief  检查数据就绪 (缓冲区满)
 */
bool ADC_TT_IsDataReady(void);

/**
 * @brief  获取缓冲区指针
 */
uint16_t* ADC_TT_GetBuffer(void);

/**
 * @brief  获取缓冲区大小
 */
uint32_t ADC_TT_GetBufferSize(void);

/**
 * @brief  计算平均值
 */
uint16_t ADC_TT_GetAverage(void);

/**
 * @brief  计算电压
 */
float ADC_TT_GetVoltage(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_TIMER_TRIGGER_H */
