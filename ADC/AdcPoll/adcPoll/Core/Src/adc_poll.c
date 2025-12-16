/**
 * @file    adc_poll.c
 * @brief   ADC单通道轮询采集
 * 
 * 原理说明:
 * =========
 * 轮询方式流程:
 * 
 *   HAL_ADC_Start()      启动转换
 *         ↓
 *   HAL_ADC_PollForConversion()  等待转换完成
 *         ↓
 *   HAL_ADC_GetValue()   读取结果
 *         ↓
 *   HAL_ADC_Stop()       停止ADC
 * 
 * 12位ADC: 0~4095 对应 0~3.3V
 * 电压 = raw * 3.3 / 4095
 */

#include "adc_poll.h"

#define ADC_VREF    3.3f      /* 参考电压 */
#define ADC_MAX     4095.0f   /* 12位最大值 */
#define ADC_TIMEOUT 100       /* 超时时间(ms) */

/**
 * @brief  单次ADC采集
 */
HAL_StatusTypeDef ADC_Poll_Read(ADC_HandleTypeDef *hadc, ADC_Result_t *result)
{
    HAL_StatusTypeDef status;
    
    if (result == NULL) return HAL_ERROR;
    
    /* 启动转换 */
    status = HAL_ADC_Start(hadc);
    if (status != HAL_OK) return status;
    
    /* 等待转换完成 */
    status = HAL_ADC_PollForConversion(hadc, ADC_TIMEOUT);
    if (status != HAL_OK) {
        HAL_ADC_Stop(hadc);
        return status;
    }
    
    /* 读取结果 */
    result->raw = HAL_ADC_GetValue(hadc);
    result->voltage = (float)result->raw * ADC_VREF / ADC_MAX;
    
    /* 停止ADC */
    HAL_ADC_Stop(hadc);
    
    return HAL_OK;
}

/**
 * @brief  多次采集取平均
 */
HAL_StatusTypeDef ADC_Poll_ReadAverage(ADC_HandleTypeDef *hadc, ADC_Result_t *result, uint8_t samples)
{
    HAL_StatusTypeDef status;
    uint32_t sum = 0;
    
    if (result == NULL || samples == 0) return HAL_ERROR;
    
    for (uint8_t i = 0; i < samples; i++) {
        /* 启动转换 */
        status = HAL_ADC_Start(hadc);
        if (status != HAL_OK) return status;
        
        /* 等待完成 */
        status = HAL_ADC_PollForConversion(hadc, ADC_TIMEOUT);
        if (status != HAL_OK) {
            HAL_ADC_Stop(hadc);
            return status;
        }
        
        /* 累加 */
        sum += HAL_ADC_GetValue(hadc);
        
        HAL_ADC_Stop(hadc);
    }
    
    /* 计算平均 */
    result->raw = sum / samples;
    result->voltage = (float)result->raw * ADC_VREF / ADC_MAX;
    
    return HAL_OK;
}
