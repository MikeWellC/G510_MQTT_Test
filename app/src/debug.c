#include "debug.h"
#include "sys_services.h"

#define DB_BUFF_SIZE 256

const static UINT32 DB_V = DB2 | DB3; 

/*sys debug , print to uart*/
void sys_debug(UINT8 lvl,INT8 *fmt,...)
{
	UINT16 len;
	va_list ap;
	UINT8 db[DB_BUFF_SIZE];
	
	if(DB_V & lvl)
	{
		va_start(ap,fmt);
		len  = (UINT16)sys_vsnprintf(db,DB_BUFF_SIZE,fmt,ap);
		va_end(ap);
		sys_uart_output(0,db,len);
	}
}
