#ifndef __FFTInc_H_
#define __FFTInc_H_
#include "main.h"
#include "arm_math.h"
#include "BSP_ADC.H"
#include "stdbool.h"


/*------------------------ 全局系数 ------------------------*/
#define pi		3.14159265358979323846f	/* 定义圆周率的近似值，方便计算正弦波等波形时使用 */
#define ZOOM	(3.3f / 65535.0f)					/* ADC模数转换缩放系数 */
#define IZOOM	(65535.0f / 3.3f)					/* ADC模数转换逆缩放系数 */
#define AD7606_ZOOM		(10.0f / 65535.0f)					/* ADC模数转换缩放系数 */
#define AD7606_IZOOM	(65535.0f / 10.0f)					/* ADC模数转换逆缩放系数 */
#define		MAX_FFT_N		 4096	

#define		FREQ	1
#define 	AMP		2
#define 	INDEX	3


extern void InitTableFFT(uint32_t n);
extern void cfft(float32_t *FFT_Input, uint32_t FFT_N );
extern bool inf_fft_with_mag_norm_f32(float32_t *ADC_DMA_floatData, float32_t *FFT_Input, float32_t *FFT_Output, uint32_t Infinite_FFT_Length);
extern bool inf_fft_without_mag_norm_f32(float32_t *ADC_DMA_floatData, float32_t *fft_buf, uint32_t Infinite_FFT_Length);
extern float32_t find_fft_freq_amp( float32_t *FFT_OutputBuf, uint8_t flag,float32_t sample_rate, uint32_t fft_n);
extern float32_t find_phase_f32(float32_t *FFT_Input, float32_t *FFT_OutputBuf, uint32_t fft_n);

#endif


