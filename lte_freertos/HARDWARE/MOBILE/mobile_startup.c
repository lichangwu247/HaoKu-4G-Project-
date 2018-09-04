/**********************************************************************
 *mobile module power on and startup, process
*********************************************************************/
#include "usr_task.h"
#include "led.h"
#include "mobile.h"
#include "mobile_startup.h"
#include "MQTTFreeRTOS.h"
#include "sys.h"
#include "user_config.h"

#define MOBILE_LIB_DBG( format, ...  )   // printf( format, ## __VA_ARGS__  )

extern TaskHandle_t MQTTClinetTask_Handler;


/*
0 =  AT CMD
1 =  GSM POWER PIN CLR
2 =  GSM POWER PIN SET
3 =  DELAY
4 =  GSM RESET PIN CLR
S =  STOP	

@@0$AT\r$OK$NOT USE$1000$20\r\n\
@@    0  $  AT\r $ OK  $ NOT USE   $  1000    $   20      \r\n\
MARK|TYPE|  CMD  | RES |  CALLBACK |  TIMEOUT |  TRYCNT  |

CALLBACK:
1: mobile_getIMEI;
2: mobile_getCCID;
3: mobile_getCCLK;
4: mobile_getCSQ;
*/

const uint8_t AtCmdStart_N10[] = {
	"@@0$AT\r$OK$NOT USE$1000$20\r\n\
	@@S"
};

/*
	@@0$AT+ZGINIT\r$OK$0$1000$3\r\n\
	@@0$AT+ZGMODE=3\r$OK$0$1000$3\r\n\
	@@0$AT+ZGFIXRATE=1,2\r$OK$0$1000$3\r\n\
	@@0$AT+ZGQOS=50,255\r$OK$0$1000$3\r\n\
	@@0$AT+ZGPSR=1\r$OK$0$1000$3\r\n\
	@@0$AT+ZGURL= supl.qxwz.com:7275\r$OK$0$1000$3\r\n\
	@@0$AT+ZGNMEA=2\r$OK$0$1000$3\r\n\
	@@0$AT+ZGRUN=1\r$+ZGPSR:$5$270000$1\r\n\
*/


//@@0$$+ZREADY$NOT USE$1000$20\r\n\@@3$15000\r\n\
//@@0$AT+ZIPCALL=1\r$OK$0$10000$3\r\n\
//@@0$AT+ZIPCALL?\r$+ZIPCALL: 1$0$1000$3\r\n\

//��һ������Ϊ "��������" $ "����" $ "���ز���" $ "�ص�����" $ "��ʱʱ��" $ "���Դ���" 
const uint8_t AtCmdStart_ME3610[] = {
	"@@3$15000\r\n\
	@@0$AT\r$OK$0$1000$20\r\n\
	@@0$ATI\r$Model: $1$1000$3\r\n\
	@@0$ATE0\r$OK$0$1000$3\r\n\
	@@0$AT+CFUN=1\r$OK$0$1000$3\r\n\
	@@0$AT+CPIN=?\r$OK$0$1000$3\r\n\
	@@0$AT+ZGETICCID\r$+ZGETICCID: $2$500$10\r\n\
	@@0$AT+CTZU=1\r$OK$0$1000$3\r\n\
	@@0$AT+CREG?\r$+CREG: 0,1$0$1000$30\r\n\
	@@0$AT+CSQ\r$+CSQ: $4$1000$10\r\n\
	@@0$AT+CCLK?\r$+CCLK: $3$1000$5\r\n\
	@@0$AT+ZIPCFG=ctnet\r$OK$0$1000$3\r\n\
	@@0$AT+ZIPCALL=1\r$+ZIPCALL: 1$0$10000$3\r\n\
	@@0$AT+CGMR\r$OK$0$1000$3\r\n\
	@@S"
};

