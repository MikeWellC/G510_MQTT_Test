#ifndef _CMD_H_
#define _CMD_H_

#include "cs_types.h"

typedef struct
{
	INT8   *cmd;
	INT32  (*cmd_handler)(const INT8 *str);
}CMD_T;



void cmd_proccess(const INT8 *str,UINT16 len);
void cmd_sock_connect_handle(UINT32 n1,UINT32 n2,UINT32 n3);
void cmd_sock_error_handle(UINT32 n1,UINT32 n2,UINT32 n3);
void cmd_sock_data_recv_handle(UINT32 n1,UINT32 n2,UINT32 n3);
void cmd_sock_close_ind_handle(UINT32 n1,UINT32 n2,UINT32 n3);
void cmd_sock_close_rsp_handle(UINT32 n1,UINT32 n2,UINT32 n3);
#endif
