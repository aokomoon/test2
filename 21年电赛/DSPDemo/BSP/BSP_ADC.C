#include "BSP_ADC.H"
#include "adc.h"
#include "stm32h7xx_it.h"
#include "usart.h"
#include "stdio.h"
#include "FFTInc.h"
#include "thd.h"
#include "SI5351.h"
#include "ifft.h"
 float32_t thd_value;
 float32_t result1;
 float32_t result2;
 float32_t result3;
 float32_t result4;
 float32_t result5;
  uint16_t index1;
  float32_t wave_out[2*MAX_FFT_N];
  int index2;
  int freq1;
  	float32_t	freq2;
	float32_t	freq3;
	float32_t	freq4;
	float32_t	freq5;
	float32_t   wave[4096];
	 int begin_flag;
//  int index3;
//  int index4;
//  int index5;
//ADC采样率
volatile uint32_t sample_rate = ADC_SAMPLE_RATE;
__attribute__((section (".RAM_SDRAM"))) ADC_def	adc_ch[3];
__attribute__((section (".RAM_SDRAM"))) float32_t dac_out_buf[ADC_SAMPLE_LENGTH];

//----------- WINDOW -----------//
__attribute__((section (".RAM_SDRAM"))) float32_t adc1_window[ADC_SAMPLE_LENGTH];
__attribute__((section (".RAM_SDRAM"))) float32_t adc2_window[ADC_SAMPLE_LENGTH];
__attribute__((section (".RAM_SDRAM"))) float32_t adc3_window[ADC_SAMPLE_LENGTH];


//----------- FFT -----------//
__attribute__((section (".RAM_SDRAM"))) float32_t adc1_fft_input[MAX_FFT_N * 2];
__attribute__((section (".RAM_SDRAM"))) float32_t adc1_fft_output[MAX_FFT_N];
__attribute__((section (".RAM_SDRAM"))) float32_t adc2_fft_input[MAX_FFT_N * 2];
__attribute__((section (".RAM_SDRAM"))) float32_t adc2_fft_output[MAX_FFT_N];
__attribute__((section (".RAM_SDRAM"))) float32_t adc3_fft_input[MAX_FFT_N * 2];
__attribute__((section (".RAM_SDRAM"))) float32_t adc3_fft_output[MAX_FFT_N];


