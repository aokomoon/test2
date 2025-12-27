#include "ifft.h"
#include "stdlib.h"


/*
* 傅里叶逆变换
* 输入：为傅里叶变换原始输出，即fft_input
* 最高支持16384个点
*/
void ifft_f32(float32_t* IFFT_Buffer, float32_t* ifft_output, uint32_t ifft_length)
{
    uint32_t N = ifft_length;
    uint32_t i, j, k, m, step, half_step;
    float32_t temp_real, temp_imag, u_real, u_imag, t_real, t_imag, angle;

    // ------ 优化1: 预计算log2N ------
    uint32_t log2N = 0;
    for (i = N; i > 1; i >>= 1) log2N++;

    // ------ 优化2: 用查表法做bit reversal ------
    // 只需要分配一次空间，避免重复log2和移位操作
    uint32_t* rev_tab = (uint32_t*)malloc(N * sizeof(uint32_t));
    rev_tab[0] = 0;
    for (i = 1; i < N; i++) {
        rev_tab[i] = (rev_tab[i >> 1] >> 1) | ((i & 1) << (log2N - 1));
    }
    for (i = 0; i < N; i++) {
        uint32_t rev = rev_tab[i];
        if (rev > i) {
            // Swap real part
            temp_real = IFFT_Buffer[2*i];
            ifft_output[2*i] = IFFT_Buffer[2*rev];
            ifft_output[2*rev] = temp_real;
            // Swap imag part
            temp_imag = IFFT_Buffer[2*i+1];
            ifft_output[2*i+1] = IFFT_Buffer[2*rev+1];
            ifft_output[2*rev+1] = temp_imag;
        }
    }
    free(rev_tab);

    // ------ 优化3: 合并常量和分支，减少乘法 ------
    for (step = 2; step <= N; step <<= 1) {
        float32_t theta = 2 * PI / step;
        float32_t w_step_real = cosf(theta);
        float32_t w_step_imag = sinf(theta);

        for (k = 0; k < N; k += step) {
            float32_t w_real = 1.0f;
            float32_t w_imag = 0.0f;

            half_step = step >> 1;
            for (m = 0; m < half_step; m++) {
                uint32_t i_even = k + m;
                uint32_t i_odd = i_even + half_step;

                t_real = w_real * ifft_output[2*i_odd] - w_imag * ifft_output[2*i_odd+1];
                t_imag = w_real * ifft_output[2*i_odd+1] + w_imag * ifft_output[2*i_odd];

                u_real = ifft_output[2*i_even];
                u_imag = ifft_output[2*i_even+1];

                // Butterfly
                ifft_output[2*i_even]     = u_real + t_real;
                ifft_output[2*i_even+1]   = u_imag + t_imag;
                ifft_output[2*i_odd]      = u_real - t_real;
                ifft_output[2*i_odd+1]    = u_imag - t_imag;

                // 递推w
                // w_new = w * w_step
                float32_t w_tmp = w_real * w_step_real - w_imag * w_step_imag;
                w_imag = w_real * w_step_imag + w_imag * w_step_real;
                w_real = w_tmp;
            }
        }
    }

    // ------ 优化4: 使用单独变量存储1/N，减少浮点除法 ------
    // float32_t inv_N = 1.0f / N;
    // for (i = 0; i < N; i++) {
    //     ifft_output[2*i]   *= inv_N;
    //     ifft_output[2*i+1] *= inv_N;
	// 	ifft_output[i] = ifft_output[2*i];  // 实部存入，覆盖虚部位置（按需调整）
	// 	ifft_output[2*i+1] = -ifft_output[2*i+1];  // 虚部取负
    // }
    float32_t inv_N = 1.0f / N;
    for (uint32_t i = 0; i < N; i++) 
    {
        ifft_output[2*i]   *= inv_N;  
        ifft_output[2*i+1] *= inv_N;  
    }
}




// 实现 ifftshift 功能的函数
void ifftshift(float32_t *data, uint32_t length)
{
    uint32_t half_length = length / 2;
    float32_t temp;
	
    for (uint32_t i = 0; i < half_length; i++) {
        // 交换前半部分和后半部分的元素
        temp = data[i];
        data[i] = data[i + half_length];
        data[i + half_length] = temp;
    }
}