const uint8_t AtCmdStart_ME3610_Add_GPRS[] = {
	"@@3$15000\r\n\
	@@0$AT\r$OK$0$1000$20\r\n\
	@@0$ATI\r$Model: $1$1000$3\r\n\
	@@0$ATE0\r$OK$0$1000$3\r\n\
	@@0$AT+CFUN=1\r$OK$0$1000$3\r\n\
	@@0$AT+CPIN=?\r$OK$0$1000$3\r\n\
	@@0$AT+ZGETICCID\r$+ZGETICCID: $2$500$10\r\n\
	@@0$AT+CTZU=1\r$OK$0$1000$3\r\n\
	@@0$AT+CREG?\r$+CREG: 0,1$0$1000$30\r\n\
	@@0$AT+CSQ\r$+CSQ: $4$1000$10\r\n\
	@@0$AT+CCLK?\r$+CCLK: $3$1000$5\r\n\
	@@0$AT+ZIPCFG=ctnet\r$OK$0$1000$3\r\n\
	@@0$AT+ZIPCALL=1\r$+ZIPCALL: 1$0$10000$3\r\n\
	@@0$AT+CGMR\r$OK$0$1000$3\r\n\
	@@0$AT+ZGINIT\r$OK$0$1000$3\r\n\
	@@0$AT+ZGMODE=3\r$OK$0$1000$3\r\n\
	@@0$AT+ZGFIXRATE=1,2\r$OK$0$1000$3\r\n\
	@@0$AT+ZGQOS=50,255\r$OK$0$1000$3\r\n\
	@@0$AT+ZGPSR=1\r$OK$0$1000$3\r\n\
	@@0$AT+ZGURL= supl.qxwz.com:7275\r$OK$0$1000$3\r\n\
	@@0$AT+ZGNMEA=2\r$OK$0$1000$3\r\n\
	@@0$AT+ZGRUN=1\r$+ZGPSR: $5$30000$1\r\n\
	@@S"
};



Timer tMobileStartupTaskDelay;

uint8_t *AtCmdStart_pt;

typedef enum{
	MOBILE_STARTUP_INIT_PHASE = 0,
	MOBILE_STARTUP_TURN_ON,
	MOBILE_STARTUP_TURN_ON1,
	MOBILE_STARTUP_GET_CMD_PHASE,
	MOBILE_STARTUP_IDLE_PHASE
} MOBILE_STARTUP_PHASE_TYPE;

MOBILE_STARTUP_PHASE_TYPE MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE;

MOBILE_AT_SATRTUP at_cmd;


uint8_t MOBILE_Startup_Init(void)
{
	MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE;
	return 0;
}


uint8_t isMOBILE_Startup_Good(void)
{
	return MOBILE_StartupPhase == MOBILE_STARTUP_IDLE_PHASE;;
}


