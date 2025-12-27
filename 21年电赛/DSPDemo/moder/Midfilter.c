#include "Midfilter.h"
#include "main.h"
#include "usart.h" 
#include "arm_math.h"
#include "stdio.h"
#include <string.h>
#include "stdlib.h"
#include "Upper.h"
//中值滤波器

#define TEST_LENGTH_SAMPLES  1024    		/* 采样点数 */

#define MidFilterOrder  16     				/* 滤波阶数 */
#define ZeroSize        MidFilterOrder

float32_t DstDate[TEST_LENGTH_SAMPLES];     /* 滤波后数据 */

static float32_t SortData[MidFilterOrder];  /* 滤波排序 */ 
static float32_t TempDate[ZeroSize + TEST_LENGTH_SAMPLES +ZeroSize] = {0}; /* 滤波阶段用到的临时变量 */

 float32_t testdata[1024];





//对一段数据进行滤波
void MidFilterBlock(float32_t *pSrc,float32_t *pDst,uint32_t blockSize,uint32_t order)
{
    arm_sort_instance_f32 s;
    uint32_t N,i;
    s.dir = ARM_SORT_ASCENDING;
	s.alg = ARM_SORT_QUICK;

    N = order / 2;
    //数值幅值给临时缓存
    for(i =0; i < blockSize; i++)
	{
		TempDate[i + ZeroSize] = pSrc[i];
	}

    //求每个数据的中点
    for(int i = 0;i<blockSize;i++)
    {
        arm_sort_f32(&s,&TempDate[i+ZeroSize-N],&SortData[0],order);        //排序

        if(N)
        {
            pDst[i] = SortData[N];
        }
        else
        {
            pDst[i] = (SortData[N]+SortData[N-1])/2;
        }
    }
}

//用于逐数据滤波
void MidFilterRT(float32_t *pSrc, float32_t *pDst, uint8_t ucFlag, uint32_t order)  
{
    arm_sort_instance_f32 s;
    uint32_t N,i;

    static uint32_t Countflag = 0;

    s.dir = ARM_SORT_ASCENDING;
	s.alg = ARM_SORT_QUICK;

    N = order / 2;

    if(ucFlag == 1)
    {
        Countflag = 0;
    }

    if(Countflag < order)
    {
        TempDate[Countflag] = pSrc[0];
        Countflag++;
    }
    else
    {
        
        for(i = 0;i<order - 1;i++)
        {
            TempDate[i] = TempDate[1+i];
        }
        TempDate[order-1] = pSrc[0];//最高位数据
    }

    arm_sort_f32(&s,&TempDate[0],&SortData[0],order);   //order是数组长度，输出数组是输入数组的从小到大的排序

   if(N)
	{
		pDst[0] = SortData[N];
	}
	else
	{
		pDst[0] = (SortData[N] + SortData[N-1])/2;
	}
}
//sort函数的排序效果
void test(void)
{
    // arm_sort_instance_f32 s;
    // s.dir = ARM_SORT_ASCENDING;
	// s.alg = ARM_SORT_QUICK;
    // float32_t sort[10]={};
    // float32_t mid[10] = {11.0f,45.0f,23.0f,13.0f,45.0f,12.0f,45.0f,14.0f,32.0f,22.0f};
    // arm_sort_f32(&s,mid,sort,10);
    // for(int i = 0;i<10;i++)
    // {
    //     printf("%f\r\n",sort[i]);
    // }
   
    for(int i = 0;i<1024;i++)
    {
        testdata[i] = 30*sin(30*3.1415926*i/512)+rand()%10;
    }
}

void MidFilterBlockTest(void)
{
    
    MidFilterBlock((float32_t*)&testdata[0],&DstDate[0],TEST_LENGTH_SAMPLES,MidFilterOrder);

    for(int i = 0; i<1024;i++)
    {
       // printf("%f,%f\r\n",testdata[i],DstDate[i]);
      Vofa_send(testdata[i],DstDate[i]);
    }
}

























