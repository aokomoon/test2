#include "main.h"
#include "SI5351.h"
#include "i2c.h"
#include "Si5351A-RevB-Registers.h"
#include "Si5351A2.h"
#include "Si5351A3.h"

void simply_10khz(void)
{
  for(int i = 0;i<SI5351A_REVB_REG_CONFIG_NUM_REGS;i++)
   {
     HAL_I2C_Mem_Write(&hi2c1, 0xC0, si5351a_revb_registers[i].address, 1, &si5351a_revb_registers[i].value,4, 10);
   }
}

void simply102_4khz(void)
{
   for(int i = 0;i<SI5351A_REVB_REG_CONFIG_NUM_REGS_2;i++)
   {
     HAL_I2C_Mem_Write(&hi2c1, 0xC0, si5351a_102_4KHZ[i].address,1, &si5351a_102_4KHZ[i].value, 4, 10);
   }
}

void simply409_6khz(void)
{
   for(int i = 0;i<SI5351A_REVB_REG_CONFIG_NUM_REGS_3;i++)
   {
     HAL_I2C_Mem_Write(&hi2c1, 0xC0, si5351a409_6HZ[i].address, 1, &si5351a409_6HZ[i].value, 4, 10);
   }
   
}
// 配置PLL（锁相环）的函数
// pll: PLL的地址
// mult: 整数倍数
// num: 分数部分的分子
// denom: 分数部分的分母
void setupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{
  uint32_t P1;  // PLL配置寄存器P1
  uint32_t P2;  // PLL配置寄存器P2
  uint32_t P3;  // PLL配置寄存器P3

  P1 = (uint32_t)(128 * ((float)num / (float)denom));
  // 计算完整的P1值
  P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
   // 计算P2的第一部分
  P2 = (uint32_t)(128 * ((float)num / (float)denom));
  // 计算完整的P2值
  P2 = (uint32_t)(128 * num - denom * P2);
   // P3等于分母
  P3 = denom;
 
  uint8_t data[8] = {
    (P3  & 0x0000FF00)>>8,        // Reg0
    P3 & 0x000000FF,               // Reg1
    (P1  & 0x00030000)>>16,       // Reg2
    (P1 & 0x0000FF00)>>8,        // Reg3
    P1 & 0x000000FF,               // Reg4
    (((P3  & 0x000F0000)>>12) | ((P2 & 0x000F0000)>>16)), // Reg5
    (P2  & 0x0000FF00)>>8,        // Reg6
    P2 & 0x000000FF                // Reg7
  };
   for(int i = 0;i<8;i++)
   {
    HAL_I2C_Mem_Write(&hi2c1, 0xC0, pll+i, 1, &data[i], 4, 10);
  }
  
}


// 配置多合成器的函数
// synth: 多合成器的地址
// divider: 分频系数
// rDiv: 最终R分频系数
void setupMultisynth(uint8_t synth, uint32_t divider, uint8_t rDiv)
{
  uint32_t P1;  // 合成器配置寄存器P1
  uint32_t P2;  // 合成器配置寄存器P2
  uint32_t P3;  // 合成器配置寄存器P3

  // 计算P1的值
  P1 = 128 * divider - 512;
  // P2设为0，P3设为1以强制分频系数为整数
  P2 = 0;
  P3 = 1;

  uint8_t data[8] = {
    (P3 & 0x0000FF00)>>8,        // Reg0
    (P3 & 0x000000FF),               // Reg1
    ((P1& 0x00030000) >> 16) | rDiv,       // Reg2
    (P1 & 0x0000FF00) >> 8,        // Reg3
    (P1 & 0x000000FF),               // Reg4
    ((P3&0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16), // Reg5
    (P2 & 0x0000FF00) >> 8,        // Reg6
    (P2 & 0x000000FF)               // Reg7
  };
   for(int i = 0;i<8;i++)
  {
  HAL_I2C_Mem_Write(&hi2c1, 0xC0, synth+i, 1, &data[i], 4, 100);
  }
}

void si5351aSetFrequency(uint32_t frequency, uint8_t Chanal, uint8_t R_DIV)
{
    uint32_t pllFreq;
    uint32_t xtalFreq = XTAL_FREQ;  // 晶体频率
    uint32_t l;
    float f;
    uint8_t mult;
    uint32_t num;
    uint32_t denom;
    uint32_t divider;
    uint8_t set_pll[1] = {0x4F | SI_CLK_SRC_PLL_A} ;
    uint8_t data1[1] = {0xA0} ;


    // 计算分频系数，900000000是内部PLL的最大频率：900MHz
    divider = 900000000 / frequency;
    // 确保分频系数为偶数
    if (divider % 2) divider--;

    // 计算PLL频率
    pllFreq = divider * frequency;

    // 确定PLL的整数倍数
    mult = pllFreq / xtalFreq;
    // 计算余数
    l = pllFreq % xtalFreq;
    f = l;
    // 计算分数部分的分子
    f *= 1048575;
    f /= xtalFreq;
    num = f;
    // 分母设为最大值1048575
    denom = 1048575;
    // 使用计算得到的倍数配置PLL A
    setupPLL(SI_SYNTH_PLL_A, mult, num, denom);

    // 根据通道设置多合成器
    if(Chanal == 0)
    {
        // 配置多合成器0
        setupMultisynth(SI_SYNTH_MS_0, divider, (uint8_t)R_DIV);
        // 复位PLL，这会导致输出出现毛刺
        HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_PLL_RESET,1,data1,1,100);
        //i2cSendRegister(SI_PLL_RESET, 0xA0);
        // 开启CLK0输出并设置输入为PLL A
        HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_CLK0_CONTROL,1,set_pll,1,100);
       // i2cSendRegister(SI_CLK0_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
    }
    else if (Chanal == 1)
    {
        // 配置多合成器1
        setupMultisynth(SI_SYNTH_MS_1, divider, (uint8_t)R_DIV);
        // 复位PLL
         HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_PLL_RESET,1,data1,1,100);
        //i2cSendRegister(SI_PLL_RESET, 0xA0);
        // 开启CLK1输出并设置输入为PLL A
         HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_CLK0_CONTROL,1,set_pll,1,100);
        //i2cSendRegister(SI_CLK1_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
    }
    else if (Chanal == 2)
    {
        // 配置多合成器2
        setupMultisynth(SI_SYNTH_MS_2, divider, (uint8_t)R_DIV);
        // 复位PLL
        HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_PLL_RESET,1,data1,1,100);
        //i2cSendRegister(SI_PLL_RESET, 0xA0);
        // 开启CLK2输出并设置输入为PLL A
         HAL_I2C_Mem_Write(&hi2c1,0xC0,SI_CLK0_CONTROL,1,set_pll,1,100);
        //i2cSendRegister(SI_CLK2_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
    }
}












