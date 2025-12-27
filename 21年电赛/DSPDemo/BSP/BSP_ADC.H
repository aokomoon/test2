#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__
#include "arm_math.h"


#define ADC_SAMPLE_RATE			409600
#define ADC_SAMPLE_LENGTH		4096
extern float32_t thd_value;
extern float32_t result1;
extern float32_t result2;
extern float32_t result3;
extern float32_t result4;
extern float32_t result5;
 extern uint16_t index1;
 extern int begin_flag;
// extern int index2;
// extern int index3;
// extern int index4;
// extern int index5;
typedef struct 
{
    uint16_t adc_buf[ADC_SAMPLE_LENGTH];                /* DMA 缓冲 */
    float32_t adc_float_buf[ADC_SAMPLE_LENGTH];         /* 定义 ADC 浮点数据存储数组 */
    volatile uint8_t conv_end_flag;                     /* DMA 转换结束标志位 */
    float32_t da_part;                                  /* 直流分量 */
}ADC_def;

extern volatile uint32_t sample_rate;	/* 采样率 */

extern ADC_def adc_ch[3];

void adc_init(void);
void  adc_dsp_working(void);
void adc_dsp_working_thd(void);
void  adc_dsp_working_wave(void);
void remove_dc_part(float32_t* inputdata,float32_t* DCpart,uint32_t length);











#endif
