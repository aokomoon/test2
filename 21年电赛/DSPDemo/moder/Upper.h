#ifndef __UPPER_H
#define __UPPER_H


#define CH_COUNT 1//通道数

void Float_to_Byte(float f,unsigned char byte[]);
void Float_send(float *data_array);
void Send_array(unsigned char* byte, int Number);

void Vofa_send(int data1,int data2);
#endif
