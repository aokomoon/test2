#ifndef __SI5351_H__
#define __SI5351_H__

#define SCL_H         HAL_GPIO_WritePin(GPIOE,GPIO_PIN_4,GPIO_PIN_SET)
#define SCL_L         HAL_GPIO_WritePin(GPIOE,GPIO_PIN_4,GPIO_PIN_RESET)
   
#define SDA_H         HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET)
#define SDA_L         HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET)

#define SCL_read      HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)
#define SDA_read      HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6)



//#define SI_SYNTH_PLL_A 0x16



//#define SI_PLL_RESET 0x17





#define SI_CLK0_CONTROL		16		// CLK0控制寄存器地址
#define SI_CLK1_CONTROL		17		// CLK1控制寄存器地址
#define SI_CLK2_CONTROL		18		// CLK2控制寄存器地址
#define SI_SYNTH_PLL_A		26		// 锁相环A合成器寄存器地址
#define SI_SYNTH_PLL_B		34		// 锁相环B合成器寄存器地址
#define SI_SYNTH_MS_0		42		// 多合成器0寄存器地址
#define SI_SYNTH_MS_1		50		// 多合成器1寄存器地址
#define SI_SYNTH_MS_2		58		// 多合成器2寄存器地址
#define SI_PLL_RESET		177		// 锁相环复位寄存器地址

#define SI_R_DIV_1		0x00		// R分频比为1
#define SI_R_DIV_2		0x10		// R分频比为2
#define SI_R_DIV_4		0x20		// R分频比为4
#define SI_R_DIV_8		0x30		// R分频比为8
#define SI_R_DIV_16		0x40		// R分频比为16
#define SI_R_DIV_32		0x50		// R分频比为32
#define SI_R_DIV_64		0x60		// R分频比为64
#define SI_R_DIV_128    0x70		// R分频比为128

#define SI_CLK_SRC_PLL_A	0x00			// 时钟源选择锁相环A
#define SI_CLK_SRC_PLL_B	0b00100000		// 时钟源选择锁相环B
#define XTAL_FREQ			25002880		// 晶体振荡器的频率，单位为Hz



void simply_10khz(void);
void simply102_4khz(void);
void simply409_6khz(void);
void setupPLL(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom);
void setupMultisynth(uint8_t synth, uint32_t divider, uint8_t rDiv);
void si5351aSetFrequency(uint32_t frequency, uint8_t Chanal, uint8_t R_DIV);
//void  simply(void);










#endif