uint8_t MOBILE_Startup(const uint8_t *cmdList)
{
	uint32_t callback;
	uint32_t delay;
	static uint32_t tryCnt = 0;
	switch(MOBILE_StartupPhase)
	{
        case MOBILE_STARTUP_INIT_PHASE:
					MOBILE_LIB_DBG("\r\n TRY TO TURN ON TIMES = %d \r\n",tryCnt);
								
					if(tryCnt >= 3)
					{
						tryCnt = 0;
						//MOBILE_POWER(MOBILEOFF);
						MOBILE_RESET_ON();//ģ�鸴λ
						TimerInit(&tMobileStartupTaskDelay);   //��ʹ����ʱ��
						TimerCountdownMS(&tMobileStartupTaskDelay, 1200);//��ʱ�����¼���1.2s
						MOBILE_StartupPhase = MOBILE_STARTUP_TURN_ON;
						MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_TURN_ON \r\n");
					}else{
						TimerInit(&tMobileStartupTaskDelay);
						TimerCountdown(&tMobileStartupTaskDelay, 1);
						MOBILE_StartupPhase = MOBILE_STARTUP_TURN_ON1;
						MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_TURN_ON1 \r\n");
					}
					tryCnt++;
        break;
								
        case MOBILE_STARTUP_TURN_ON:
					if(TimerIsExpired(&tMobileStartupTaskDelay))//1.2S ��ʱ�������
          {
						//MOBILE_POWER(MOBILEON);
						MOBILE_RESET_OFF();//ģ�鸴λdisable
						TimerInit(&tMobileStartupTaskDelay);
						TimerCountdown(&tMobileStartupTaskDelay, 32); //��ʱ�����¼���32ms
						MOBILE_StartupPhase = MOBILE_STARTUP_TURN_ON1;
						MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_TURN_ON1 \r\n");
          }
        break;
								
        case MOBILE_STARTUP_TURN_ON1:
					if(TimerIsExpired(&tMobileStartupTaskDelay))//32msʱ�䵽�����
					{
						AtCmdStart_pt = (uint8_t *)cmdList; 	//ָ�������б�
				    TimerInit(&tMobileStartupTaskDelay);
						TimerCountdownMS(&tMobileStartupTaskDelay, 5);//������ʱ5ms
				    MOBILE_StartupPhase = MOBILE_STARTUP_GET_CMD_PHASE;
						MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_GET_CMD_PHASE \r\n");
					}
        break;
								
        case MOBILE_STARTUP_GET_CMD_PHASE:
					if (TimerIsExpired(&tMobileStartupTaskDelay))//5msʱ�䵽��
          {
							memset(&at_cmd,0,sizeof(MOBILE_AT_SATRTUP));
              AtCmdStart_pt = (uint8_t *)strstr((char *)AtCmdStart_pt,"@@");
              AtCmdStart_pt += 2;
              if(AtCmdStart_pt == NULL) //��������б�Ϊ��
              {
								MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE; //���»ص���ʼ���׶�
								MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_INIT_PHASE \r\n");
                break;
              }
							at_cmd.cmdType = AtCmdStart_pt[0];// ȡ����������
              AtCmdStart_pt++;
							
              switch(at_cmd.cmdType)
              {
								case '0':
									sscanf((const char *)AtCmdStart_pt,"$%[^$]$%[^$]$%d$%d$%d\r\n",
                                                              at_cmd.cmd,
                                                              at_cmd.res,
                                                              &callback,
                                            									&at_cmd.timeout,
                                                              &at_cmd.tryCnt);
									MOBILE_LIB_DBG("atcmd.cmd: %s\r\n",at_cmd.cmd);
									MOBILE_LIB_DBG("at_cmd.res: %s\r\n",at_cmd.res);
									MOBILE_LIB_DBG("callback: %d\r\n",callback);
									MOBILE_LIB_DBG("at_cmd.timeout: %d\r\n",at_cmd.timeout);
									MOBILE_LIB_DBG("at_cmd.tryCnt: %d\r\n",at_cmd.tryCnt);

                  if(at_cmd.timeout == 0) at_cmd.timeout = 1000;
									switch(callback)  //���ûص�����
									{
										case 0:
											at_cmd.callback = NULL;
										break;
										case 1:
											at_cmd.callback = mobile_getIMEI;
										break;
										case 2:
											at_cmd.callback = mobile_getCCID;
										break;
										case 3:
											at_cmd.callback = mobile_getCCLK;
										break;
										case 4:
											at_cmd.callback = mobile_getCSQ;
										break;
										case 5:
											at_cmd.callback =GPRS_GET_CUR_LOCT;
											break;
										default:
											at_cmd.callback = NULL;
										break;
									}
									if(0 != at_send_cmd( 0, at_cmd.cmd,
															  at_cmd.res,
																at_cmd.callback,
																at_cmd.timeout,
																at_cmd.tryCnt)){
																	MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE;  //����ATָ����ݷ���ִֵ�лص�����
																}
                break;
											
                case '1':
									MOBILE_POWER(MOBILEOFF);;
								break;
								
								case '2':
									MOBILE_POWER(MOBILEON);
								break;
								
								case '3':
									sscanf((const char *)AtCmdStart_pt,"$%d",&delay);
								  MOBILE_LIB_DBG("at_cmd.delay: %d\r\n",delay);
									if(1 == tryCnt)
									{
										TimerInit(&tMobileStartupTaskDelay);
										TimerCountdownMS(&tMobileStartupTaskDelay, delay);//������ʱ
									}								  
                break;
								
								case '4':
									//MOBILE_RESET();
								break;
								
                case 'S':
									tryCnt = 0;
								  MOBILE_StartupPhase = MOBILE_STARTUP_IDLE_PHASE;//ģ���ʼ������Ϊ����
									MOBILE_LIB_DBG("\r\n MOBILE_STARTUP_IDLE_PHASE \r\n");
								break;
								default:
									MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE;
                  break;
							}
	          }
        break;
								
        case MOBILE_STARTUP_IDLE_PHASE:
				break;
				
        default:
					MOBILE_StartupPhase = MOBILE_STARTUP_INIT_PHASE;
				break;
	}
	return 0;
}

extern TaskHandle_t MOBILETask_Handler;
extern TaskHandle_t MOBILE_GPRS_Task_Handler;

void vMOBILE_StartupTask(void *arg)
{
	MOBILE_Startup_Init();  //��ʼ������
	
	while(1)
	{

#ifdef USE_GPRS		  //���ʹ��GPRS��λ���ܣ���user_config.h�д򿪶���
		MOBILE_Startup(AtCmdStart_ME3610_Add_GPRS);
#else
		MOBILE_Startup(AtCmdStart_ME3610);
#endif
		vTaskDelay(50);	//��ʱ50ms��Ҳ����50��ʱ�ӽ���
		
		if(isMOBILE_Startup_Good())
		{
			vTaskResume( MQTTClinetTask_Handler );//�ָ�MQTT�ͻ�������
			vTaskResume( MOBILE_GPRS_Task_Handler);//�ָ�GPRS����
			vTaskDelete( NULL );	  //ɾ��������
			MOBILETask_Handler = NULL;
		}	 		
	}
}


