#ifndef __SELECTOR_H__
#define __SELECTOR_H__

#include "sys.h"


//单位:ms
#define COIN_WIDTH_MIN      15//15000
#define COIN_WIDTH_MAX      120//120000
#define COIN_WIDTH_PD       160//160000


#define TB_WIDTH_MIN      10//18000
#define TB_WIDTH_MAX      120//300000
#define TB_WIDTH_PD       700//80000

#define ON			1
#define OFF			0

#define HIGH		0
#define LOW			1

#define ACCEPTOR_RCC     RCC_APB2Periph_GPIOB
#define ACCEPTOR_RCC1     RCC_APB2Periph_GPIOA

#define ACCEPTOR_PORT	  GPIOB 
#define ACCEPTOR_PORT1	  GPIOA 

#define TB_OUT1         GPIO_Pin_0  
#define COIN_OUT        GPIO_Pin_1

#define COIN_OUT_READ() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)

#define TB_IN           GPIO_Pin_8
#define COIN_IN         GPIO_Pin_8
//PA.8 COIN_IN
#define COIN_IN_READ()			GPIO_ReadInputDataBit(GPIOA, COIN_IN)

//PB.8 TB_IN
#define TB_IN_READ()			  GPIO_ReadInputDataBit(GPIOB, TB_IN)

typedef struct 
{
  u16 polarity;//0: N.C   1:N.O
	u16 width;
	u16 interval;
}selector;


typedef struct 
{
  u16 polarity;//0: N.C   1:N.O
	u16 width;
	u16 interval;
}tb_out;

typedef struct 
{
	u16 devStatus;//当前设备运行状态
	u16 ctrStatus;//当前控制设备状态
  u16 coin_res;//当前需要投的币数
	u16 tbout_has;//当前的退币或出娃娃的数量
	u16 coin_has;//当前实体投币数量
}service;


extern selector g_selector_info;
extern service  g_service_info;

typedef struct
{
    uint32_t coinOnTime;
		uint32_t coinOffTime;
		uint32_t coinCounter;
		uint8_t coinStatus;
		uint8_t coinPolarity;
		uint32_t coinTotal;
} COIN_TYPE;

extern COIN_TYPE	coinCtr,tboutCtr;

void user_selector_init(void);
void coin_out_action(uint32_t num);
void COIN_OUT_SET(uint8_t status);

void CtrCoin(uint32_t timeInterval);
void CoinCtr(COIN_TYPE *coinCtr, uint32_t times);
void CoinInSet(COIN_TYPE *coinCtr,uint32_t onTime,uint32_t offTime,uint32_t polarity,uint32_t num);

void SELECTOR_GPIO_Config(void);

void coin_tb_in_det(uint32_t timeInterval);
#endif

