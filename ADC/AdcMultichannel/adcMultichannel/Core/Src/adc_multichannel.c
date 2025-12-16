/**
 * @file    adc_multichannel.c
 * @brief   ADC多通道扫描采集
 * 
 * 扫描模式原理:
 * ==============
 * 配置3个通道 (CH0, CH1, CH2)，扫描模式下ADC依次转换：
 * 
 *   时间 →
 *   ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
 *   │ CH0 │ CH1 │ CH2 │ CH0 │ CH1 │ CH2 │ CH0 │ CH1 │ CH2 │...
 *   └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
 *   │←─ 一轮扫描 ─→│←─ 一轮扫描 ─→│←─ 一轮扫描 ─→│
 * 
 * DMA缓冲区数据排列:
 * ===================
 *   Buffer[0]  = CH0 第1次采样
 *   Buffer[1]  = CH1 第1次采样
 *   Buffer[2]  = CH2 第1次采样
 *   Buffer[3]  = CH0 第2次采样
 *   Buffer[4]  = CH1 第2次采样
 *   Buffer[5]  = CH2 第2次采样
 *   ...
 * 
 * 提取第N通道数据:
 *   for (i = 0; i < SAMPLES; i++)
 *       ch_data[i] = Buffer[i * CHANNEL_COUNT + N]
 */

#include "adc_multichannel.h"
#include <string.h>

#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;

/* DMA缓冲区 */
static uint16_t s_adc_buffer[ADC_BUFFER_SIZE];

static volatile bool     s_data_ready = false;
static volatile uint32_t s_conv_count = 0;

/*============================================================================*/
/*                              私有函数                                       */
/*============================================================================*/

/**
 * @brief  计算单通道平均值
 * @param  channel: 通道索引
 * @return 平均值
 */
static uint16_t CalcChannelAverage(uint8_t channel)
{
    if (channel >= ADC_CHANNEL_COUNT) return 0;
    
    uint32_t sum = 0;
    
    /* 提取该通道的所有采样点 */
    for (uint32_t i = 0; i < ADC_SAMPLES_PER_CH; i++) {
        sum += s_adc_buffer[i * ADC_CHANNEL_COUNT + channel];
    }
    
    return sum / ADC_SAMPLES_PER_CH;
}

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void ADC_Multi_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_data_ready = false;
    s_conv_count = 0;
    memset(s_adc_buffer, 0, sizeof(s_adc_buffer));
}

/**
 * @brief  启动采集
 */
void ADC_Multi_Start(void)
{
    if (s_hadc == NULL) return;
    
    s_data_ready = false;
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_adc_buffer, ADC_BUFFER_SIZE);
}

/**
 * @brief  停止采集
 */
void ADC_Multi_Stop(void)
{
    if (s_hadc != NULL) {
        HAL_ADC_Stop_DMA(s_hadc);
    }
}

/**
 * @brief  检查数据就绪
 */
bool ADC_Multi_IsDataReady(void)
{
    return s_data_ready;
}

/**
 * @brief  获取所有通道结果
 */
bool ADC_Multi_GetResult(ADC_MultiResult_t *result)
{
    if (result == NULL || !s_data_ready) {
        result->valid = false;
        return false;
    }
    
    /* 计算每个通道的平均值 */
    for (uint8_t ch = 0; ch < ADC_CHANNEL_COUNT; ch++) {
        result->ch[ch].raw = CalcChannelAverage(ch);
        result->ch[ch].voltage = (float)result->ch[ch].raw * ADC_VREF / ADC_MAX;
    }
    
    result->valid = true;
    s_data_ready = false;
    
    return true;
}

/**
 * @brief  获取单通道结果
 */
bool ADC_Multi_GetChannelResult(uint8_t channel, ADC_ChannelResult_t *result)
{
    if (result == NULL || channel >= ADC_CHANNEL_COUNT || !s_data_ready) {
        return false;
    }
    
    result->raw = CalcChannelAverage(channel);
    result->voltage = (float)result->raw * ADC_VREF / ADC_MAX;
    
    return true;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  DMA传输完成回调
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc) {
        s_data_ready = true;
        s_conv_count++;
    }
}
