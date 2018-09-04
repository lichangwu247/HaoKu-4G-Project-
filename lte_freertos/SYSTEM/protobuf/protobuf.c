#include "protobuf.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "encoding.h"


static int Serialize(unsigned char *buf, const char *fmt, va_list args)
{
	  char * s;
	  unsigned char * numptr;
    int num = 0;      /* Width of output field */
	  int16_t hParam;
	  int32_t iParam;
	  int64_t lParam;
	  float fParam;
	  static int16_t len = 0;

    for (; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *buf++ = *fmt;
            continue;
        }
        fmt++; /* This also skips first '%' */

        /* Default base */
        switch (*fmt)
        {
            case 'l':
            {
              lParam =  va_arg(args, int64_t);
							numptr = (unsigned char *)&lParam;
							memcpy(buf, numptr, sizeof(int64_t));
							num += sizeof(int64_t);
							buf += sizeof(int64_t);
							break;
            }
            case 'f':
            {
              fParam = va_arg(args, double);
							numptr = (unsigned char *)&fParam;
							memcpy(buf, numptr, sizeof(float));
							num += sizeof(float);
							buf += sizeof(float);
							break;
            }
            case 'i':
            {
              iParam = va_arg(args, int32_t);
							numptr = (unsigned char *)&iParam;
							memcpy(buf, numptr, sizeof(int32_t));
							num += sizeof(int32_t);
							buf += sizeof(int32_t);
							break;
            }
						case 'h':
            {
              hParam = va_arg(args, int32_t);
							len = hParam;
							uint32_t sinttemp = htonl(hParam);
							numptr = (unsigned char *)&sinttemp;
							numptr +=2;
							memcpy(buf, numptr, sizeof(int16_t));
							num += sizeof(int16_t);
							buf += sizeof(int16_t);
							break;
            }
            case 's':
            {
              s = va_arg(args, char *);
							memcpy(buf, s, len);
							num += len;
							buf += len;
							break;
            }

            default:
            {
                break;
            }
        }  /* end of switch (*fmt) */

    } /* end of for (; *fmt; fmt++) */
    return num;
}

int SerializeToOstream(unsigned char *buf, const char *fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = Serialize(buf, fmt, args);
    va_end(args);

    return n;
}



void* ParseForStream(uint16_t *msg_len, uint8_t **msg, uint16_t *field, uint16_t *data_len)
{
	uint16_t dat_len,field_no;
	uint8_t* pstr;
	
	if(*msg_len > 4)
	{
 		memcpy(&field_no,*msg,sizeof(uint16_t));
		*field =  ntohs(field_no);
		*msg += sizeof(uint16_t);
		//dat_len = *(uint16_t*)*msg;
		memcpy(&dat_len,*msg,sizeof(uint16_t));
		*data_len = ntohs(dat_len);
		*msg += sizeof(uint16_t);
		*msg_len -= 2*sizeof(uint16_t);
		
		if(*msg_len >= *data_len)
		{
			*msg_len -= *data_len;
			pstr = *msg;
			*msg += *data_len;
			return pstr;
		}else{
			*msg_len = 0;
			return NULL; 
		}
	}
	return NULL;
}


