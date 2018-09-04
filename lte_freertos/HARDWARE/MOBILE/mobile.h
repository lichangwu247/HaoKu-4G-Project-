#ifndef __MOBILE_H
#define __MOBILE_H	 
#include "sys.h"


#define MOBILE_RCC1     RCC_APB2Periph_GPIOC
#define MOBILE_PORT1	  GPIOC
#define MOBILE_EN		    GPIO_Pin_4


#define MOBILE_RCC2     RCC_APB2Periph_GPIOB
#define MOBILE_PORT2	  GPIOB
#define MOBILE_RST		  GPIO_Pin_9


#define MOBILEON			1
#define MOBILEOFF			0

typedef struct
{
	 float  UTC;
   double  latitude;
	 double  longtitude;
   float  hdop;
   float  altitude;	
}GPRS_Location;

void MOBILE_RESET_ON(void);
void MOBILE_RESET_OFF(void);

void MOBILE_POWER(uint8_t status);

void MOBILE_CTR_Config(void);
	 				

void COMM_Puts(uint8_t *s);
void COMM_PutCs(uint8_t *s, u16 len);


int at_send_cmd(u8 cmd, u8 *pCmd, u8 *pAck, int (*callback)(uint8_t* s, uint16_t cnt), int timeOutMs, uint32_t tryCnt);
int at_wait_ack(char *pAck, int (*callback)(uint8_t* s, uint16_t cnt), int timeout_ms);
int mobile_getIMEI(uint8_t *s, uint16_t cnt);
int mobile_getCCID(uint8_t *s, uint16_t cnt);
int mobile_getCSQ(uint8_t *s, uint16_t cnt);
int mobile_getCCLK(uint8_t *s, uint16_t cnt);


int GPRS_GET_CUR_LOCT(uint8_t *s, uint16_t cnt);



int moblie_data_filter(void);
u8 mobile_get_signal(void );
int updata_mobile_signal(void);
int updata_mobile_time(void);


#endif


