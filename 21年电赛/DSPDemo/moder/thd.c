#include "thd.h"
#include "main.h"
#include "arm_math.h"
//5次谐波
//基波频率 10khz，谐波：20khz,30khz,40khz,50khz
void thd(float32_t funda_wave,float32_t harmonic_2,float32_t harmonic_3,float32_t harmonic_4,float32_t harmonic_5,float32_t* thd)
{
    float32_t mag[5];
    float32_t sum;
    float32_t sqrt_sum;

    mag[0] = funda_wave*funda_wave;
    mag[1] = harmonic_2*harmonic_2;
    mag[2] = harmonic_3*harmonic_3;
    mag[3] = harmonic_4*harmonic_4;
    mag[4] = harmonic_5*harmonic_5;

    sum = mag[1]+mag[2]+mag[3]+mag[4];
    arm_sqrt_f32(sum,&sqrt_sum);
    *thd = sqrt_sum/funda_wave;
}





















