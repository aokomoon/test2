#include "usart.h"
#include "Upper.h"
#include "string.h"
//要点提示:
//1.float和unsigned long具有相同的数据结构长度
//2.union据类型里的数据存放在相同的物理空间
typedef union
{
    float fdata;
    unsigned long ldata;
} FloatLongType;

//将浮点数f转化为4个字节数据存放在byte[4]中
//传递指针
void Float_to_Byte(float f,unsigned char byte[])
{
    FloatLongType fl;
    fl.fdata=f;
    byte[0]=(unsigned char)fl.ldata;
    byte[1]=(unsigned char)(fl.ldata>>8);
    byte[2]=(unsigned char)(fl.ldata>>16);
    byte[3]=(unsigned char)(fl.ldata>>24);
}
//串口数组发送数据
void Send_array(unsigned char* byte, int Number)
{
    // 增加基本参数校验
    if(byte == NULL || Number == 0){
        return;
    }
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, byte, Number, 10);
    if (status != HAL_OK) {
    }
}
//Justfloat协议发送
void Float_send(float *data_array)
{
    //参数为浮点数组
    unsigned char tail[4] = {0x00, 0x00, 0x80, 0x7f};//定义帧尾
    //按照通道数发送数据，CH_COUNT为发送数据的通道数
    for (int i = 0; i < CH_COUNT; i++){
        unsigned char byte[4];
        Float_to_Byte(data_array[i],byte); // 将每个float转换为4个字节
        Send_array(byte,4);           // 发送这4个字节
    }
    Send_array(tail,4);//发送数据帧尾
}


void Vofa_send(int data1,int data2)
{
    int data[6];     
    float tempFloat[2];   //定义的临时变量
    uint8_t tempData[12];   //定义的传输Buffer，一个int占4个字节，如果传6个，4*6=24，后面还有四个校验位，24+4=28
 
    data[0] = data1;
    data[1] = data2;
   
 
    tempFloat[0] = (float)data[0];     //转成浮点数
    tempFloat[1] = (float)data[1];

    memcpy(tempData, (uint8_t *)tempFloat, sizeof(tempFloat));  //通过拷贝把数据重新整理
 
    tempData[8] = 0x00;//写入结尾数据
    tempData[9] = 0x00;
    tempData[10] = 0x80;
    tempData[11] = 0x7f;
 
    HAL_UART_Transmit(&huart1, tempData, 12,0xff);//通过串口传输数据，前面开多大的数组，后面占多大长度
}