void flat_top_window(float32_t* adc_value, float32_t* window, uint32_t N)
{
	arm_hft248d_f32(window, N);
    for (uint16_t n = 0; n < N; ++n) {
        adc_value[n] *= window[n];
    }
}
void hanning_window(float32_t* adc_value, float32_t* window, uint32_t N) 
{
	arm_hanning_f32(window, N);
    for (uint16_t n = 0; n < N; ++n) {
        adc_value[n] *= window[n];
    }
}
//adc数据处理函数
void  adc_dsp_working(void)
{
	//通过记录上次数据与这次数据的差值来判断是否波形改变
	static uint16_t index1_last = 0;
	static float32_t mag1_last= 0;
	static float32_t mag2_last= 0;
	static float32_t mag3_last= 0;
	static float32_t mag4_last= 0;
	static float32_t mag5_last= 0;
	float32_t abs1=0;
	float32_t abs2=0;
	float32_t abs3=0;
	float32_t abs4=0;
	float32_t abs5=0;
	float32_t result;
	uint16_t index;
	uint32_t fl;
	arm_rfft_instance_f32 s ;
	static uint16_t flag =1;
    if(adc_ch[0].conv_end_flag == 1)
    {
        adc_ch[0].conv_end_flag = 0;

		for(uint32_t i = 0;i<ADC_SAMPLE_LENGTH;i++)
		{
			adc_ch[0].adc_float_buf[i] = (float32_t)adc_ch[0].adc_buf[i];
		}

		/* 去除直流分量 */
		//此函数是输入float_buf，返回的数据覆盖输入数组，整体去直流，直流数载入da_part
		
		remove_dc_part(adc_ch[0].adc_float_buf, &adc_ch[0].da_part, ADC_SAMPLE_LENGTH);
		
		inf_fft_with_mag_norm_f32(adc_ch[0].adc_float_buf, adc1_fft_input, adc1_fft_output, MAX_FFT_N);
		arm_max_f32(adc1_fft_output,MAX_FFT_N/2,&result1,&index1);//一次谐波
		
		index = 409600/(index1*409600/4096);
		freq1 = (index1*409600/4096);//基波频率
	
		//通过三元运算可以求出绝对值，无非就是大的减小的
		abs1 = (adc1_fft_output[index1]>=mag1_last)?adc1_fft_output[index1]-mag1_last:mag1_last-adc1_fft_output[index1];
		abs2 = (adc1_fft_output[index1*2]>=mag2_last)?adc1_fft_output[index1*2]-mag2_last:mag2_last-adc1_fft_output[index1*2];
		abs3 = (adc1_fft_output[index1*3]>=mag3_last)?adc1_fft_output[index1*3]-mag3_last:mag3_last-adc1_fft_output[index1*3];
		abs4 = (adc1_fft_output[index1*4]>=mag4_last)?adc1_fft_output[index1*4]-mag4_last:mag4_last-adc1_fft_output[index1*4];
		abs5 = (adc1_fft_output[index1*5]>=mag5_last)?adc1_fft_output[index1*5]-mag5_last:mag5_last-adc1_fft_output[index1*5];
		//刚开始先输出一次波形
		if(begin_flag == 1)
		{
			begin_flag = 0;
			for(int i = 0;i<255;i++)
		{
			float32_t t = (float32_t)i/255;//一定要转类型，否者为0
			wave[i] = 			adc1_fft_output[index1]  * arm_sin_f32(2 * pi * t)  + // 基波
                      			adc1_fft_output[index1*2] * arm_sin_f32(2 * pi * 2*t) +  // 2次谐波
                     			adc1_fft_output[index1*3] * arm_sin_f32(2 * pi * 3*t) +  // 3次谐波
                     			adc1_fft_output[index1*4] * arm_sin_f32(2 * pi * 4*t) +  // 4次谐波
                       			adc1_fft_output[index1*5] * arm_sin_f32(2 * pi * 5*t);   // 5次谐波
			printf("add 8,0,%d\xFF\xFF\xFF",(int)(wave[i]*80+120));
			
		} 
		}
		


		//其实如果显示一个周期的波形，就固定点输出255个点
		//因为在21年的要求中是计算5次谐波的值，所以只合成到5次谐波
		//其实真正输出波形应该是在FFT计算后判断幅值是否大于一定会对波形影响的波形，在记录标号（只需要比较一半即可，但相对幅值要乘2）
		//那此时的合成波要经行频率比例运算，例如一个最低频率波为10，而另外一个是35，那前面的系数就要是乘3.5
		//那此时就是index2/index1，注意一定不要是整形除法,否则偏差较大
		if((index1!=index1_last)
		||((index1==index1_last)&&((abs1>0.1)||(abs2>0.1)||(abs3>0.1)||(abs4>0.1)||(abs5>0.1))))
		{
 		for(int i = 0;i<255;i++)
		{
			float32_t t = (float32_t)i/255;//一定要转类型，否者为0
			wave[i] = 			adc1_fft_output[index1]   * arm_sin_f32(2 * pi * t)  + // 基波
                      			adc1_fft_output[index1*2] * arm_sin_f32(2 * pi * 2*t) +  // 2次谐波
                     			adc1_fft_output[index1*3] * arm_sin_f32(2 * pi * 3*t) +  // 3次谐波
                     			adc1_fft_output[index1*4] * arm_sin_f32(2 * pi * 4*t) +  // 4次谐波
                       			adc1_fft_output[index1*5] * arm_sin_f32(2 * pi * 5*t);   // 5次谐波
			
			//printf("%f,%f,%d,%f,%f\n",wave[i],adc1_fft_output[index1],index, arm_cos_f32(2 * pi * t),t);
			//printf("%f,%f,%f,%f,%f\n",adc1_fft_output[index1],adc1_fft_output[index1*2],adc1_fft_output[index1*3],adc1_fft_output[index1*4],adc1_fft_output[index1*5]);
			//printf("%d\n",(int)(wave[i]*80+120));
			printf("add 8,0,%d\xFF\xFF\xFF",(int)(wave[i]*80+120));
			
		}
		}
		//printf("%f,%f,%f,%f,%f\n",adc1_fft_output[index1],adc1_fft_output[index1*2],adc1_fft_output[index1*3],adc1_fft_output[index1*4],adc1_fft_output[index1*5]);
		//printf("%f,%f,%f,%f,%f\n",abs1,abs2,abs3,abs4,abs5);
		 index1_last = index1;
		 mag1_last= adc1_fft_output[index1] ;
	     mag2_last= adc1_fft_output[index1*2] ;
	     mag3_last= adc1_fft_output[index1*3] ;
	     mag4_last= adc1_fft_output[index1*4] ;
	     mag5_last= adc1_fft_output[index1*5] ;
		result1 = adc1_fft_output[index1];
		result2 = adc1_fft_output[index1*2];
		result3 = adc1_fft_output[index1*3];
		result4 = adc1_fft_output[index1*4];
		result5 = adc1_fft_output[index1*5];
		thd(result1,result2,result3,result4,result5,&thd_value);
		printf("freq.txt=\"%d\"\xFF\xFF\xFF",freq1);
		printf("thd.txt=\"%.2f\"\xFF\xFF\xFF",thd_value*100);
		
		 index1_last = index1;
		 mag1_last= adc1_fft_output[index1] ;
	     mag2_last= adc1_fft_output[index1*2] ;
	     mag3_last= adc1_fft_output[index1*3] ;
	     mag4_last= adc1_fft_output[index1*4] ;
	     mag5_last= adc1_fft_output[index1*5] ;
		
	}
}

