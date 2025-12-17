/**
 * @file    adc_injected.c
 * @brief   ADC注入通道 - 高优先级采样
 *
 * 规则通道 vs 注入通道:
 * ======================
 *
 * 规则通道 (Regular):
 *   - 最多16个通道
 *   - 普通优先级
 *   - 共用1个数据寄存器 (DR)，需要DMA或及时读取
 *   - 适合：连续采集、批量采集
 *
 * 注入通道 (Injected):
 *   - 最多4个通道
 *   - 高优先级，可以打断规则通道
 *   - 每个通道独立数据寄存器 (JDR1~JDR4)，不会被覆盖
 *   - 适合：紧急采样、电机电流采样、保护触发
 *
 *
 * 打断机制:
 * ==========
 *
 * 规则通道正在转换:
 *   时间 →
 *   ┌─────────────────────────────────────────────────
 *   │ 规则CH0 │ 规则CH1 │ 规则CH2 │ 规则CH0 │ ...
 *   └─────────────────────────────────────────────────
 *
 *
 * 触发注入通道:
 *   时间 →                    ↓ 触发
 *   ┌────────────────────────┬──────────┬────────────
 *   │ 规则CH0 │ 规则CH1 │暂停│ 注入CH   │ 恢复规则...
 *   └────────────────────────┴──────────┴────────────
 *                             ↑          ↑
 *                        打断规则    注入完成后
 *                                   	自动恢复规则
 *
 * 重要提示：
 * - 注入完成后规则通道会自动恢复（手册RM0090 11.3.9节）
 * - 前提是中断回调快速返回，不要在回调里做printf等耗时操作
 * - 如果在回调里加printf，会干扰硬件自动恢复机制，导致规则通道停止
 *
 * 应用场景:
 * ==========
 * - 电机FOC控制：PWM中心触发采集相电流
 * - 过流保护：检测到过流立即采样
 * - 紧急测量：需要立即获取某个值
 */

#include "adc_injected.h"
#include <stdio.h>
#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;

static volatile uint16_t s_regular_value = 0;
static volatile uint16_t s_injected_value = 0;
static volatile bool     s_injected_ready = false;
static volatile uint32_t s_trigger_count = 0;

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void ADC_Inj_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_injected_ready = false;
    s_trigger_count = 0;
}

/**
 * @brief  启动规则通道连续转换
 */
void ADC_Inj_StartRegular(void)
{
    if (s_hadc == NULL) return;

    /* 启动规则通道 (连续模式，不用中断) */
    HAL_ADC_Start_IT(s_hadc);
}

/**
 * @brief  触发注入通道 (高优先级)
 */
void ADC_Inj_TriggerInjected(void)
{
    if (s_hadc == NULL) return;

    s_injected_ready = false;

    /* 启动注入通道转换 (会打断规则通道) */
    HAL_ADCEx_InjectedStart_IT(s_hadc);
}

/**
 * @brief  停止
 */
void ADC_Inj_Stop(void)
{
    if (s_hadc == NULL) return;

    HAL_ADCEx_InjectedStop_IT(s_hadc);
    HAL_ADC_Stop_IT(s_hadc);
}

/**
 * @brief  检查注入转换完成
 */
bool ADC_Inj_IsInjectedReady(void)
{
    return s_injected_ready;
}

/**
 * @brief  获取结果
 */
bool ADC_Inj_GetResult(ADC_Injected_Result_t *result)
{
    if (result == NULL) return false;

    /* 读取规则通道当前值 */
    s_regular_value = HAL_ADC_GetValue(s_hadc);

    result->regular_raw = s_regular_value;
    result->regular_voltage = (float)s_regular_value * ADC_VREF / ADC_MAX;
    result->injected_raw = s_injected_value;
    result->injected_voltage = (float)s_injected_value * ADC_VREF / ADC_MAX;

    s_injected_ready = false;

    return true;
}

/**
 * @brief  获取触发次数
 */
uint32_t ADC_Inj_GetTriggerCount(void)
{
    return s_trigger_count;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  注入通道转换完成回调
 * @note   注入转换完成后，规则通道会自动恢复
 */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc) {
        /* 读取注入通道值 (Rank 1) */
        s_injected_value = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
        s_injected_ready = true;
        s_trigger_count++;
			  /* 不需要手动重启规则通道，硬件会自动恢复 */
        /* 警告：如果在这里加printf等耗时操作，会干扰自动恢复机制！ */
    }
}
