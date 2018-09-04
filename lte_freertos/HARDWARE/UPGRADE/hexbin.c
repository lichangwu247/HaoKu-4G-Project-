#include "hexbin.h"
#include "sys.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "FreeRTOS.h"

HexToBinData g_bin_info;

static int HexCharToByte(char c)  
{  
    if(c>='0' && c<='9')  
        return c -'0';  
    else if(c>='a' && c<='f')  
        return c-'a'+0x0a;  
    else if(c>='A' && c <='F')  
        return c-'A'+0x0a;  
      
    return -1;  
}



static BYTE GetFirstByte(const char *str)  
{  
    //ASSERT(str !=NULL);  
  
    BYTE tmp =0;  
    tmp =HexCharToByte(str[0]);  
    tmp <<=4;  
    tmp +=HexCharToByte(str[1]);  
  
    return tmp;  
}



//从字符串中获取一行  
static int GetLine(const char *str,char *pBuf)  
{  
    //ASSERT(str !=NULL);  
    //ASSERT(pBuf !=NULL);
  
    char *start =strchr((char *)str,':');  
    if(NULL ==start){
			//COM_DBG("::::::: error\r\n");				
        return -1;  
    }  
  
    char *end =strstr(start,"\r\n");
    if(NULL == end){
			//OM_DBG("RNRNRNRNR error\r\n");	
        return -1;  
    }
		
    char *p =start;  
    char *p2 =pBuf;  
    int len=0;  
    for (;p<end+2;p++,p2++)  
    {  
        if(*p =='\0')  
            break;  
        *p2 =*p;  
        len ++;  
    }  
    *p2 ='\0';  
  
    return len;  
}  



//获取一行的数据  
static int GetHexLineData(const char *line,HexLinData *pData)  
{  
    //ASSERT(line !=NULL);  
    //ASSERT(pData !=NULL); 
    BYTE temp,linesum,sum = 0;	
  
    if(line[0] !=':')  
        return -1;  
    int i=1;
		temp =GetFirstByte(&line[i]);
		pData->len =temp;  
    sum += temp;
    i +=2;
		temp =GetFirstByte(&line[i]);
    pData->pos =temp;  
		sum += temp;
    i +=2;
    pData->pos <<=8;
		temp = GetFirstByte(&line[i]);
    pData->pos += temp;
		sum += temp;
    i +=2;
		temp =GetFirstByte(&line[i]);
    pData->type =temp;
    sum += temp;		
    i +=2;  
    for(int j=0;j<pData->len;i+=2,j++){  
        pData->data[j] =GetFirstByte(&line[i]);
			  sum += pData->data[j];
    }
		sum = ~sum + 1; 
		//i +=2;
		linesum = GetFirstByte(&line[i]);
		if(sum != linesum)
			return -1; 
    return 0;  
}


//获取下一行数据及对应类型，偏移量，长度等信息  
static int GetNextLine(const char *str, HexLinData *pData)  
{  
	//ASSERT(str !=NULL);
	//ASSERT(pData !=NULL);  
  
    char *p =(char *)str;  
    char line[128];  
    HexLinData data ={0};  
    int len =strlen(str);  
    int dataLen =0;  
  
    for(;p<str+len;p+=dataLen){  
        memset(line,0,128);  
        dataLen =GetLine(p,line);  
        if(dataLen <0)
				{
					//COM_DBG("dataLen <0\r\n");		
					return -1;
				}  
              
        memset(&data,0x00,sizeof(HexLinData));  
        if(0 !=GetHexLineData(line,&data))
				{
					//COM_DBG("GetHexLineData error\r\n");		
					return -1;
				}
				
        memcpy(pData,&data,sizeof(HexLinData));
        return 0;     
    }  
    return -1;  
}



static int GetStartAddress(HexLinData *pData,UINT *pStartAddress)  
{
	UINT basePos=0;  
  for(int i=0;i<pData->len;i++)
	{
    basePos <<=8;  
    basePos +=pData->data[i];  
  }
  *pStartAddress = basePos<<16;  
  return 0;  
}



static int GetAppAddress(HexLinData *pData,UINT *pAppAddress)  
{
	UINT Address=0;  
  for(int i=0;i<pData->len;i++)
	{
    Address <<=8;  
    Address +=pData->data[i];  
  }
  *pAppAddress = Address;  
  return 0;  
}



int ConvertHexToBin(char *str,HexToBinData *pData)  
{
	char* pstr = str;
	HexLinData data={0};
  BYTE pos_first = 0;
	
	pData->len = 0;
	
	UINT startAddress; //刷写的起始地址
	
	//COM_DBG("ConvertHexToBin: %s\r\n", pstr);
	
	//pData->pContent =(BYTE *)malloc(sizeof(BYTE)*BINBUFF_SIZE);
	pData->pContent =(BYTE *)pvPortMalloc(sizeof(BYTE)*BINBUFF_SIZE);
	if(NULL == pData->pContent)
	{
		COM_DBG("pData->pContent NULL\r\n");
		return 0;
	}
	memset(pData->pContent, 0x0, BINBUFF_SIZE);
	
	while(*pstr)
	{
	  if(0 != GetNextLine(pstr, &data))
		{
			pData->byte_used = pstr - str;
			COM_DBG("pData->byte_used0: %d\r\n", pData->byte_used);
			free(str);
			return pData->len;  
		}
		
		switch(data.type)
		{
			case 0x00://数据记录
				memcpy((pData->pContent + pData->len), data.data, data.len);
			  pData->len += data.len;
			  if(pos_first == 0)
				{
					pData->offset = data.pos;
          pos_first = 1;					
				}
				//COM_DBG("pData->pContent: \r\n");
				//COMDBG_HEX(pData->pContent, BINBUFF_SIZE);
				
			break;
			
			case 0x01://标识HEX文件结束
				pData->percent = 10000;//标记完成百分比100%
			break;
			
			case 0x02://标识扩展段地址记录
				
			break;
			
			case 0x03://开始段地址记录
				
			break;
			
			case 0x04://标识扩展线性地址记录
				GetStartAddress(&data, &startAddress);
			  COM_DBG("Firmware 0x40 Address:0x%08X\r\n", startAddress);		
			  if(pData->startAddress == 0)
					pData->startAddress = startAddress;
				else{
					if(pData->startAddress != startAddress)
					{
						if(pData->sAddr_change == 0)
						{
							pData->sAddr_change = 1;
							goto exit;
						}else{
							pData->startAddress = startAddress;
							pData->sAddr_change = 0;
						}
					}
				}

				COM_DBG("Firmware Start Address:0x%08X\r\n", *&pData->startAddress);		

			break;
			
			case 0x05://开始线性地址记录
				GetAppAddress(&data, &pData->appAddress);
			break;
			
			default:				
			break;
		}
	  pstr += (2*data.len + 13);//下一个line
	}
exit:	
	pData->byte_used = pstr - str;	
	COM_DBG("pData->byte_used1: %d\r\n", pData->byte_used);
	free(str);
  return pData->len;  
}



