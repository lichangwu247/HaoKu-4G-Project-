
/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _RSA_CRYPT_H
#define _RSA_CRYPT_H


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



/* 包含的头文件 --------------------------------------------------------------*/
#include "crypto.h"

/* 类型定义 ------------------------------------------------------------------*/

/* 宏定义 --------------------------------------------------------------------*/
#define RSA_BIT                   1024                //RSA 1024
#define RSA_MSG_MAXLEN            (RSA_BIT/8 - 11)    //最大加密长度

/* 函数------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

int32_t RSA_Encrypt(RSApubKey_stt *P_pPubKey,
                    const uint8_t *P_pInputMessage,
                    int32_t P_InputSize,
                    uint8_t *P_pOutput);

int32_t RSA_Decrypt(RSAprivKey_stt * P_pPrivKey,
                    const uint8_t * P_pInputMessage,
                    uint8_t *P_pOutput,
                    int32_t *P_OutputSize);

int32_t  RSA_Encryption(uint8_t* msg, uint8_t msg_size, uint8_t* Encrypted);
int32_t  RSA_Decryption(uint8_t* in_output, int32_t* output_size);

#endif /* _RSA_CRYPT_H */

/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
