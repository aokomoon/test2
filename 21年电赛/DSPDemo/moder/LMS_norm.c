#include "LMS_norm.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdio.h"
#include "usart.h"

//自适应滤波器


#define TEST_LENGTH_SAMPLES  2048    /* 采样点数 */
#define BLOCK_SIZE           2048    /* 调用一次arm_lms_norm_f32处理的采样点个数 */
#define NUM_TAPS             20      /* 滤波器系数个数 */
uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;
static float32_t testinput_50hz_200hz[TEST_LENGTH_SAMPLES];
static float32_t testinput_REF[TEST_LENGTH_SAMPLES];
static float32_t test_f32_ERR[TEST_LENGTH_SAMPLES]={0};         //误差数据
static float32_t testoutput[TEST_LENGTH_SAMPLES] = {0};         //输出数据
static float32_t lmsStateF32[BLOCK_SIZE + NUM_TAPS - 1]={0}; //状态缓存
static float32_t lmsCoeffs32[NUM_TAPS] = {0};          //滤波器数

void test_init(void)
{
    
}
void arm_lms_f32_test(void)
{
    uint32_t i;
    float32_t *inputF32,*outputF32,*inputREF,*outputERR;
    arm_lms_instance_f32 lmsS = {0};

    for(int i =1;i<2048;i++)
    {
        testinput_50hz_200hz[i] = arm_sin_f32(2*3.1415926f*50*i/1000)+
                                        arm_sin_f32(2*3.1415926f*200*i/1000);
        testinput_REF[i] = arm_sin_f32(2*3.1415926f*50*i/1000);
    }

    memset(lmsCoeffs32,0,sizeof(lmsCoeffs32));
    memset(lmsStateF32,0,sizeof(lmsStateF32));	
    inputF32 = (float32_t*)&testinput_50hz_200hz[0];
    outputF32 = (float32_t*)&testoutput[0];
    inputREF  = (float32_t*)&testinput_REF[0];
    outputERR = (float32_t*)&test_f32_ERR[0];

    arm_lms_init_f32(&lmsS,NUM_TAPS,(float32_t*)&lmsCoeffs32[0],&lmsStateF32[0],0.1,BLOCK_SIZE);

    arm_lms_f32(&lmsS,inputF32,inputREF,outputF32,outputERR,BLOCK_SIZE);
        for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f, %f\r\n", testinput_50hz_200hz[i], testoutput[i], test_f32_ERR[i]);
	}


}

void  arm_lms_f32_test2(void)
{
    uint32_t i;
    float32_t *inputF32,*outputF32,*inputREF,*outputERR;
    arm_lms_instance_f32 lmsS = {0};

    for(int i =1;i<2048;i++)
    {
        testinput_50hz_200hz[i] = 20*arm_sin_f32(2*3.1415926f*50*i/1000)+rand()%10;
                        
        testinput_REF[i] = 20*arm_sin_f32(2*3.1415926f*50*i/1000);
    }

    memset(lmsCoeffs32,0,sizeof(lmsCoeffs32));
    memset(lmsStateF32,0,sizeof(lmsStateF32));	
    inputF32 = (float32_t*)&testinput_50hz_200hz[0];
    outputF32 = (float32_t*)&testoutput[0];
    inputREF  = (float32_t*)&testinput_REF[0];
    outputERR = (float32_t*)&test_f32_ERR[0];

    arm_lms_init_f32(&lmsS,NUM_TAPS,(float32_t*)&lmsCoeffs32[0],&lmsStateF32[0],0.1,BLOCK_SIZE);

    arm_lms_f32(&lmsS,inputF32,inputREF,outputF32,outputERR,BLOCK_SIZE);

        for(i=0; i<TEST_LENGTH_SAMPLES; i++)
	{
		printf("%f, %f, %f\r\n", testinput_50hz_200hz[i], testoutput[i], test_f32_ERR[i]);
	}



}













