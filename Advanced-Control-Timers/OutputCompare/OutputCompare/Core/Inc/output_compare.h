/**
 * @file    output_compare.h
 * @brief   输出比较功能演示 - 不等间隔脉冲
 * @note    硬件: STM32F429IGT6
 * 
 * 输出比较的真正价值：每次匹配都能动态改变下一次间隔
 * PWM做不到这一点（PWM每个周期都一样）
 */

#ifndef __OUTPUT_COMPARE_H
#define __OUTPUT_COMPARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/**
 * @brief  初始化输出比较
 * @param  htim: 定时器句柄
 * @param  Channel: 通道
 */
void OC_Init(TIM_HandleTypeDef *htim, uint32_t Channel);

/**
 * @brief  演示1: 步进电机梯形加减速
 *         脉冲间隔: 2000→1000→500→500→1000→2000 (微秒)
 *         效果: 慢→快→慢
 */
void OC_Demo_StepperAccel(void);

/**
 * @brief  演示2: 自定义不等间隔脉冲序列
 * @param  intervals: 间隔数组 (单位: 定时器计数)
 * @param  count: 脉冲数量
 */
void OC_Demo_CustomPulse(uint32_t *intervals, uint32_t count);

/**
 * @brief  停止输出
 */
void OC_Stop(void);

/**
 * @brief  检查是否完成
 */
bool OC_IsFinished(void);

#ifdef __cplusplus
}
#endif

#endif /* __OUTPUT_COMPARE_H */
