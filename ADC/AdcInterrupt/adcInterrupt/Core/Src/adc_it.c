/**
 * @file    adc_it.c
 * @brief   ADC中断方式采集
 * 
 * 轮询 vs 中断:
 * ==============
 * 轮询方式:
 *   Start() → PollForConversion() → GetValue()
 *                  ↑
 *            CPU在这里等待
 * 
 * 中断方式:
 *   Start_IT() → CPU去做别的事 → 中断触发 → Callback()读取
 *                     ↑
 *               CPU不用等待！
 * 
 * 时序图:
 * ========
 *   主循环:  ──┬─ StartConversion() ─────────────────┬─ GetResult()
 *             │                                      │
 *   ADC:      └──→ [采样] [转换] ──→ EOC中断 ────────┘
 *                   (4μs左右)
 */

#include "adc_it.h"

#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;

static volatile uint16_t s_adc_raw = 0;        /* 原始值 */
static volatile bool     s_data_ready = false; /* 数据就绪标志 */
static volatile uint32_t s_conv_count = 0;     /* 转换计数 */

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void ADC_IT_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_data_ready = false;
    s_conv_count = 0;
}

/**
 * @brief  触发一次转换
 */
void ADC_IT_StartConversion(void)
{
    if (s_hadc != NULL) {
        HAL_ADC_Start_IT(s_hadc);
    }
}

/**
 * @brief  检查数据就绪
 */
bool ADC_IT_IsDataReady(void)
{
    return s_data_ready;
}

/**
 * @brief  获取结果
 */
bool ADC_IT_GetResult(ADC_Result_t *result)
{
    if (result == NULL || !s_data_ready) {
        return false;
    }
    
    result->raw = s_adc_raw;
    result->voltage = (float)s_adc_raw * ADC_VREF / ADC_MAX;
    
    s_data_ready = false;  /* 清除标志 */
    
    return true;
}

/**
 * @brief  获取转换次数
 */
uint32_t ADC_IT_GetConversionCount(void)
{
    return s_conv_count;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  ADC转换完成回调 (重写HAL WEAK函数)
 * @note   中断上下文，保持简短！
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc) {
        /* 读取转换结果 */
        s_adc_raw = HAL_ADC_GetValue(hadc);
        s_data_ready = true;
        s_conv_count++;
    }
}
