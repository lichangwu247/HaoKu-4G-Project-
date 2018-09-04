/******************************************************************************

******************************************************************************/
#ifndef AMPM_GSM_STARTUP_H__
#define AMPM_GSM_STARTUP_H__
#include <stdint.h>


typedef struct{
	uint32_t cmdType;
	uint8_t cmd[64];
	uint8_t res[32];
	int (*callback)(uint8_t* s, uint16_t cnt);
	int timeout;
	uint32_t tryCnt;
}MOBILE_AT_SATRTUP;

typedef enum{
	THIS_MODULE_NOT_SUPPORT = 0,
	N10,
	ME3610,
}MODULE_TYPE;

uint8_t MOBILE_Startup_Init(void);
uint8_t MOBILE_Startup(const uint8_t *cmdList);
void vMOBILE_StartupTask(void *arg);
uint8_t isMOBILE_Startup_Good(void);

#endif
