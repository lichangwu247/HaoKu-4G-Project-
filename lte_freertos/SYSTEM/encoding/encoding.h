

#ifndef __ENCODING_H__
#define __ENCODING_H__

#include "sys.h"


int utf8_to_ucs2 (uint8_t *input,uint8_t **end_ptr);
int utf8s_to_ucs2s (int16_t *out,uint8_t *in);
uint16_t big2litel_endian(uint16_t *da,uint16_t len);
uint16_t unilen( uint16_t *uni);
uint16_t *uni_srtsrt(uint16_t *s1,uint16_t *s2);

uint16_t htons(uint16_t n);
uint16_t ntohs(uint16_t n);
uint32_t htonl(uint32_t n);
uint32_t ntohl(uint32_t n);
uint64_t htonll(uint64_t n);
uint64_t ntohll(uint64_t n);

#endif