//总谐波失真度
void  adc_dsp_working_thd(void)
{
	// static uint16_t index1_last = 0;
	// static float32_t mag1_last= 0;
	// static float32_t mag2_last= 0;
	// static float32_t mag3_last= 0;
	// static float32_t mag4_last= 0;
	// static float32_t mag5_last= 0;
	// float32_t abs1=0;
	// float32_t abs2=0;
	// float32_t abs3=0;
	// float32_t abs4=0;
	// float32_t abs5=0;
	// float32_t result;
	// uint16_t index;
	// uint32_t fl;
	// int freq=0;
	// //arm_rfft_instance_f32 s ;
	// //static uint16_t flag =1;
    // if(adc_ch[0].conv_end_flag == 1)
    // {
    //     adc_ch[0].conv_end_flag = 0;

	// 	for(uint32_t i = 0;i<ADC_SAMPLE_LENGTH;i++)
	// 	{
	// 		adc_ch[0].adc_float_buf[i] = (float32_t)adc_ch[0].adc_buf[i];
	// 	}

	// 	/* 去除直流分量 */
	// 	//此函数是输入float_buf，返回的数据覆盖输入数组，整体去直流，直流数载入da_part
		
	// 	remove_dc_part(adc_ch[0].adc_float_buf, &adc_ch[0].da_part, ADC_SAMPLE_LENGTH);
		
	// 	inf_fft_with_mag_norm_f32(adc_ch[0].adc_float_buf, adc1_fft_input, adc1_fft_output, MAX_FFT_N);
	// 	arm_max_f32(adc1_fft_output,MAX_FFT_N/2,&result1,&index1);//一次谐波
		
	// 	index = 409600/(index1*409600/4096);
	// 	freq1 = (index1*409600/4096);//基波频率
	// 	// freq2 = freq1*2;
	// 	// freq3 = freq1*3;
	// 	// freq4 = freq1*4;
	// 	// freq5 = freq1*5;
	// 	abs1 = (adc1_fft_output[index1]>=mag1_last)?adc1_fft_output[index1]-mag1_last:mag1_last-adc1_fft_output[index1];
	// 	abs2 = (adc1_fft_output[index1*2]>=mag2_last)?adc1_fft_output[index1*2]-mag2_last:mag2_last-adc1_fft_output[index1*2];
	// 	abs3 = (adc1_fft_output[index1*3]>=mag3_last)?adc1_fft_output[index1*3]-mag3_last:mag3_last-adc1_fft_output[index1*3];
	// 	abs4 = (adc1_fft_output[index1*4]>=mag4_last)?adc1_fft_output[index1*4]-mag4_last:mag4_last-adc1_fft_output[index1*4];
	// 	abs5 = (adc1_fft_output[index1*5]>=mag5_last)?adc1_fft_output[index1*5]-mag5_last:mag5_last-adc1_fft_output[index1*5];




	// 	//采样频率是409.6khz，也就是点数要跟着1/409.6k  刻度变化
	// 	//这个是输出
	// 	if((index1!=index1_last)
	// 	||((index1==index1_last)&&((abs1>0.1)||(abs2>0.1)||(abs3>0.1)||(abs4>0.1)||(abs5>0.1))))
	// 	{
 	// 	for(int i = 0;i<255;i++)
	// 	{
	// 		float32_t t = (float32_t)i/255;//一定要转类型，否者为0
	// 		wave[i] = 			adc1_fft_output[index1]  * arm_sin_f32(2 * pi * t)  + // 基波
    //                   			adc1_fft_output[index1*2] * arm_sin_f32(2 * pi * 2*t) +  // 2次谐波
    //                  			adc1_fft_output[index1*3] * arm_sin_f32(2 * pi * 3*t) +  // 3次谐波
    //                  			adc1_fft_output[index1*4] * arm_sin_f32(2 * pi * 4*t) +  // 4次谐波
    //                    			adc1_fft_output[index1*5] * arm_sin_f32(2 * pi * 5*t);   // 5次谐波
			
	// 		//printf("%f,%f,%d,%f,%f\n",wave[i],adc1_fft_output[index1],index, arm_cos_f32(2 * pi * t),t);
	// 		//printf("%f,%f,%f,%f,%f\n",adc1_fft_output[index1],adc1_fft_output[index1*2],adc1_fft_output[index1*3],adc1_fft_output[index1*4],adc1_fft_output[index1*5]);
	// 		//printf("%d\n",(int)(wave[i]*80+120));
	// 		printf("add 8,0,%d\xFF\xFF\xFF",(int)(wave[i]*80+120));
			
	// 	}
	// 	}
	// 	//printf("%f,%f,%f,%f,%f\n",adc1_fft_output[index1],adc1_fft_output[index1*2],adc1_fft_output[index1*3],adc1_fft_output[index1*4],adc1_fft_output[index1*5]);
	// 	//printf("%f,%f,%f,%f,%f\n",abs1,abs2,abs3,abs4,abs5);
		
	//     result1 = adc1_fft_output[index1];
	// 	result2 = adc1_fft_output[index1*2];
	// 	result3 = adc1_fft_output[index1*3];
	// 	result4 = adc1_fft_output[index1*4];
	// 	result5 = adc1_fft_output[index1*5];
	// 	thd(result1,result2,result3,result4,result5,&thd_value);
	// 	printf("freq.txt=\"%d\"\xFF\xFF\xFF",freq);
	// 	printf("thd.txt=\"%.2f\"\xFF\xFF\xFF",thd_value*100);
		
	// 	 index1_last = index1;
	// 	 mag1_last= adc1_fft_output[index1] ;
	//      mag2_last= adc1_fft_output[index1*2] ;
	//      mag3_last= adc1_fft_output[index1*3] ;
	//      mag4_last= adc1_fft_output[index1*4] ;
	//      mag5_last= adc1_fft_output[index1*5] ;
    // }
}


