#include "Digital_Filter.h"
#include "arm_math.h"
#include "main.h"
#include "arm_const_structs.h"
#include "DSP/window_functions.h"
#include "math.h"
#include "stdlib.h"
uint8_t fir_fliter_function(float32_t *adc_value, float32_t* fir_output, uint32_t fir_length, uint32_t FIR_NUMTAPS_LENGTH, float32_t* fir_pCoeffs)
{
    arm_fir_instance_f32 * fir_S;	/* FIR实例化结构体 */
    uint16_t fir_numTaps = FIR_NUMTAPS_LENGTH;	/* FIR滤波器系数个数 */
    uint32_t fir_blockSize = fir_length;	/* 块处理大小 */
	uint32_t FIR_PSTATE_LENGTH = fir_length + FIR_NUMTAPS_LENGTH - 1;	/* FIR滤波器状态变量的长度 */
    float32_t  fir_pState[FIR_PSTATE_LENGTH];	/* FIR滤波器状态变量暂存数组：状态数组的大小为 fir_numTaps + fir_blockSize - 1 */
    float32_t fliter_InputBufer[fir_length];	/* FIR滤波输入数组 */

    for (uint16_t i = 0; i < fir_length; i++) {
        fliter_InputBufer[i] = adc_value[i];
    }
    /* 为FIR实例分配内存 */ 
    fir_S = (arm_fir_instance_f32 *)malloc(sizeof(arm_fir_instance_f32));  
    if (fir_S == NULL) {
            return 0; /*return true; 内存分配失败，处理错误 */
    }
    /* FIR滤波器函数是arm_fir_f32。使用此函数可以设计FIR低通，高通，带通和带阻 */
    arm_fir_init_f32(fir_S,fir_numTaps,fir_pCoeffs,fir_pState,fir_blockSize);
    arm_fir_f32(fir_S,fliter_InputBufer,fir_output,fir_blockSize);
    free(fir_S);      /* 释放内存 */
    fir_S = NULL; 		/* 将指针设置为 NULL，以避免悬挂指针 */	
    return 1;
}


uint8_t iir_fliter_function(float32_t *adc_value, uint32_t iir_length)
{
    uint32_t iir_blockSize = IIR_BLOCK_SIZE;				/* 每次处理的数据个数，与 IIR_BLOCK_SIZE 相同 */
    uint32_t iir_numBlocks = iir_length / IIR_BLOCK_SIZE;	/* 需要调用 arm_biquad_cascade_df1_f32 函数的次数，通过总采样点数除以每次处理的数据个数得到 */
    arm_biquad_casd_df1_inst_f32 iir_S;						/* 定义一个结构体变量，用于 IIR 滤波器的初始化 */
    float32_t *iir_inputF32, *iir_outputF32;					/* 定义两个指针变量，分别用于存放滤波器输入和输出缓存的地址 */
    float32_t iir_ScaleValue;									/* 定义一个变量，用于存放放缩系数 */
    float32_t iir_pState[4 * IIR_NUMTAPS_LENGTH];			    /* 状态缓存，大小为 4 * IIR_NUMTAPS_LENGTH，直接 I 型滤波器需要 2N 个延迟器和 2N 个乘法器 */
    float32_t fliter_InputBufer[iir_length];			        /* IIR滤波输入数组 */
    float32_t fliter_OutputBufer[iir_length];			        /* IIR滤波输出数组 */

    iir_inputF32 = &fliter_InputBufer[0];		            /* 初始化输入缓存指针 */
    iir_outputF32 = &fliter_OutputBufer[0];	                /* 初始化输出缓存指针 */
    for (uint16_t i = 0; i < iir_length; i++) {
        fliter_InputBufer[i] = adc_value[i];
    }
    memset(iir_pState, 0, sizeof(iir_pState)); /* 初始化前清零状态 */
    /* 滤波器结构体初始化 */
    arm_biquad_cascade_df1_init_f32(&iir_S, 						/* 初始化结构体S【S就相当于滤波器配置参数，对S结构体的各个成员变量完成初始化】*/
                                IIR_NUMTAPS_LENGTH, 									/* 初始化2阶IIR滤波的个数【滤波器阶数=IIR_NUMTAPS_LENGTH*2，每个IIR_NUMTAPS_LENGTH有5个系数】*/
                                (float32_t *)&iir_Coeffs32LP[0], 			/* 初始化S的滤波器系数地址【滤波器的核心选择都在这些系数里面】*/
                                (float32_t *)&iir_pState[0]);					/* 初始化S的计算缓存空间 */
    for(uint16_t i = 0; i < iir_numBlocks; i++){
        arm_biquad_cascade_df1_f32(&iir_S, 									/* 使用滤波器iir_S */
                                        iir_inputF32 + (i * iir_blockSize), 	/* 滤波器原始输入数据地址 */
                                        iir_outputF32 + (i * iir_blockSize), /* 滤波器滤波后输出数据地址 */
                                        iir_blockSize);											/* 每次处理数据点的个数 */
    }
    /* 计算放缩系数 */
    iir_ScaleValue = 
			0.511924579939916246651421261049108579755*
			0.417706597721717809434949231217615306377*
			0.273985957877031172102988421102054417133*
			0.115392129591839615043724620591092389077*
			0.198038712734786737090786346016102470458;
    /* 对IIR滤波输出数组进行放缩 */
    for(uint16_t i = 0; i < iir_length; i ++){
        fliter_OutputBufer[i] *= iir_ScaleValue;
        adc_value[i] = fliter_OutputBufer[i];
    }
    
    return 1;
}


