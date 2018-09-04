/* ��������Get_ChipID
 * ����  ����ȡоƬID
 * ����  ����
 * ���  ����
 * ˵��  ��96λ��ID��stm32Ψһ��ݱ�ʶ��������8bit��16bit��32bit��ȡ
           �ṩ�˴�˺�С�����ֱ�ʾ����
*/


#include "chip.h"
#include "sys.h"

void Get_ChipID(char* chipId)
{
#if 0
 u32 ChipUniqueID[3];
     //��ַ��С����,�ȷŵ��ֽڣ��ٷŸ��ֽڣ�С��ģʽ
     //��ַ��С����,�ȷŸ��ֽڣ��ٷŵ��ֽڣ����ģʽ
 ChipUniqueID[2] = *(__IO u32*)(0X1FFFF7E8);  // ���ֽ�
 ChipUniqueID[1] = *(__IO u32 *)(0X1FFFF7EC); // 
 ChipUniqueID[0] = *(__IO u32 *)(0X1FFFF7F0); // ���ֽ�
 COM_DBG("######## оƬ��ΨһIDΪ: %X-%X-%X  \r\n",ChipUniqueID[0],ChipUniqueID[1],ChipUniqueID[2]);
          //����������32λ
#else   //�����˴�С��ģʽ����ISP���������һ��
    u8 temp[12];   
    u32 temp0,temp1,temp2;
    temp0=*(__IO u32*)(0x1FFFF7E8);    //��ƷΨһ��ݱ�ʶ�Ĵ�����96λ��
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
	  //COM_DBG("CHIP ID: %s\r\n", chipId); //���ڴ�ӡ��оƬID
#endif
}


