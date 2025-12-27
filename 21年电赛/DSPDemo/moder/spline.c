#include "spline.h" 
#include "usart.h" 
#include "stdio.h"
#include "arm_math.h"

//样条插补

//arm_spline_instance_f32第二个参数设置
//ARM_SPLINE_NATURAL 表自然样条
//ARM_SPLINE_PARABOLIC_RUNOUT 表示抛物线样条
//x轴坐标必须是递增的
//第六个参数插补数缓存大小，如果原数据个数是n，插补因数个数必须大于等于3n
//第七个参数临时缓存大小，如果原始数据个数是n，临时缓冲大小必须大于等于2n-1




#define INPUT_TEST_LENGTH_SAMPLES 128 /* 输入数据个数 */
#define OUT_TEST_LENGTH_SAMPLES 1024 /* 输出数据个数 */

#define SpineTab OUT_TEST_LENGTH_SAMPLES/INPUT_TEST_LENGTH_SAMPLES /* 插补末尾的 8 个坐标值不使用 */

float32_t xn[INPUT_TEST_LENGTH_SAMPLES]; /* 输入数据 x 轴坐标 */
float32_t yn[INPUT_TEST_LENGTH_SAMPLES]; /* 输入数据 y 轴坐标 */

float32_t coeffs[3*(INPUT_TEST_LENGTH_SAMPLES - 1)]; /* 插补系数缓冲 */
float32_t tempBuffer[2 * INPUT_TEST_LENGTH_SAMPLES - 1]; /* 插补临时缓冲 */

float32_t xnpos[OUT_TEST_LENGTH_SAMPLES]; /* 插补计算后 X 轴坐标值 */
float32_t ynpos[OUT_TEST_LENGTH_SAMPLES]; /* 插补计算后 Y 轴数值 */

void spline_test(void)
{
    arm_spline_instance_f32 s;

    for(int i = 0;i<INPUT_TEST_LENGTH_SAMPLES;i++)
    {
        xn[i] = i*SpineTab;
        yn[i] = 1 + cos(2*3.1415926*i/256 + 3.1415926/3);
    }

    /* 插补后 X 轴坐标值，这个是需要用户设置的 */
    for(int i=0; i<OUT_TEST_LENGTH_SAMPLES; i++)
    {
        xnpos[i] = i;
    }
    arm_spline_init_f32(&s,ARM_SPLINE_NATURAL,xn,yn,INPUT_TEST_LENGTH_SAMPLES,coeffs,tempBuffer);


}




















