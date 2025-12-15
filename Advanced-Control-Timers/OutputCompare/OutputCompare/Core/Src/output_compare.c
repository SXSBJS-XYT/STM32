/**
 * @file    output_compare.c
 * @brief   输出比较功能演示 - 不等间隔脉冲
 * 
 * 核心区别:
 * =========
 * PWM模式:    每个周期间隔相同，硬件自动循环
 * 输出比较:   每次匹配可设置不同间隔，通过中断动态控制
 * 
 * 本DEMO演示:
 * ===========
 * 步进电机梯形加减速曲线
 * 
 *   脉冲间隔
 *     ↑
 * 2ms │■
 *     │  ■
 * 1ms │    ■
 *     │      ■ ■
 * 0.5ms        ■ ■
 *     │            ■
 *     │              ■
 *     └──────────────────→ 脉冲序号
 *       加速   匀速   减速
 * 
 * 这种变间隔输出，PWM模式无法实现
 */

#include "output_compare.h"
#include <string.h>
/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static TIM_HandleTypeDef *s_htim = NULL;
static uint32_t s_channel = 0;

/* 脉冲序列 */
static uint32_t s_intervals[64];   /* 间隔数组 */
static uint32_t s_pulse_count = 0; /* 总脉冲数 */
static volatile uint32_t s_current_index = 0;  /* 当前索引 */
static volatile bool s_finished = true;        /* 完成标志 */

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void OC_Init(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    s_htim = htim;
    s_channel = Channel;
    s_finished = true;
}

/**
 * @brief  演示: 步进电机梯形加减速
 */
void OC_Demo_StepperAccel(void)
{
    /*
     * 梯形速度曲线:
     * - 加速段: 间隔从大到小 (速度从慢到快)
     * - 匀速段: 间隔不变
     * - 减速段: 间隔从小到大 (速度从快到慢)
     * 
     * 间隔单位: 定时器计数 (假设1MHz，则1000=1ms)
     */
    uint32_t accel_intervals[] = {
        /* 加速段: 慢→快 */
        2000,   /* 2ms - 最慢 */
        1600,
        1200,
        800,
        500,    /* 0.5ms - 最快 */
        
        /* 匀速段 */
        500,
        500,
        500,
        500,
        
        /* 减速段: 快→慢 */
        800,
        1200,
        1600,
        2000,   /* 2ms - 停止 */
    };
    
    OC_Demo_CustomPulse(accel_intervals, sizeof(accel_intervals) / sizeof(accel_intervals[0]));
}

/**
 * @brief  自定义脉冲序列
 */
void OC_Demo_CustomPulse(uint32_t *intervals, uint32_t count)
{
    if (s_htim == NULL || intervals == NULL || count == 0) return;
    if (count > 64) count = 64;
    
    /* 复制间隔数组 */
    memcpy(s_intervals, intervals, count * sizeof(uint32_t));
    s_pulse_count = count;
    s_current_index = 0;
    s_finished = false;
    
    /* 设置第一个比较值 */
    uint32_t cnt = __HAL_TIM_GET_COUNTER(s_htim);
    __HAL_TIM_SET_COMPARE(s_htim, s_channel, cnt + s_intervals[0]);
    
    /* 启动输出比较 */
    HAL_TIM_OC_Start_IT(s_htim, s_channel);
}

/**
 * @brief  停止
 */
void OC_Stop(void)
{
    if (s_htim != NULL) {
        HAL_TIM_OC_Stop_IT(s_htim, s_channel);
    }
    s_finished = true;
}

/**
 * @brief  检查是否完成
 */
bool OC_IsFinished(void)
{
    return s_finished;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  输出比较匹配回调
 * @note   每次匹配后，设置下一个不同的间隔 —— 这是PWM做不到的！
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim != s_htim) return;
    
    /* 检查通道 */
    bool channel_match = false;
    if (s_channel == TIM_CHANNEL_1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        channel_match = true;
    } else if (s_channel == TIM_CHANNEL_2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        channel_match = true;
    } else if (s_channel == TIM_CHANNEL_3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        channel_match = true;
    } else if (s_channel == TIM_CHANNEL_4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
        channel_match = true;
    }
    
    if (!channel_match) return;
    
    /* 移动到下一个脉冲 */
    s_current_index++;
    
    if (s_current_index >= s_pulse_count) {
        /* 序列完成 */
        OC_Stop();
        return;
    }
    
    /* 关键点: 设置下一个间隔 (每次可以不同！) */
    uint32_t current_ccr = __HAL_TIM_GET_COMPARE(s_htim, s_channel);
    uint32_t next_interval = s_intervals[s_current_index];
    __HAL_TIM_SET_COMPARE(s_htim, s_channel, current_ccr + next_interval);
}
