
#include "hash_crypt.h"
#include "sys.h"



/**
  * @brief  MD5 HASH digest compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: can be HASH_SUCCESS if success or one of
  *         HASH_ERR_BAD_PARAMETER, HASH_ERR_BAD_CONTEXT,
  *         HASH_ERR_BAD_OPERATION if error occured.
  */
int32_t STM32_MD5_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                               uint8_t *MessageDigest, int32_t* MessageDigestLength)
{
  MD5ctx_stt P_pMD5ctx;
  uint32_t error_status = HASH_SUCCESS;

  /* Set the size of the desired hash digest */
  P_pMD5ctx.mTagSize = CRL_MD5_SIZE;

  /* Set flag field to default value */
  P_pMD5ctx.mFlags = E_HASH_DEFAULT;

  error_status = MD5_Init(&P_pMD5ctx);

  /* check for initialization errors */
  if (error_status == HASH_SUCCESS)
  {
    /* Add data to be hashed */
    error_status = MD5_Append(&P_pMD5ctx,
                              InputMessage,
                              InputMessageLength);

    if (error_status == HASH_SUCCESS)
    {
      /* retrieve */
      error_status = MD5_Finish(&P_pMD5ctx, MessageDigest, MessageDigestLength);
    }
  }

  return error_status;
}




/**
  * @brief  SHA1 HMAC compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  HMAC_key: pointer to key used in the HMAC computation
  * @param  HMAC_keyLength: HMAC key length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: can be HASH_SUCCESS if success or one of
  *         HASH_ERR_BAD_PARAMETER, HASH_ERR_BAD_CONTEXT,
  *         HASH_ERR_BAD_OPERATION if error occured.
  */
int32_t STM32_SHA1_HMAC_Compute(uint8_t* InputMessage,
                          uint32_t InputMessageLength,
                          uint8_t *HMAC_key,
                          uint32_t HMAC_keyLength,
                          uint8_t *MessageDigest,
                          int32_t* MessageDigestLength)
{
  HMAC_SHA1ctx_stt HMAC_SHA1ctx;
  uint32_t error_status = HASH_SUCCESS;

  /* Set the size of the desired MAC*/
  HMAC_SHA1ctx.mTagSize = CRL_SHA1_SIZE;

  /* Set flag field to default value */
  HMAC_SHA1ctx.mFlags = E_HASH_DEFAULT;

  /* Set the key pointer in the context*/
  HMAC_SHA1ctx.pmKey = HMAC_key;

  /* Set the size of the key */
  HMAC_SHA1ctx.mKeySize = HMAC_keyLength;

  /* Initialize the context */
  error_status = HMAC_SHA1_Init(&HMAC_SHA1ctx);

  /* check for initialization errors */
  if (error_status == HASH_SUCCESS)
  {
    /* Add data to be hashed */
    error_status = HMAC_SHA1_Append(&HMAC_SHA1ctx,
                                    InputMessage,
                                    InputMessageLength);

    if (error_status == HASH_SUCCESS)
    {
      /* retrieve */
      error_status = HMAC_SHA1_Finish(&HMAC_SHA1ctx, MessageDigest, MessageDigestLength);
    }
  }

  return error_status;
}



/*
md5
*/ 
uint32_t md5_calc(uint8_t *data, uint32_t len, uint8_t *digest)
{
	int32_t status = HASH_SUCCESS;
	int32_t MessageDigestLength = 0;
	uint8_t out[CRL_MD5_SIZE] = {0}, i;
	
	const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

  /* DeInitialize STM32 Cryptographic Library */
  Crypto_DeInit();

  status = STM32_MD5_HASH_DigestCompute((uint8_t*)data,
                                  len, out,
                                  &MessageDigestLength);
	
	if(status == HASH_SUCCESS)
	{
		for( i=0; i < CRL_MD5_SIZE; i++ )
		{
			digest[i * 2] = hex[out[i] >> 4];
  		digest[i * 2 + 1] = hex[out[i] & 0xF];
	  }
	}
	return status;
}


/*
hmac_sha1
*/
uint32_t hmac_sha1_calc(uint8_t *data, uint32_t data_len, 
                        uint8_t *key,uint8_t key_len, 
                        uint8_t *digest)
{
	int32_t status = HASH_SUCCESS;
	int32_t MessageDigestLength = 0;
	uint8_t out[CRL_SHA1_SIZE] = {0}, i;
	
	const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

  /* DeInitialize STM32 Cryptographic Library */
  Crypto_DeInit();

  status = STM32_SHA1_HMAC_Compute((uint8_t*)data,
                                  data_len, key, key_len, out,
                                  &MessageDigestLength);
	
	if(status == HASH_SUCCESS)
	{
		for( i=0; i < CRL_SHA1_SIZE; i++ )
		{
			digest[i * 2] = hex[out[i] >> 4];
  		digest[i * 2 + 1] = hex[out[i] & 0xF];
	  }
	}
	
	return status;
}


/***********************************************************/

