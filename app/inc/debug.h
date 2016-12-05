#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "cs_types.h"

#define DB1 (1)
#define DB2 (1 << 1)
#define DB3 (1 << 2)
#define DB4 (1 << 3)
#define DB5 (1 << 4)
#define DB6 (1 << 5) 

#define GAGENT_CRITICAL    0x01
#define GAGENT_ERROR       0X01
#define GAGENT_WARNING     0X02
#define GAGENT_INFO        0X03
#define GAGENT_DEBUG       0x04
#define GAGENT_DUMP        0x05


void sys_debug(UINT8 lvl,INT8 *fmt,...);


#define DB(l,f,...) sys_debug(l,"\r\nF[%s] L[%d]"#f,__FUNCTION__,__LINE__,##__VA_ARGS__)
#define GAgent_Printf DB

#endif