void lms_filter_f32(float32_t* adc_value, float32_t* adc_ref, float32_t* lms_output, float32_t* lms_err, float32_t* lmsCoeffs32)
{
    uint32_t blockSize = LMS_BLOCK_SIZE; 
    uint32_t numBlocks = LMS_BLOCK_SIZE / LMS_BLOCK_SIZE;        	/* 需要调用arm_lms_norm_f32的次数 */
    float32_t lmsStateF32[LMS_BLOCK_SIZE + LMS_NUM_TAPS - 1];  	/* 状态缓存，大小numTaps + blockSize - 1 */
//    float32_t lmsCoeffs32[LMS_NUM_TAPS];                      	/* 滤波器系数 */

    float32_t  *inputF32, *outputF32, *inputREF, *outputERR;
    arm_lms_norm_instance_f32 lmsS = {0};  /* 注意：arm_lms_norm_instance_f32内部成员可能仍使用float32_t，但属于库定义，无需修改 */
	
    /* 如果是实时性的滤波，仅需清零一次 */
    memset(lmsCoeffs32, 0, sizeof(*lmsCoeffs32));
    memset(lmsStateF32, 0, sizeof(lmsStateF32));	

    /* 初始化输入输出缓存指针 */
    inputF32 = (float32_t *)&adc_value[0];          /* 原始信号 */
    inputREF = (float32_t *)&adc_ref[0];            /* 参考信号 */  
	outputF32 = (float32_t *)&lms_output[0];         /* 输出信号 */
    outputERR = (float32_t *)&lms_err[0];     /* 误差信号 */  
 
    /* 归一化LMS初始化 */
    arm_lms_norm_init_f32(&lmsS,                         /* LMS结构体 */
                          LMS_NUM_TAPS,                      /* 滤波器系数个数 */
                          (float32_t *)&lmsCoeffs32[0], /* 滤波器系数 */ 
                          &lmsStateF32[0],              /* 状态缓存 */
                          0.1f,                    	  /* 步长（添加f后缀明确为单精度） */
                          blockSize);                   /* 处理的数据个数 */
	
    /* 实现LMS自适应滤波，这里每次处理1个点 */
    for (uint16_t i = 0; i < numBlocks; i++)
    {
        arm_lms_norm_f32(&lmsS, /* LMS结构体 */
                         inputF32 + (i * blockSize),   /* 输入数据 */
                         inputREF + (i * blockSize),   /* 参考数据 */
                         outputF32 + (i * blockSize),  /* 输出数据 */
                         outputERR + (i * blockSize),  /* 误差数据 */
                         blockSize);					  /* 处理的数据个数 */
    }
}













