/**
 * @file    adc_watchdog.c
 * @brief   ADC模拟看门狗 - 电压超限自动报警
 * 
 * 模拟看门狗功能:
 * ================
 * 
 * ADC转换完成后，硬件自动比较结果与阈值:
 * 
 *   High Threshold (HTR)  ─────────────────  上限
 *                              ↑
 *                         正常范围
 *                              ↓
 *   Low Threshold (LTR)   ─────────────────  下限
 * 
 * 
 * 当 ADC值 > HTR 或 ADC值 < LTR 时:
 *   → AWD标志置位
 *   → 如果使能了AWD中断，触发中断
 * 
 *
 * 重要配置:
 * ==========
 * CubeMX必须配置 Continuous Conversion Mode = Enabled
 * 否则ADC只转换一次就停止，AWD也只能触发一次
 * 
 *
 * 应用场景:
 * ==========
 * - 过压/欠压保护
 * - 电池电量监测
 * - 温度超限报警
 * - 电流过载保护
 * 
 * 
 * 优势:
 * ======
 * - 硬件自动比较，无需软件轮询
 * - 响应速度快（ADC转换完成后立即判断）
 * - CPU零负担
 */

#include "adc_watchdog.h"

#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;

static volatile bool     s_alarm_flag = false;
static volatile uint32_t s_alarm_count = 0;
static volatile AWD_AlarmType_t s_alarm_type = AWD_ALARM_NONE;

static uint16_t s_high_threshold = 4095;
static uint16_t s_low_threshold = 0;

/*============================================================================*/
/*                              私有函数                                       */
/*============================================================================*/

/**
 * @brief  电压转原始值
 */
static uint16_t VoltageToRaw(float voltage)
{
    if (voltage <= 0) return 0;
    if (voltage >= ADC_VREF) return 4095;
    return (uint16_t)(voltage / ADC_VREF * ADC_MAX);
}

/**
 * @brief  原始值转电压
 */
static float RawToVoltage(uint16_t raw)
{
    return (float)raw * ADC_VREF / ADC_MAX;
}

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

void AWD_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc = hadc;
    s_alarm_flag = false;
    s_alarm_count = 0;
    s_alarm_type = AWD_ALARM_NONE;
}

/**
 * @brief  设置阈值 (电压)
 */
void AWD_SetThreshold(float low_voltage, float high_voltage)
{
    s_low_threshold = VoltageToRaw(low_voltage);
    s_high_threshold = VoltageToRaw(high_voltage);
    
    if (s_hadc != NULL) {
        /* 配置看门狗阈值寄存器 */
        /* 注意：必须在ADC停止时配置，或使用HAL函数 */
         ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
         AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
         AnalogWDGConfig.HighThreshold = s_high_threshold;
         AnalogWDGConfig.LowThreshold = s_low_threshold;
         AnalogWDGConfig.Channel = ADC_CHANNEL_0;
         AnalogWDGConfig.ITMode = ENABLE;
         HAL_ADC_AnalogWDGConfig(s_hadc, &AnalogWDGConfig);
    }
}

/**
 * @brief  设置阈值 (原始值)
 */
void AWD_SetThresholdRaw(uint16_t low_raw, uint16_t high_raw)
{
    s_low_threshold = low_raw;
    s_high_threshold = high_raw;
    
    if (s_hadc != NULL) {
        s_hadc->Instance->HTR = s_high_threshold;
        s_hadc->Instance->LTR = s_low_threshold;
    }
}

/**
 * @brief  启动监测
 */
void AWD_Start(void)
{
    if (s_hadc == NULL) return;
    
    s_alarm_flag = false;
    
    /* 启动ADC (连续模式) */
    HAL_ADC_Start_IT(s_hadc);
    
    /* 使能AWD中断 */
    __HAL_ADC_ENABLE_IT(s_hadc, ADC_IT_AWD);
}

/**
 * @brief  停止监测
 */
void AWD_Stop(void)
{
    if (s_hadc == NULL) return;
    
    __HAL_ADC_DISABLE_IT(s_hadc, ADC_IT_AWD);
    HAL_ADC_Stop(s_hadc);
}

/**
 * @brief  检查报警
 */
bool AWD_IsAlarm(void)
{
    return s_alarm_flag;
}

/**
 * @brief  获取报警类型并清除
 */
AWD_AlarmType_t AWD_GetAlarmType(void)
{
    AWD_AlarmType_t type = s_alarm_type;
    s_alarm_flag = false;
    s_alarm_type = AWD_ALARM_NONE;
    return type;
}

/**
 * @brief  获取当前ADC值
 */
uint16_t AWD_GetCurrentValue(void)
{
    if (s_hadc == NULL) return 0;
    return s_hadc->Instance->DR;
}

/**
 * @brief  获取当前电压
 */
float AWD_GetCurrentVoltage(void)
{
    return RawToVoltage(AWD_GetCurrentValue());
}

/**
 * @brief  获取状态
 */
void AWD_GetStatus(AWD_Status_t *status)
{
    if (status == NULL) return;
    
    status->high_threshold = s_high_threshold;
    status->low_threshold = s_low_threshold;
    status->high_voltage = RawToVoltage(s_high_threshold);
    status->low_voltage = RawToVoltage(s_low_threshold);
    status->alarm_count = s_alarm_count;
    status->last_alarm = s_alarm_type;
}

/**
 * @brief  清除报警计数
 */
void AWD_ClearAlarmCount(void)
{
    s_alarm_count = 0;
}

/*============================================================================*/
/*                              HAL回调函数                                    */
/*============================================================================*/

/**
 * @brief  模拟看门狗中断回调
 * @note   当ADC值超出阈值范围时触发
 */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == s_hadc) {
        uint16_t value = hadc->Instance->DR;
        
        /* 判断是超上限还是超下限 */
        if (value > s_high_threshold) {
            s_alarm_type = AWD_ALARM_HIGH;
        } else if (value < s_low_threshold) {
            s_alarm_type = AWD_ALARM_LOW;
        }
        
        s_alarm_flag = true;
        s_alarm_count++;
        /* 注意：不要在这里加printf等耗时操作！ */
    }
}
