#include "selector.h"
#include "info_storage.h" 
#include "sys.h"


selector g_selector_info;
tb_out   g_tb_out_info;
service  g_service_info;


COIN_TYPE	coinCtr;	


/*COIN_OUT*/
void COIN_OUT_SET(uint8_t status)
{
	if(status){
		//COM_DBG("\r\nCOIN_OUT 1\n");
		GPIO_SetBits(ACCEPTOR_PORT,COIN_OUT);
	}else{
		//COM_DBG("\r\nCOIN_OUT 2\n");
		GPIO_ResetBits(ACCEPTOR_PORT,COIN_OUT);
	}
}



/******************************************************************************
 * FunctionName : user_selector_init
 * Description  : 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_selector_init(void)
{
	selector_data_read();
	
	SELECTOR_GPIO_Config();
	
	COM_DBG("\r\nselector_pramarater:\n");
	COM_DBG("polarity: %d\n",g_selector_info.polarity);
	COM_DBG("width: %d\n",g_selector_info.width);
	COM_DBG("interval: %d\n",g_selector_info.interval);

	coinCtr.coinPolarity = g_selector_info.polarity;
}

/******************************************************************************
 * FunctionName : coin_in//�����豸  Ͷ�ң�������壬����Ϊg_selector_info������
 * Description  : coin in precess
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void coin_out_action(uint32_t num)
{
	CoinInSet(&coinCtr,g_selector_info.width,g_selector_info.interval,g_selector_info.polarity,num);
}


/*******************************************************************************
* ������  : SELECTOR_GPIO_Config
* ����    : LED IO����
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : 
*******************************************************************************/
void SELECTOR_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//����һ��GPIO_InitTypeDef���͵�GPIO��ʼ���ṹ��
	
	RCC_APB2PeriphClockCmd(ACCEPTOR_RCC1, ENABLE);	//ʹ��USART1��GPIOAʱ��
	RCC_APB2PeriphClockCmd(ACCEPTOR_RCC, ENABLE);	//ʹ��GPIOBʱ��
	
	
	//IO�������ģʽ
	GPIO_InitStructure.GPIO_Pin = TB_OUT1 | COIN_OUT;				    //ѡ��Ҫ��ʼ����GPIOB����(PB0 1 )
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������Ź���ģʽΪͨ��������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(ACCEPTOR_PORT, &GPIO_InitStructure);			//���ÿ⺯���е�GPIO��ʼ������
	
	GPIO_InitStructure.GPIO_Pin = TB_IN;				    //ѡ��Ҫ��ʼ����GPIOB����(PB0 1 )
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//�������Ź���ģʽΪ����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(ACCEPTOR_PORT, &GPIO_InitStructure);			//���ÿ⺯���е�GPIO��ʼ������
	
	GPIO_InitStructure.GPIO_Pin = COIN_IN;				    //ѡ��Ҫ��ʼ����GPIOB����(PB0 1 )
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//�������Ź���ģʽΪ����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(ACCEPTOR_PORT1, &GPIO_InitStructure);			//���ÿ⺯���е�GPIO��ʼ������
		 
}



void CoinInSet(COIN_TYPE *coinCtr,uint32_t onTime,uint32_t offTime,uint32_t polarity,uint32_t num)		
{
	coinCtr->coinOnTime = onTime;
	coinCtr->coinOffTime = offTime;
	coinCtr->coinCounter = 0;
	coinCtr->coinPolarity = polarity;
	coinCtr->coinTotal += num;//������������
}													


/*COIN_OUT ��˸����*/
void CoinCtr(COIN_TYPE *coinCtr, uint32_t times)	
{
	if(coinCtr->coinCounter >= times)
		coinCtr->coinCounter -= times;
	else coinCtr->coinCounter = 0;
	
	if(coinCtr->coinCounter == 0) 
	{
		if(coinCtr->coinTotal) 
		{
			coinCtr->coinTotal--;
			g_service_info.coin_res--;	//Ͷ����ͬ�����٣��ڵ���ʱ����
			coinCtr->coinCounter = coinCtr->coinOffTime + coinCtr->coinOnTime;
			coinCtr->coinStatus = ON;
		}
	}
	
	if(coinCtr->coinCounter <= coinCtr->coinOffTime)
	{
		coinCtr->coinStatus = OFF;
	}
}