/*
*******************************************************************************************************************************************
* 函 数 名: MidFilterRT【适合噪声和脉冲过滤】
* 功能说明: 
* 此函数实现中值滤波器的功能，用于逐个数据进行实时滤波。它适合对包含噪声和脉冲的信号进行处理，能有效过滤掉异常值。
* 在使用该函数前，可在主函数中定义相关变量，示例如下：
* float32_t ADC1_IN3_DMA_floatData[ADC_DMA_SAMPLE_LENGTH];  // 存储原始数据
* float32_t Mid_OutputBufer[ADC_DMA_SAMPLE_LENGTH];         // 存储滤波后的数据
*
* 形    参: 
* - pSrc: 源数据地址，指向需要进行滤波处理的原始数据。
* - pDst: 滤波后数据地址，用于存储滤波后的结果。
* - ucFlag: 控制滤波的阶段，1 表示首次滤波，后续继续滤波时需将其设置为 0。
* - order: 滤波器的阶数，至少为 2 阶。
*
* 返 回 值: 无
*
* 使用举例:
* 					// 从头开始，先滤第 1 个数据 
* 					mid_filter_f32(ADC_Channels[0].DMA_floatData, Mid_OutputBufer, 1, MidFilterOrder);
* 					// 逐次滤波后续数据 
* 					for(int i = 1; i < ADC_DMA_SAMPLE_LENGTH; i++)
* 					{
* 							mid_filter_f32(ADC_Channels[0].DMA_floatData + i , Mid_OutputBufer + i, 0, MidFilterOrder);
* 					}
* 					// 打印数据
* 					for(int i = 0; i < ADC_DMA_SAMPLE_LENGTH; i++){
* 							Set_Current_USART(USART1_IDX);
* 							printf("%f,%f\r\n", ADC_Channels[0].DMA_floatData[i] * 3.3 / 65535.0, Mid_OutputBufer[i] * 3.3 / 65535.0);
* 					}
*
* 注意事项:
* - 输入的 order 参数必须至少为 2 阶，否则可能无法正常进行滤波操作。
* - 首次调用函数时，ucFlag 需设置为 1，后续调用应设置为 0。
*******************************************************************************************************************************************
*/
void mid_filter_f32(float32_t *pSrc, float32_t *pDst, uint8_t ucFlag, uint32_t order)
{
	float32_t Mid_SortData[MidFilterOrder];  					/* 滤波排序 */ 
	float32_t Mid_TempDate[ZeroSize + Mid_LENGTH +ZeroSize]; 	/* 滤波阶段用到的临时变量 */
	arm_sort_instance_f32 S;
	uint16_t N;
	uint32_t CountFlag = 0;
	
	S.dir = ARM_SORT_ASCENDING;
	S.alg = ARM_SORT_QUICK;
	
	N = order / 2;
	
	/* 首次滤波先清零 */
	if(ucFlag == 1)
	{
		CountFlag = 0;
	}
	
	/* 填充数据 */
	if(CountFlag < order)
	{
		Mid_TempDate[CountFlag] = pSrc[0];
		CountFlag++;
	}
	else
	{
		for(uint32_t i =0; i < order - 1; i++)
		{
			Mid_TempDate[i] = Mid_TempDate[1 + i];  
		}
		Mid_TempDate[order - 1] = pSrc[0];
	}
	
	/* 排序 */
	arm_sort_f32(&S, &Mid_TempDate[0], &Mid_SortData[0], order);
	
	/* 奇数 */
	if(N)
	{
		pDst[0] = Mid_SortData[N];
	}
	/* 偶数 */
	else
	{
		pDst[0] = (Mid_SortData[N] + Mid_SortData[N-1])/2;
	}
}

//*************************************************************************//
//*****************************ADC常用滤波算法*****************************//
//*************************************************************************//

/*******************高通、低通滤波器*******************/
// 高低通滤波中，Alpha：0-1。设置越大，越平滑，低通相移会更大，高通幅值削减会更小。
double low_pass_filter(Filter_t* filter, double input)
{
	float output = filter->Alpha * filter->Pre_out + (1 - filter->Alpha) * input;
	filter->Pre_out = output;
	return output;
}
double High_Pass_Filter(Filter_t* filter, double input)
{
	float output = filter->Alpha * (input - filter->Pre_in) + filter->Alpha * filter->Pre_out;
	filter->Pre_in = input;
	filter->Pre_out = output;
	return output;
}
/*******************滑动均值滤波*******************/
// 滑动平均滤波中，AVG_LEN 值越大，波形越平滑，但是幅值会越小，相移会越大。
double average_filter(AVG_Flt_t* filter, double input)
{
	uint8_t i = 0;
	double sum = 0, avg = 0;
	
	filter->Data[filter->index++] = input;
	
	if(filter->index >= AVG_LEN) {
		filter->index = 0;
	}
	for(i = 0; i < AVG_LEN; i ++) {
		sum += filter->Data[i];
	}
	avg = sum / AVG_LEN;
	
	return avg;
}
/*******************中值滤波*******************/
// 中值滤波滤出波形不够平滑，但是对尖峰失真有很好的抑制作用。
int compare(const void* a, const void* b)
{
	double da = *(const double*)a;
	double db = *(const double*)b;
	return (da > db) - (da < db);
}
double median_filter(MEDIAN_Flt_t* filter, double input)
{
	uint8_t i = 0;
	double median = 0;
	
	// 新输入数据存入数组
	filter->Data[filter->index++] = input;
	
	// 索引超出数组范围，则循环回到数组开头
	if(filter->index >= MEDIAN_LEN) {
		filter->index = 0;
	}
	
	// 创建一个临时数组用于排序
	double tempData[MEDIAN_LEN];
	for(i = 0; i < MEDIAN_LEN; i ++) {
		tempData[i] = filter->Data[i];
	}
	
	// 对临时数组排序
	qsort(tempData, MEDIAN_LEN, sizeof(double), compare);
	
	// 取中值
	median = tempData[MEDIAN_LEN / 2];
	
	return median;
}

/*******************卡尔曼滤波*******************/
// R值固定，Q值越大，代表越信任测量值，Q值无穷大，代表只用测量值。
// 			Q值越小，代表越信任模型预测值，Q值为0，代表只用模型预测值。
float kalman_filter(KALMAN_Flt_t* filter, double inData)
{
    filter->p = filter->p + filter->q;
    filter->kGain = filter->p / ( filter->p + filter->r );                                      //计算卡尔曼增益
    inData = filter->prevData + ( filter->kGain * ( inData - filter->prevData ) );      //计算本次滤波估计值
    filter->p = ( 1 - filter->kGain ) * filter->p;                                      //更新测量方差
    filter->prevData = inData;
    return inData;                                             //返回估计值
}


