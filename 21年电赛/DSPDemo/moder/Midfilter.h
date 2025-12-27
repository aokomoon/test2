#ifndef __MIDFILTER_H__
#define __MIDFILTER_H__
#include "arm_math.h"


void MidFilterBlock(float32_t *pSrc,float32_t *pDst,uint32_t blockSize,uint32_t order);
void MidFilterRT(float32_t *pSrc, float32_t *pDst, uint8_t ucFlag, uint32_t order) ;
void test(void);
void MidFilterBlockTest(void);


#endif




