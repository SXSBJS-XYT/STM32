/**
 * @file    adc_multichannel.h
 * @brief   ADC多通道扫描采集 (DMA方式)
 * @note    硬件: STM32F429IGT6
 *          通道: PA0(IN0), PA1(IN1), PA2(IN2)
 */

#ifndef __ADC_MULTICHANNEL_H
#define __ADC_MULTICHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* 通道数量 */
#define ADC_CHANNEL_COUNT   3

/* 每通道采样次数 (用于平均) */
#define ADC_SAMPLES_PER_CH  16

/* 总缓冲区大小 */
#define ADC_BUFFER_SIZE     (ADC_CHANNEL_COUNT * ADC_SAMPLES_PER_CH)

/**
 * @brief  单通道结果
 */
typedef struct {
    uint16_t raw;       /* 原始值/平均值 */
    float    voltage;   /* 电压值 */
} ADC_ChannelResult_t;

/**
 * @brief  多通道结果
 */
typedef struct {
    ADC_ChannelResult_t ch[ADC_CHANNEL_COUNT];
    bool valid;
} ADC_MultiResult_t;

/**
 * @brief  初始化多通道ADC
 * @param  hadc: ADC句柄
 */
void ADC_Multi_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  启动采集
 */
void ADC_Multi_Start(void);

/**
 * @brief  停止采集
 */
void ADC_Multi_Stop(void);

/**
 * @brief  检查数据就绪
 */
bool ADC_Multi_IsDataReady(void);

/**
 * @brief  获取所有通道结果
 * @param  result: 结果输出
 */
bool ADC_Multi_GetResult(ADC_MultiResult_t *result);

/**
 * @brief  获取单个通道结果
 * @param  channel: 通道索引 (0, 1, 2...)
 * @param  result: 结果输出
 */
bool ADC_Multi_GetChannelResult(uint8_t channel, ADC_ChannelResult_t *result);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_MULTICHANNEL_H */
