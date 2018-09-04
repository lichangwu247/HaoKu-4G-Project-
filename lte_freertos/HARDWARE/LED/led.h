#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


typedef struct _ledDEF
{
    uint32_t ledOnTime;
		uint32_t ledOffTime;
		uint32_t ledCounter;
		uint8_t ledStatus;
		uint8_t ledEnable;
		uint32_t ledTimes;
} LED_TYPE;


#define TIMER_PERIOD	1	//ms


#define LED_ON_TIME_DFG	  (500 / TIMER_PERIOD) /*1s */
#define LED_OFF_TIME_DFG	(500 / TIMER_PERIOD) /*1s */


#define LED_TURN_ON	1
#define LED_TURN_OFF 0

//LED端口定义
#define LED_RCC     RCC_APB2Periph_GPIOC                            
#define LED_PORT	  GPIOC    
#define LED         GPIO_Pin_5    

#define LEDON			  1
#define LEDOFF			0


void LED_Init(void);//初始化
void CtrLed(uint32_t timeInterval);
void LedSetStatus(uint32_t onTime,uint32_t offTime,uint32_t times);


#define LED0 PAout(5)	// PA5-LED MCU

		 				    
#endif