//周期波形输出函数
void  adc_dsp_working_wave(void)
{
    if(adc_ch[0].conv_end_flag == 1)
    {
        adc_ch[0].conv_end_flag = 0;

		for(uint32_t i = 0;i<ADC_SAMPLE_LENGTH;i++)
		{
			adc_ch[0].adc_float_buf[i] = (float32_t)adc_ch[0].adc_buf[i];
		}

		/* 去除直流分量 */
		//此函数是输入float_buf，返回的数据覆盖输入数组，整体去直流，直流数载入da_part
		remove_dc_part(adc_ch[0].adc_float_buf, &adc_ch[0].da_part, ADC_SAMPLE_LENGTH);
		inf_fft_with_mag_norm_f32(adc_ch[0].adc_float_buf, adc1_fft_input, adc1_fft_output, MAX_FFT_N);
		ifft_f32(adc1_fft_output,adc1_fft_input,MAX_FFT_N);
		for(int i = 0; i<MAX_FFT_N;i++)
		{
			printf("%f\r\n",adc1_fft_input[i]);
		}
	
    }
}




void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1) adc_ch[0].conv_end_flag = 1;
	// if (hadc->Instance == ADC2) adc_ch[1].conv_end_flag = 1;
	// if (hadc->Instance == ADC3) adc_ch[2].conv_end_flag = 1;
}




void adc_init(void)
{
	/* 设置采样率 */
	//simply409_6khz();     //采样率409.6khz
	//simply102_4khz();
	//simply_10khz();
	//HAL_Delay(100);
	/* 双边谱：可测频率范围 0 <= f <= fs/2 */
	/* 单边谱：可测频率范围 0 <= f <= fs/2 */
	//control_f1_SI5351(0, ADC_SAMPLE_RATE);
	// control_f1_SI5351(1, ADC_SAMPLE_RATE);
	// control_f1_SI5351(2, ADC_SAMPLE_RATE);
	/* ADC校准 */
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);	
	//HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	// HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_Delay(50);
	/* ADC启动 */
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&adc_ch[0].adc_buf, ADC_SAMPLE_LENGTH);	
	//HAL_ADC_Start_DMA(&hadc2,(uint32_t*)&adc_ch[1].adc_buf, ADC_SAMPLE_LENGTH);
	//HAL_ADC_Start_DMA(&hadc3,(uint32_t*)&adc_ch[2].adc_buf, ADC_SAMPLE_LENGTH);
}




//去直流分量
void remove_dc_part(float32_t* inputdata,float32_t* DCpart,uint32_t length)
{
	//整体求平均值，一般周期函数的平均值都是0，有直流分量就是该平均值，在对整体数组减去直流就完成了去直流
	arm_mean_f32(inputdata,length,DCpart);	//浮点数平均值
	arm_offset_f32(inputdata,-(*DCpart),inputdata,length);//对每个浮点数移位
}



//基波
void findmax(float32_t* pscr,float32_t* result,float32_t* index)
{
	arm_max_f32(&(*pscr),4096,&(*result),&(*index));
}














