/* 函数名：Get_ChipID
 * 描述  ：获取芯片ID
 * 输入  ：无
 * 输出  ：无
 * 说明  ：96位的ID是stm32唯一身份标识，可以以8bit、16bit、32bit读取
           提供了大端和小端两种表示方法
*/


#include "chip.h"
#include "sys.h"

void Get_ChipID(char* chipId)
{
#if 0
 u32 ChipUniqueID[3];
     //地址从小到大,先放低字节，再放高字节：小端模式
     //地址从小到大,先放高字节，再放低字节：大端模式
 ChipUniqueID[2] = *(__IO u32*)(0X1FFFF7E8);  // 低字节
 ChipUniqueID[1] = *(__IO u32 *)(0X1FFFF7EC); // 
 ChipUniqueID[0] = *(__IO u32 *)(0X1FFFF7F0); // 高字节
 COM_DBG("######## 芯片的唯一ID为: %X-%X-%X  \r\n",ChipUniqueID[0],ChipUniqueID[1],ChipUniqueID[2]);
          //此条语句输出32位
#else   //调整了大小端模式，与ISP下载软件的一致
    u8 temp[12];   
    u32 temp0,temp1,temp2;
    temp0=*(__IO u32*)(0x1FFFF7E8);    //产品唯一身份标识寄存器（96位）
    temp1=*(__IO u32*)(0x1FFFF7EC);
    temp2=*(__IO u32*)(0x1FFFF7F0);
    temp[0] = (u8)(temp0 & 0x000000FF);
    temp[1] = (u8)((temp0 & 0x0000FF00)>>8);
    temp[2] = (u8)((temp0 & 0x00FF0000)>>16);
    temp[3] = (u8)((temp0 & 0xFF000000)>>24);
    temp[4] = (u8)(temp1 & 0x000000FF);
    temp[5] = (u8)((temp1 & 0x0000FF00)>>8);
    temp[6] = (u8)((temp1 & 0x00FF0000)>>16);
    temp[7] = (u8)((temp1 & 0xFF000000)>>24);
    temp[8] = (u8)(temp2 & 0x000000FF);
    temp[9] = (u8)((temp2 & 0x0000FF00)>>8);
    temp[10] = (u8)((temp2 & 0x00FF0000)>>16);
    temp[11] = (u8)((temp2 & 0xFF000000)>>24);
    
		sprintf(chipId,"%.2X%.2X%.2X%.2X-%.2X%.2X%.2X%.2X-%.2X%.2X%.2X%.2X",
		               temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7],temp[8],temp[9],temp[10],temp [11]);
	  //COM_DBG("CHIP ID: %s\r\n", chipId); //串口打印出芯片ID
#endif
}


