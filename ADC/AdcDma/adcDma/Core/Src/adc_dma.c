/**
 * @file    adc_dma.c
 * @brief   ADC DMA方式采集
 * 
 * 三种方式对比:
 * ==============
 * 
 * 轮询:   CPU启动 → CPU等待 → CPU读取
 *              ↑        ↑        ↑
 *            CPU      CPU      CPU
 * 
 * 中断:   CPU启动 → 硬件转换 → 中断 → CPU读取
 *              ↑                        ↑
 *            CPU                      CPU
 * 
 * DMA:    CPU启动 → 硬件转换 → DMA搬运 → 中断通知
 *              ↑                           ↑
 *            CPU                     仅通知一次
 *                    (N次转换期间CPU完全空闲)
 * 
 * 
 * DMA工作流程:
 * =============
 * 
 *   ADC ──转换完成──→ DMA ──搬运──→ Buffer[0]
 *   ADC ──转换完成──→ DMA ──搬运──→ Buffer[1]
 *   ADC ──转换完成──→ DMA ──搬运──→ Buffer[2]
 *   ...
 *   ADC ──转换完成──→ DMA ──搬运──→ Buffer[N-1]
 *                              ↓
 *                     DMA传输完成中断
 *                     (循环模式下自动重新开始)
 */

#include "adc_dma.h"
#include <string.h>

#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;

/* DMA缓冲区 */
static uint16_t s_adc_buffer[ADC_DMA_BUF_SIZE];

static volatile bool     s_data_ready = false;     /* 数据就绪标志 */
static volatile uint32_t s_complete_count = 0;     /* 完成计数 */

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void ADC_DMA_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_data_ready = false;
    s_complete_count = 0;
    memset(s_adc_buffer, 0, sizeof(s_adc_buffer));
}

/**
 * @brief  启动DMA采集
 */
void ADC_DMA_Start(void)
{
    if (s_hadc == NULL) return;
    
    s_data_ready = false;
    
    /* 启动ADC DMA */
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_adc_buffer, ADC_DMA_BUF_SIZE);
}

/**
 * @brief  停止采集
 */
void ADC_DMA_Stop(void)
{
    if (s_hadc != NULL) {
        HAL_ADC_Stop_DMA(s_hadc);
    }
}

/**
 * @brief  检查数据就绪
 */
bool ADC_DMA_IsDataReady(void)
{
    return s_data_ready;
}

/**
 * @brief  获取结果 (计算统计值)
 */
bool ADC_DMA_GetResult(ADC_DMA_Result_t *result)
{
    if (result == NULL || !s_data_ready) {
        return false;
    }
    
    uint32_t sum = 0;
    uint16_t min_val = 4095;
    uint16_t max_val = 0;
    
    /* 遍历缓冲区 */
    for (uint32_t i = 0; i < ADC_DMA_BUF_SIZE; i++) {
        uint16_t val = s_adc_buffer[i];
        sum += val;
        
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }
    
    /* 计算结果 */
    result->raw = sum / ADC_DMA_BUF_SIZE;
    result->voltage = (float)result->raw * ADC_VREF / ADC_MAX;
    result->min = min_val;
    result->max = max_val;
    
    s_data_ready = false;  /* 清除标志 */
    
    return true;
}

/**
 * @brief  获取原始缓冲区
 */
uint16_t* ADC_DMA_GetBuffer(void)
{
    return s_adc_buffer;
}

/**
 * @brief  获取完成计数
 */
uint32_t ADC_DMA_GetCompleteCount(void)
{
    return s_complete_count;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  ADC DMA传输完成回调
 * @note   整个缓冲区填满后才触发一次
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc) {
        s_data_ready = true;
        s_complete_count++;
    }
}
