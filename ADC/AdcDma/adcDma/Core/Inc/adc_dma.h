/**
 * @file    adc_dma.h
 * @brief   ADC DMA方式采集
 * @note    硬件: STM32F429IGT6, PA0 (ADC1_IN0)
 *          DMA自动搬运数据，CPU零负担
 */

#ifndef __ADC_DMA_H
#define __ADC_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* 采样缓冲区大小 */
#define ADC_DMA_BUF_SIZE    64

/**
 * @brief  ADC采集结果
 */
typedef struct {
    uint16_t raw;       /* 原始值/平均值 (0~4095) */
    float    voltage;   /* 电压值 (0~3.3V) */
    uint16_t min;       /* 最小值 */
    uint16_t max;       /* 最大值 */
} ADC_DMA_Result_t;

/**
 * @brief  初始化ADC DMA
 * @param  hadc: ADC句柄
 */
void ADC_DMA_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  启动DMA采集 (循环模式)
 */
void ADC_DMA_Start(void);

/**
 * @brief  停止DMA采集
 */
void ADC_DMA_Stop(void);

/**
 * @brief  检查数据是否就绪 (缓冲区满)
 */
bool ADC_DMA_IsDataReady(void);

/**
 * @brief  获取采集结果 (自动计算平均值)
 * @param  result: 结果输出
 */
bool ADC_DMA_GetResult(ADC_DMA_Result_t *result);

/**
 * @brief  获取原始缓冲区指针 (高级用法)
 */
uint16_t* ADC_DMA_GetBuffer(void);

/**
 * @brief  获取转换完成次数
 */
uint32_t ADC_DMA_GetCompleteCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_DMA_H */
