#include "led.h"

//LED IO初始化
void LED_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);	 //使能PA端口时钟
		
	 GPIO_InitStructure.GPIO_Pin = LED;				 //LED0-->PA.1 端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(LED_PORT, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.8
	 GPIO_SetBits(LED_PORT,LED);						 //PA.1 输出高

	 LedSetStatus(2000,2000,0x5);
}
 


/*点亮LED.PA1*/
void LED_ON(void) 
{
	GPIO_ResetBits(LED_PORT,LED);
}

/*关闭LED.PA81*/
void LED_OFF(void)
{
	GPIO_SetBits(LED_PORT,LED);
}

LED_TYPE	ledCtr;	
	
void LedSetStatus(uint32_t onTime,uint32_t offTime,uint32_t times)		
{
	ledCtr.ledOnTime = onTime;
	ledCtr.ledOffTime = offTime;
	ledCtr.ledCounter = 0;
	ledCtr.ledEnable = LED_TURN_ON;
	ledCtr.ledTimes = times;
}													



/*LED 闪烁控制*/
void LedCtr(LED_TYPE *ledCtr, uint32_t times)	
{
	if(ledCtr->ledEnable == LED_TURN_ON) 
	{
			if(ledCtr->ledCounter > times)
				ledCtr->ledCounter -= times;
			else ledCtr->ledCounter = 0;
				
			if(ledCtr->ledCounter == 0) 
			{
				if(ledCtr->ledTimes) 
				{
					ledCtr->ledTimes--;
					ledCtr->ledCounter = ledCtr->ledOffTime + ledCtr->ledOnTime;
					ledCtr->ledStatus = LEDON;
				}
			}
			
			if(ledCtr->ledCounter <= ledCtr->ledOffTime) 
				ledCtr->ledStatus = LEDOFF;
	}
}


void CtrLed(uint32_t timeInterval) //在定时器中周期执行此函数，
{
	LedCtr(&ledCtr, timeInterval);
	
	if(ledCtr.ledStatus == LEDON)
	{
		LED_ON();
	}
	else 
	{
		LED_OFF();
	}
}	

