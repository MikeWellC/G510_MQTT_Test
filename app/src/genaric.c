/******************************************************************************
  Copyright (C), 2010, Shenzhen G&T Industrial Development Co.,Ltd

  File:   Generic.c
  Author:	  shumin
  Version:  1.0
  Date:      2005.8
  
  Description:    Base of AT command process
******************************************************************************/

#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "sys_services.h"
#include "cmddef.h"
#include "debug.h"

//#include "task.h"
//#include "uart.h"

#define  _SDEBUG



/** struct send_command - the command while sending now */
typedef struct sending_command {
	command_t *command;	/**< command: command*/
	int	send_lock;	/**< send_lock: sending lock */
	int	ok_lock;		/**< ok_lock: a lock wait for ok or timeout*/
	int	wait_timer;
	int		cmd_flag;	/**< cmd_flag: */
} sending_command_t;


/** sending_cmd: the command which send and wait response */
static sending_command_t sending_cmd;

/** current_command -curent command in proccessing*/
static command_t current_command;

static void wait_timeout(void *arg);

/**wait for "OK" */
int wait_ok_proc(command_t *cmd, const char *resp)
{
//	debug_out_str(DM_RUNINFO, "888");
	if (strequ("OK", resp)) {
//		debug_out_str(DM_RUNINFO, "777");
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

INLINE void before_command(void)
{
	sys_sem_wait(sending_cmd.send_lock);
	sys_sem_wait(sending_cmd.ok_lock);
}

INLINE void after_command(void)
{
	sys_sem_signal(sending_cmd.ok_lock);
	sys_sem_signal(sending_cmd.send_lock);
}


int send_command(const char *cmdstr)
{
	unsigned int cmd_len = strlen(cmdstr);
	return sys_at_send(cmdstr,cmd_len);
}

/* error code and error info. */ 

static int modem_err_code = ERR_UNKNOWN;
static char modem_err_info[128];

/**
 *send_command_wait - send a command and wait for responce or timeout
 *@param cmdarg: basic command argument
 *@param exargs: external command argument
 *@param timeout: timeout(ms)¡£-1 means wait forever
 *@return command return 
 */
void *send_command_wait(const char* cmdarg, void *exargs, cmd_proc_t proc,unsigned int timeout)
{
	void	*rv = NULL;
	before_command();

	if (NULL == proc) 
	{
		send_command(cmdarg);
		rv = RETURN_OK;
	}
	else 
	{
		current_command.proc = proc;
		current_command.basicarg = (char*)cmdarg;
		current_command.exargs   = exargs;
		current_command.return_value = NULL;
		sending_cmd.cmd_flag = 1;

		if (timeout < 1000)
		{
			timeout = 1000;
		}
		if (-1 != send_command(cmdarg))
		{
			sending_cmd.wait_timer = sys_timer_new(timeout,wait_timeout,NULL);
			sys_sem_wait(sending_cmd.ok_lock);
			sys_timer_free(sending_cmd.wait_timer);
		}
		rv = sending_cmd.command->return_value;
		sending_cmd.cmd_flag = 0;
		current_command.proc = NULL;
	}

	if (rv == NULL)
	{		/* time out */ 
		modem_err_code = ERR_TIMEOUT;
		strcpy(modem_err_info, "Receive Timeout!");
	}
	
	if (rv == RETURN_ERR) 
	{
		/* error code return */ 
		rv = NULL;
	}
	
	after_command();
	return rv;
}

/**
 *is_error - check for error
 */
static int check_error(const char *resp)
{
	if (strequsub(SCMEERROR, resp) || strequsub(SCMSERROR, resp)) 
	{
		sget_response(resp, "128s", modem_err_info);
		if ((*modem_err_info >= '0') && (*modem_err_info <= '9')) 
		{
			char	*p;
			modem_err_code = strtol(modem_err_info, &p, 10);
		}
		else 
		{
			modem_err_code = ERR_STRING;
		}
		return 1;
	}
	else if (strequsub(SERROR, resp)) 
	{
		modem_err_code = ERR_UNKNOWN;
		strcpy(modem_err_info, "Unknown error!");
		return 1;
	}
	return 0;
}

int  AT_GetErrorCode(void)
{
	return modem_err_code;
}

char *AT_GetErrorInfo(void)
{
	return modem_err_info;
}

/**
 *process_response - process response from module, if has command wait, let the 
 *command procedure process it, the function called by recv thread
 *@param resp: the chars recved from module
 */
int process_response(const char *resp, int len)
{
	int		stat;

	if (sending_cmd.cmd_flag) 
	{
		stat = -1;
		if (sending_cmd.command->proc != NULL) 
		{
			stat = sending_cmd.command->proc(sending_cmd.command, resp);
			if ((stat == -1) && check_error(resp))
			{
				sending_cmd.command->return_value = RETURN_ERR;
				stat = 0;
			}
		}
		if (stat == -1)
		{
			return other_response(resp);
		}
		else if (stat == 0) 
		{
			sys_sem_signal(sending_cmd.ok_lock);
		}
	}
	else
	{
		return other_response(resp);
	}
	return 1;
}

static void wait_timeout(void *arg)
{
	sys_sem_signal(sending_cmd.ok_lock);
}

/**
 *init_sending_command - command module init
 */
int init_sending_command(urt_port_t* urt)
{
	static char  init_urt_lock = 0;


	if (init_urt_lock == 1)
	{
		return 0;
	}
	
	sending_cmd.send_lock = sys_sem_new(1);
	sending_cmd.ok_lock =  sys_sem_new(1);
	sending_cmd.cmd_flag = 0;
	sending_cmd.wait_timer = -1;
	current_command.proc = NULL;
	sending_cmd.command = &current_command;

	init_urt_lock = 1;
	return 0;
}

#define CMD_BUFLEN	1024
char cmd_buffer[CMD_BUFLEN+4];

void at_handle(INT8 *data,UINT16 data_len)
{
	char  	*pStart, *pEnd;
	int		length, res;
	pStart = strchr(data, '\r');
	if (NULL == pStart)
		return;

	pStart += 2;
	if(data_len < pStart - data)
	{
	    return;
	}
	
	while (1) {
		pEnd = strchr(pStart, '\r');
		if (NULL == pEnd) {
			length = data_len - (int)(pStart-data);
			strncpy(cmd_buffer, pStart, length);
			cmd_buffer[length] = 0;
			res = process_response(cmd_buffer, length);

			break;
		}
		else {
			length = (int)(pEnd - pStart);
			strncpy(cmd_buffer, pStart, length);
			cmd_buffer[length] = 0;
			res = process_response(cmd_buffer, length);
			pStart = pEnd + 2;
			if ((pStart-data) >= data_len)
				break;
		}
	}
	
	return;

}


