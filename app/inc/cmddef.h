/******************************************************************************
  Copyright (C), 2010, Shenzhen G&T Industrial Development Co.,Ltd

  File:        cmddef.h
  Author:	  shumin
  Version:  1.0
  Date:      2004.8
  
  Description:    
******************************************************************************/
#ifndef CMDDEF_H
#define CMDDEF_H

#include "string.h"

#define output_at_error(x)

#define urt_port_t void

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define TIMEOUT_UNIT   3000


typedef struct command command_t;

/* cmd handler
  * return 0 : command handle success
  * return 1 : command in proccesing
  * return -1 : is not the response for this command
  */
typedef int (*cmd_proc_t)(command_t *, const char *);

/**
 * struct command - a command object
 */
struct command {
	cmd_proc_t proc;	/**< proc: pointer of command handle function*/
	char *basicarg;		/**< basicarg: basic command argument*/
	void *exargs;		/**< exargs:external command argument */
	void *return_value;	/**< return_value: return value */
};

int init_sending_command(urt_port_t* urt);

/** send_command_wait -send a command and wait for response*/
void *send_command_wait(const char* cmdarg, void *exargs, cmd_proc_t proc,unsigned int timeout);

/* send_hex_command */ 
//int send_hex_command(char *cmdstr, int len);

/** send_command - send a command*/
int send_command(const char *cmdstr);

/*wait for "OK" and set return_value*/
int wait_ok_proc(command_t *cmd, const char *resp);

int process_response(const char *resp, int len);

/*
  *unsolicited message handle
 */
int other_response(const char *resp);


#define  ERR_STRING		9999
#define  ERR_UNKNOWN	1000
#define  ERR_TIMEOUT	1001
/** reurn the last error code*/
int  AT_GetErrorCode(void);

/**return the last error info*/
char *AT_GetErrorInfo(void);

//extern void  output_at_error(char *cmd);

#define strequsub(s1, s2) (strncmp(s1, s2, strlen(s1)) == 0)
#define	strequ(s1, s2) (strcmp(s1, s2) == 0)
//#define	strequ(s1, s2) 	strequsub(s1, s2)

/** sget_response -got some formated value from a stirng, just like the scanf()*/
int sget_response(const char* resp, const char *fmt, ...);
//int sget_resp_direct(const char* resp, const char *fmt, ...);

#define SERROR	  "ERROR"
#define SCMEERROR "+CME ERROR"
#define SCMSERROR "+CMS ERROR"
#define SWIND  "+WIND"
#define SOK		 "OK"
#define SBUSY "BUSY"
#define SNOANSWER "NO ANSWER"
#define SNOCARRIER "NO CARRIER"
#define SRING "RING"
#define SCLIP "+CLIP"
#define SSTIN "+STIN"
#define SCONNECT "CONNECT"
#define SCMTI "+CMTI"
#define SCSQ "+CSQ"
#define SCREG "+CREG"
#define SCCWA "+CCWA"
#define SWBCI "+WBCI"
#define SCGREG "+CGREG"
#define CTRL_Z 0x1a
#define SCTRL_Z "\x1a"
#define CCTRL_Z '\x1a'

/* reutrn value*/
#define SUCCESS 1
#define ATBUSY   2
#define NOANSWER 3
#define NOCARRIER 4

/*incoming call type*/
#define INCOMING_DATA 0
#define INCOMING_FAX 1
#define INCOMING_SPEECH 2

/* SPEAKER command arguments*/
#define SPEAKER_ONE 0
#define SPEAKER_TWO 1

/* GPRS type*/
#define GPRS_PPP 0
#define GPRS_IP 1

/* command return value */ 
#define RETURN_OK   (void*)0x1000
#define RETURN_ERR  (void*)0x1001

#define GSM_DISABLE  0
#define GSM_ENABLE   1

#define GSM_OFF      0
#define GSM_ON       1

#define GT880
//#define MOTO_APP
#undef MOTO_APP

#endif				/* GSMDEF_H */
