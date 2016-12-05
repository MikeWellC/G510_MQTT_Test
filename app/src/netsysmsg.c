/******************************************************************************
  Copyright (C), 2010, Shenzhen G&T Industrial Development Co.,Ltd

  File:        netsysmsg.c
  Author:	  shumin
  Version:  1.0
  Date:      2004.8
  
  Description:    Unrequest Result Code process
******************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "cmddef.h"
#include "sys_services.h"

static void ok_proc(const char *resp)
{
}
static void busy_proc(const char *resp)
{
}
static void no_answer(const char *resp)
{
}
static void no_carrier(const char *resp)
{
}

/**
 network register handler
 +CREG:0-5 see event.h
 */
static void creg_proc(const char *resp)
{
	int	creg = 0;

	if (sget_response(resp, "d", &creg) > 0)
	{
		if (creg == 5)
			creg = 1;
	}
}

/**
 *gprs handler
 +CGREG:0-5 see event.h
*/
static void cgreg_proc(const char* resp)
{
	int	creg = 0;

	if (sget_response(resp, "d", &creg) > 0)
	{
		if (creg == 5)
			creg = 1;
	}
}

static void ring_proc(const char *resp)
{
}

/**
 *incoming call number
 +CLIP:"+875545455555",123
 +CLIP: "02082345708",129,,,"soutec"
 */
#define MAX_INPHONE_LEN	 32
static char incoming_phone[MAX_INPHONE_LEN];

static void clip_proc(const char *resp)
{
	memset(incoming_phone, 0, MAX_INPHONE_LEN);
	if (sget_response(resp, "32s", incoming_phone) > 0)
	{
	}
}

/**new sms
 +CMTI:"SM",3
 */
#define SSM "SM"
#define SBM "BM"

#define IN_SM 0	
#define IN_BM 1	

static void cmti_proc(const char *resp)
{
	char		mem[8];
	int		no;

	if (sget_response(resp, "8sd", mem, &no) > 0) 
	{
	}
}

typedef void (*proc_t)(const char *);

typedef struct {
	char	*idstr;
	proc_t	proc;
} notify_proc_t;

static notify_proc_t at_proc[] = {
	{"+CGREG",		cgreg_proc},
	{"+CLIP",			clip_proc},
	{"+CMTI",		cmti_proc},
	{"+CREG",		creg_proc},
	{"BUSY",			busy_proc},
	{"NO ANSWER",	no_answer},
	{"NO CARRIER",	no_carrier},
	{"OK",			ok_proc},
	{"RING",			ring_proc},
};


static const int MAX_POS = (sizeof(at_proc)/sizeof(at_proc[0]) -1);


static notify_proc_t *find_command(const char *resp)
{
	int		start, end, mid, res;

	
	res = strncmp(at_proc[0].idstr, resp, strlen(at_proc[0].idstr));
	
	if (res == 0) 
		return &at_proc[0];
	if (res > 0)
		return NULL; 

	res = strncmp(at_proc[MAX_POS].idstr, resp, strlen(at_proc[MAX_POS].idstr));
	if (res == 0) 
		return &at_proc[MAX_POS];
	if (res < 0)
		return NULL;

	start = 0;
	end   = MAX_POS;
	while (1)
	{
		mid = ((start+end)>>1);
		if (mid == start)
			return NULL;

		res = strncmp(at_proc[mid].idstr, resp, strlen(at_proc[mid].idstr));
		if (res == 0)
			return &at_proc[mid];
		
		if (res < 0) 
			start = mid;
		else
			end = mid;
	}
}


/*
  *unsolicited message handler
 */
int other_response(const char *resp)
{

	notify_proc_t	*find_proc;


	find_proc = find_command(resp);
	if (find_proc) 
	{
		find_proc->proc(resp);
	}

	return 1;
}