void CtrCoin(uint32_t timeInterval)
{
	CoinCtr(&coinCtr, timeInterval);
	
	if(coinCtr.coinStatus == ON)
	{
		if(coinCtr.coinPolarity == 0)
		    COIN_OUT_SET(HIGH);
			else
				COIN_OUT_SET(LOW);
	}else{
		if(coinCtr.coinPolarity == 0)
		  COIN_OUT_SET(LOW);
		else
			COIN_OUT_SET(HIGH);
	}
}	


static void user_coin_in_detected(void)
{
	g_service_info.coin_has++;
	g_service_info.coin_res ++;
	coin_out_action(1);   //�����������
}


static void user_tb_in_detected(void)
{
	g_service_info.tbout_has++;
}



void coin_tb_in_det(uint32_t timeInterval)
{
	static uint16_t coin_time = 0,tb_time = 0;
	static uint16_t coin_ctr_time = 0,tb_ctr_time = 0;
	u8 coin_in_level_now,tb_in_level_now;
	static u8 coin_in_level_last = 0,tb_in_level_last = 0;
	static u8 coin_in_level = 0,tb_in_level = 0;
	static u8 coin_flag = 0,tb_flag = 0;
	

	coin_in_level_now = COIN_IN_READ();

	tb_in_level_now = TB_IN_READ();

	if(coin_in_level_last != coin_in_level_now){
		if(coin_flag == 0)
		{
		  coin_in_level = coin_in_level_now;
		  coin_flag = 1;
			coin_time = 0;
		}else{
		    if(coin_ctr_time >= COIN_WIDTH_MIN)
		    {
		        COM_DBG("coin_ctr_time0: %dms\n",coin_ctr_time);
		        user_coin_in_detected();
		    }
			coin_ctr_time = 0;
			coin_flag = 0;
			coin_time = 0;		
		}	
	}

	if(tb_in_level_last != tb_in_level_now){
		if(tb_flag == 0)
		{
		  tb_in_level = tb_in_level_now;
		  tb_flag = 1;
			tb_time = 0;
		}else{
		    if(tb_ctr_time >= TB_WIDTH_MIN)
		    { 
		        COM_DBG("tb_ctr_time0: %dms\n",tb_ctr_time);
		        user_tb_in_detected();
		    }
			tb_ctr_time = 0;
			tb_flag = 0;
			tb_time = 0;		
		}
	}

	if(coin_flag == 1)
	{
	  coin_time += timeInterval;
		if(coin_in_level == coin_in_level_now)
		{
		    coin_ctr_time += timeInterval;
		}
		if(coin_time >= COIN_WIDTH_PD)
		{
		    if((coin_ctr_time >= COIN_WIDTH_MIN) && (coin_ctr_time <= COIN_WIDTH_MAX))
		    {
		        COM_DBG("coin_ctr_time1: %dms\n",coin_ctr_time);
		        user_coin_in_detected();
		    }
			coin_ctr_time = 0;
			coin_flag = 0;
			coin_time = 0;
		}
	}	

	if(tb_flag == 1)
	{
	  tb_time += timeInterval;
		if(tb_in_level == tb_in_level_now)
		{
			tb_ctr_time += timeInterval;
		}
		if(tb_time >= TB_WIDTH_PD)
		{
		    if((tb_ctr_time >= TB_WIDTH_MIN) && (tb_ctr_time <= TB_WIDTH_MAX))
		    {
		        COM_DBG("tb_ctr_time1: %dms\n",tb_ctr_time);
		        user_tb_in_detected();
		    }
			tb_ctr_time = 0;
			tb_flag = 0;
			tb_time = 0;
		}
	}
	
	tb_in_level_last = tb_in_level_now;
	coin_in_level_last = coin_in_level_now;
}



