/**
 * @file    adc_timer_trigger.c
 * @brief   定时器触发ADC采集
 * 
 * 为什么需要定时器触发？
 * =======================
 * 
 * 软件触发的问题:
 * ---------------
 *   while(1) {
 *       HAL_ADC_Start();        // 启动
 *       HAL_ADC_PollForConversion();  // 等待
 *       value = HAL_ADC_GetValue();
 *       HAL_Delay(1);           // 延时1ms → 目标1kHz采样
 *   }
 * 
 *   问题：实际采样间隔 = 转换时间 + 代码执行时间 + 延时
 *         不是精确的1ms！会有抖动！
 * 
 * 
 * 定时器触发的优势:
 * -----------------
 *   TIM3 每1ms产生TRGO信号
 *         ↓
 *   ADC收到触发，自动开始转换
 *         ↓
 *   DMA自动搬运数据
 * 
 *   精确1kHz，与CPU无关！
 * 
 * 
 * 时序对比:
 * =========
 * 
 * 软件触发 (不精确):
 *   │  1.02ms │ 0.98ms │  1.05ms │ 0.97ms │  ← 有抖动
 *   ↓         ↓        ↓         ↓
 *   [ADC]     [ADC]    [ADC]     [ADC]
 * 
 * 
 * 定时器触发 (精确):
 *   │  1.00ms │  1.00ms │  1.00ms │  1.00ms │  ← 完美等间隔
 *   ↓         ↓         ↓         ↓
 *   [ADC]     [ADC]     [ADC]     [ADC]
 *   ↑         ↑         ↑         ↑
 *   TIM3      TIM3      TIM3      TIM3  (硬件触发)
 */

#include "adc_timer_trigger.h"
#include <string.h>

#define ADC_VREF    3.3f
#define ADC_MAX     4095.0f

/*============================================================================*/
/*                              私有变量                                       */
/*============================================================================*/

static ADC_HandleTypeDef *s_hadc = NULL;
static TIM_HandleTypeDef *s_htim = NULL;

/* DMA缓冲区 */
static uint16_t s_adc_buffer[ADC_TT_BUFFER_SIZE];

static volatile bool s_data_ready = false;

/*============================================================================*/
/*                              公共函数                                       */
/*============================================================================*/

/**
 * @brief  初始化
 */
void ADC_TT_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
    s_hadc = hadc;
    s_htim = htim;
    s_data_ready = false;
    memset(s_adc_buffer, 0, sizeof(s_adc_buffer));
}

/**
 * @brief  设置采样率
 */
void ADC_TT_SetSampleRate(uint32_t sample_rate, uint32_t tim_clock)
{
    if (s_htim == NULL || sample_rate == 0) return;
    
    /*
     * 定时器频率 = tim_clock / (PSC+1) / (ARR+1)
     * 采样率 = 定时器溢出频率
     * 
     * 例: tim_clock = 90MHz, 目标1kHz
     *     PSC = 89, ARR = 999
     *     频率 = 90MHz / (89 + 1) / 1000 = 1kHz
     */
    
    uint32_t period = tim_clock / sample_rate;
    uint16_t psc = 0;
    uint16_t arr = period - 1;
    
    /* 如果period太大，增加预分频 */
    while (arr > 65535) {
        psc++;
        arr = (period / (psc + 1)) - 1;
    }
    
    __HAL_TIM_SET_PRESCALER(s_htim, psc);
    __HAL_TIM_SET_AUTORELOAD(s_htim, arr);
}

/**
 * @brief  启动采集
 */
void ADC_TT_Start(void)
{
    if (s_hadc == NULL || s_htim == NULL) return;

    /* 先停止 */
    HAL_TIM_Base_Stop(s_htim);
    HAL_ADC_Stop_DMA(s_hadc);
    
    /* 重置定时器计数器 */
    __HAL_TIM_SET_COUNTER(s_htim, 0);
	
    s_data_ready = false;
    
    /* 启动ADC DMA (等待触发) */
    HAL_ADC_Start_DMA(s_hadc, (uint32_t *)s_adc_buffer, ADC_TT_BUFFER_SIZE);
    
    /* 启动定时器 (产生触发信号) */
    HAL_TIM_Base_Start(s_htim);
}

/**
 * @brief  停止采集
 */
void ADC_TT_Stop(void)
{
    if (s_htim != NULL) {
        HAL_TIM_Base_Stop(s_htim);
    }
    if (s_hadc != NULL) {
        HAL_ADC_Stop_DMA(s_hadc);
    }
}

/**
 * @brief  检查数据就绪
 */
bool ADC_TT_IsDataReady(void)
{
    return s_data_ready;
}

/**
 * @brief  获取缓冲区
 */
uint16_t* ADC_TT_GetBuffer(void)
{
    return s_adc_buffer;
}

/**
 * @brief  获取缓冲区大小
 */
uint32_t ADC_TT_GetBufferSize(void)
{
    return ADC_TT_BUFFER_SIZE;
}

/**
 * @brief  计算平均值
 */
uint16_t ADC_TT_GetAverage(void)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < ADC_TT_BUFFER_SIZE; i++) {
        sum += s_adc_buffer[i];
    }
    return sum / ADC_TT_BUFFER_SIZE;
}

/**
 * @brief  计算电压
 */
float ADC_TT_GetVoltage(void)
{
    return (float)ADC_TT_GetAverage() * ADC_VREF / ADC_MAX;
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
    }
}
