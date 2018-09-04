#ifndef __PROTOBUF_H__
#define __PROTOBUF_H__

#include "sys.h"

int SerializeToOstream(unsigned char *buf, const char *fmt, ...);
void* ParseForStream(uint16_t *msg_len, uint8_t **msg, uint16_t *field, uint16_t *data_len);

#endif
