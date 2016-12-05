/******************************************************************************
  Copyright (C), 2010, Shenzhen G&T Industrial Development Co.,Ltd

  File:        manage.c
  Author:	  shumin
  Version:  1.0
  Date:      2004.8
  
  Description:    AT command about  network
******************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "cmddef.h"
#include "manage.h"
#include "sys_services.h"

static int cgmi_proc(command_t *cmd, const char* resp)
{
	if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	else {
		sget_response(resp, "s", cmd->exargs);
		return 1;
	}
/*
	if (strequsub("+CGMI", resp)) {
		sget_response(resp, "s", cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
*/
	return -1;
}

/** 
 *AT_GetManufactory  --- get the name of manufacturer 
 *@return            -1 fail, 0 success
 */
int AT_GetManufactory(char *name)
{
	void *rv;

	rv = send_command_wait("AT+CGMI\r", name, cgmi_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

static int cgsn_proc(command_t *cmd, const char* resp)
{
	if (strequsub("+CGSN", resp)) {
		sget_response(resp, "s", cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}
/** 
 *AT_GetSerialNumber  --- get serial number
 *@return            -1 fail, 0 success
 */
int AT_GetSerialNumber(char *name)
{
	void *rv;

	rv = send_command_wait("AT+CGSN\r", name, cgsn_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}


static int cimi_proc(command_t *cmd, const char* resp)
{
	if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	else {
		sget_response(resp, "16s", cmd->exargs);
		return 1;
	}
	return -1;
#if 0
	moto
	if (strequsub("+CIMI", resp)) {
		sget_response(resp, "16s", cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
#endif

}
/** 
 *AT_GetIMSI  ---get imsi
 *@return            -1 fail, 0 success
 */
int AT_GetIMSI(char *name)
{
	void *rv;

	rv = send_command_wait("AT+CIMI\r", name, cimi_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}
#if 0
static int cscs_proc(command_t *cmd, const char* resp)
{
	if (strequsub("+CSCS", resp)) {
		sget_response(resp, "s", cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}
#endif

/**
 *+CGREG command handler
 */
static int cgreg_proc(command_t *cmd, const char* resp)
{
	int		set;

	if (strequsub("+CGREG", resp)) {
		sget_response(resp, "dd", &set, (int*)cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

int AT_GetGRegister(void)
{
#ifdef NOT_FOR_IDEN
//	remove for iden
	int		id;
	void	*rv;

	rv = send_command_wait("AT+CGREG?\r", &id, cgreg_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;

	if (id == 5)		//  roam 
		id = 1;
	return id;
#else
	return 1;
#endif
}

/**
 *Gprs_active - attach or detach gprs service
 *@param id identified gprs pdp context 1-32
 *@param state TRUE means active FALSE means deactive 
*@return 0 success, other fail
 */
int AT_GPRSActive(int id, int state)
{
	void	*rv;
	char	cmd[16];

	sprintf(cmd, "AT+CGACT=%d\r", state);
	rv = send_command_wait(cmd, NULL, wait_ok_proc, TIMEOUT_UNIT);

	if (rv == NULL)
		return -1;
	else
		return 0;	
}

static int cgatt_proc(command_t *cmd, const char* resp)
{
	int		id;

	if (strequsub("+CGREG", resp)) {
		sget_response(resp, "d", &id);
		if (id == 1) {
			cmd->return_value = RETURN_OK;
			return 0;
		}
		return 1;
	}
	else if (strequ("OK", resp)) {
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

/**
 *Gprs_attach - attach or detach gprs service
 *@param att TRUE means attach FALSE means detach
 *@return 0 success, other fail
 */
int AT_GPRSAttach(int att, int timeout)
{
//	return 0;
	void	*rv;
	char	cmd[16];
	sprintf(cmd, "AT+CGATT=%d\r", att);
	rv = send_command_wait(cmd, NULL, cgatt_proc, timeout*1000);

	if (rv == NULL)
		return -1;
	return 0;	
}


/**
 *Gprs_setcount - configure gprs , AT+CGDCONT
 *@return 0 success, other fail
 */
int AT_SetGPRSLink(int id, int type, char* acname)
{
	void	*rv;
	char	cmd[80];

	/* only IP support */ 
	sprintf(cmd, "AT+CGDCONT=%d,\"%s\",\"%s\"\r", id, "IP", acname);
	rv = send_command_wait(cmd, NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;	
}

#ifndef	MOTO_APP
#if 0
static int tsim_proc(command_t *cmd, const char* resp)
{
	/* Ã»ÓÐÃ°ºÅ */ 
	if (strequsub("%TSIM", resp)) {
		sget_response(resp+5, "d", (int*)cmd->exargs);
		cmd->return_value = RETURN_OK;
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}
#endif
#endif

/** check sim */ 
int AT_CheckSIM(void)
{
#ifdef NOT_FOR_IDEN
//	remove for iden
#ifdef	MOTO_APP
	int		id;
	id = AT_CheckPin();
	if (id > 0)
		return 1;
	return -1;
#else
	void	*rv;
	int		id;
	
	id = -1;
	rv = send_command_wait("AT%TSIM\r", &id, tsim_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return id;
#endif

#else
	return 1;
#endif
}

/*+CPWD handler*/
static int cpwd_proc(command_t *cmd, const char* resp)
{
	if (strequsub("+CPWD:", resp)) {
		// process here
		return 1;
	}
	else if (strequ("OK", resp)) {
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

/*change password*/
int AT_ChangePwd(char *oldpwd, char *newpwd)
{
	void *rv;
	char strcmd[32];

	sprintf(strcmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"\r", oldpwd, newpwd);
	rv = send_command_wait(strcmd, NULL, cpwd_proc, TIMEOUT_UNIT);
	if (rv == NULL) 
		return -1;
	return 0;
}


/** +cpin handler function
 *@param cmd   command struct
 *@param resp  responce
 *@return    0 --complete; 1 --need proccess; -1 --not for us
 */
/* +CPIN:READY */

static int cpin_proc(command_t *cmd, const char* resp)
{
	if (strequsub("+CPIN", resp)) {
		sget_response(resp, "s", cmd->exargs);
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 1;
	}
	else if (strequ("OK", resp)) {
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

/** 
 *AT_CheckPin  ---check sim status
 *@return            -1 fail or return the sim status
 */
int AT_CheckPin(void)
{
	void *rv;
	char status[32];

	rv = send_command_wait("AT+CPIN?\r", status, cpin_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		return -1;
	}

	if (strequ("READY", status))  /* ME is not pending for any password */
		return PIN_OK;
	else if (strequ("SIMPIN", status))  /* CHV1 is required */
		return PIN_PIN1;
	else if (strequ("SIMPUK", status))  /* PUK1 is required */
		return PIN_PUK1;
	else if (strequ("SIMPIN2", status))  /* CHV2 is required */
		return PIN_PIN2;
	else if (strequ("SIMPUK2", status))  /* PUK2 is required */
		return PIN_PUK2;
	else if (strequ("PH-SIMPIN", status)) /* SIM lock (phone-to-SIM) is required */
		return PIN_LOCK;
	else if (strequ("PH-NETPIN", status)) /* Network personnalisation is required */
		return PIN_NETWORK;
	return 0;
}

/** 
 *AT_InputPin ---  input puk and pin
 *@param            puk 
 *@param            pin 
 *@return            -1 fail, 0 success
 */
int AT_InputPin(char *puk, char *pin)
{
	void *rv;
	char strcmd[64];

	if (puk == NULL)
		sprintf(strcmd, "AT+CPIN=\"%s\"\r", pin);
	else
		sprintf(strcmd, "AT+CPIN=\"%s\",\"%s\"\r", puk, pin);
	rv = send_command_wait(strcmd, NULL, cpin_proc, TIMEOUT_UNIT);
	if (rv == NULL) 
		return -1;
	return 0;
}


/** csq handler
 *@param cmd    command struct
 *@param resp   responce
 *@return    0 --complete; 1 --need proccess; -1 --not for us
 */
/* +CSQ:15,0 */
static int csq_proc(command_t *cmd, const char* resp)
{
	int		stat = 99, val;
	if (strequsub("+CSQ:", resp)) {
		sget_response(resp, "dd", &val, &stat);
		if (val == 99)
			val = 0;
//		if (stat != 99)
			*((int*)cmd->exargs) = val;
//		else
//			*((int*)cmd->exargs) = val;		// -1
		return 1;
	} 
	else if (strequ("OK", resp)) {
		if (cmd->return_value == NULL)
			cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

/** 
 *AT_GetSignal  --- get the signal strength 
 *@return            -1 fail, other signal strength 
 */
int AT_GetSignal(void)
{
	int		csq;
	void	*rv;

	csq = 0;
	rv = send_command_wait("AT+CSQ\r", &csq, csq_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		return -1;
	}
	return csq;
}


//lliangzt mod
/*
at+mcell=0,21

OK

MCC:460,MNC:  0,LAC:09339,Cell ID:03600,BSIC:019,(P)BCCH ARFCN:0060,RxLev:075,RxDbm:-35
MCC:460,MNC:  0,LAC:09339,Cell ID:04361,BSIC:047,(P)BCCH ARFCN:0068,RxLev:034,RxDbm:-76
MCC:460,MNC:  0,LAC:09339,Cell ID:00000,BSIC:002,(P)BCCH ARFCN:0054,RxLev:009,RxDbm:-101
MCC:460,MNC:  0,LAC:09339,Cell ID:04362,BSIC:030,(P)BCCH ARFCN:0072,RxLev:021,RxDbm:-89
MCC: -1,MNC:255,LAC:00000,Cell ID:65535,BSIC:255,(P)BCCH ARFCN:0000,RxLev:000,RxDbm:-110
MCC: -1,MNC:255,LAC:00000,Cell ID:65535,BSIC:255,(P)BCCH ARFCN:0000,RxLev:000,RxDbm:-110
MCC: -1,MNC:255,LAC:00000,Cell ID:65535,BSIC:255,(P)BCCH ARFCN:0000,RxLev:000,RxDbm:-110
*/
void getCellInfo(const char* resp, CellInfo_t *pCellInfo, UINT8 count)
{
	char *p;
	char *pEnd;
	//UINT8 i;
	
	if(NULL == resp)
		return;
	p = resp;
	pEnd = p+(strlen(resp)-1);
	
	p = strstr(p,"MCC:");
	if(p==NULL)
		return;
	p=p+4;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].iMcc = atoi(p);

	p = strstr(p,"MNC:");
	if(p==NULL)
		return;
	p=p+4;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].iMnc = atoi(p);

	p = strstr(p,"LAC:");
	if(p==NULL)
		return;
	p=p+4;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].iLac= atoi(p);
	
	p = strstr(p,"Cell ID:");
	if(p==NULL)
		return;
	p=p+8;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].iCellID = atoi(p);

	p = strstr(p,"BSIC:");
	if(p==NULL)
		return;
	p=p+5;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].nBsic = atoi(p);

	p = strstr(p,"BCCH ARFCN:");
	if(p==NULL)
		return;
	p=p+11;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].iArfcn = atoi(p);

	p = strstr(p,"RxLev:");
	if(p==NULL)
		return;
	p=p+6;
	while(*p==' ')
		p++;
	if(p>=pEnd)
		return;
	pCellInfo[count].nRxLev = atoi(p);
}

static int cellinfo_proc(command_t *cmd, const char* resp)
{
	static UINT8 count = 0;

	if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 1;
	}
	else if(strstr(resp,"MCC")){
		getCellInfo(resp, (CellInfo_t *)cmd->exargs,count);
		count++;
		if(count>=7)
		{
			count = 0;
			return 0;
		}
		else
			return 1;
	}
	return -1;
}


int AT_GetCellInfo(CellInfo_t *pCellInfo)
{
	void *rv;

	rv = send_command_wait("AT+MCELL=0,21\r", (void*)pCellInfo, cellinfo_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}


/** 
 *AT_RestorePara  --- restore  factory settings
 *@return            0 success, -1 fail
 */
int AT_RestorePara(void)
{
	void	*rv;

	rv = send_command_wait("AT&F\r", NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

/** 
 *AT_SetErrorNotify  --- set error type
 *@return            0 success, -1 fail
 */
int AT_SetErrorNotify(int status)
{
	void	*rv;
	char	strcmd[16];

	sprintf(strcmd, "AT+CMEE=%d\r", status);
	rv = send_command_wait(strcmd, NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		return -1;
	}
	return 0;
}


/** +creg handler
 *@param cmd    command struct
 *@param resp   responce
 *@return    0 --complete; 1 --need proccess; -1 --not for us
 */
/* +CMEE:0 */
static int creg_proc(command_t *cmd, const char* resp)
{
	int		tmp;

	if (strequsub("+CREG:", resp)) {
		sget_response(resp, "dd", &tmp, (int*)cmd->exargs);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}
/** AT_GetRegister --- get network status
 *@return            0 success, -1 fail
 */
int AT_GetRegister(void)
{
	int		id;
	void	*rv;

	rv = send_command_wait("AT+CREG?\r", &id, creg_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		return -1;
	}
	return id;
}


/** +CCLK handler
 *@param cmd    command struct
 *@param resp   responce
 *@return    0 --complete; 1 --need proccess; -1 --not for us
 */
/* +CCLK:"99/12/31,23:59:59,(-47-+48)" */
extern int getmsg_date(char *p, int *date, int *time);

static int cclk_proc(command_t *cmd, const char* resp)
{
	if (strequsub("+CCLK:", resp)) {
		//module_date_t *date = (module_date_t*)cmd->exargs;
		//getmsg_date((char*)(resp+7), &date->date, &date->time);
		return 1;
	}
	else if (strequ("OK", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

/** 
 *AT_GetModuleTime  ---get  moderm time
 *@return            0 success, -1 fail
 */
int AT_GetModuleTime(module_date_t *time)
{
	void	*rv;

	rv = send_command_wait("AT+CCLK?\r", time, cclk_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		memset(time, 0, sizeof(module_date_t));
		return -1;
	}
	return 0;
}

/** 
 *AT_SetModuleTime  ---set moderm time
 *@return            0 success, -1 fail
 */
int AT_SetModuleTime(module_date_t *time)
{
	void	*rv;
	char	strcmd[32]; 

	sprintf(strcmd, "AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d\"\r",
				    (time->date>>16)%100,
					(time->date>>8)&0xFF,
					(time->date)&0xFF,
					(time->time>>16)&0xFF,
					(time->time>>8)&0xFF,
					(time->time)&0xFF);

	rv = send_command_wait(strcmd, NULL, cclk_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

int AT_SwitchToCmd(void)
{
	void	*rv;

	rv = send_command_wait("+++\r", NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

static int ato_proc(command_t *cmd, const char* resp)
{
	if (strequsub("CONNECT", resp)) {
		cmd->return_value = RETURN_OK;
		return 0;
	}
	return -1;
}

int AT_SwitchToData(void)
{
	void	*rv;

	rv = send_command_wait("ATO\r", NULL, ato_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

int AT_SetEcho(int id)
{
	void	*rv;
	char	strcmd[16];

	sprintf(strcmd, "ATE%d\r", id);
	rv = send_command_wait(strcmd, NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

int AT_SetSleep(int id)
{
#ifndef	MOTO_APP
	void	*rv;
	char	strcmd[16];

	sprintf(strcmd, "AT%%SLEEP=%d\r", id);
	rv = send_command_wait(strcmd, NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
#endif
	return 0;
}

int AT_SetAutoGPRSReg(void)
{
	int	status;
	void	*rv;

	rv = send_command_wait("AT+CGREG=1\r", &status, cgreg_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

int AT_SetAutoGSMReg(void)
{
	int	status;
	void	*rv;

	rv = send_command_wait("AT+CREG=1\r", &status, creg_proc, TIMEOUT_UNIT);
	if (rv == NULL)
		return -1;
	return 0;
}

/** 
 *AT_InitSet  --- init 
 *@return            0 success, other fail
 */
int AT_InitSet(void)
{
	void	*rv;
	int		status, res;

	res = 0;

	status = AT_SetEcho(0);
	if (status < 0) {
		output_at_error("+ATE0:");
		res = -1;
	}

	status = AT_SetErrorNotify(2);
	if (status < 0) {
		output_at_error("+CMEE:");
		res = -2;
	}

	rv = send_command_wait("AT+CRC=0\r", NULL, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		output_at_error("+CRC:");
		res = -3;
	}
	
	rv = send_command_wait("AT+CREG=1\r", &status, creg_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		output_at_error("+CREG:");
		res = -4;
	}

	rv = send_command_wait("AT+CLIP=1\r", &status, wait_ok_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		output_at_error("+CREG:");
		res = -6;
	}

	rv = send_command_wait("AT+CGREG=1\r", &status, cgreg_proc, TIMEOUT_UNIT);
	if (rv == NULL) {
		output_at_error("+CGREG:");
		res = -5;
	}

	if (status < 0) {
		output_at_error("+CSCS:");
		res = -6;
	}
	return res;
}


/** 
 *GPRS_InitSet  --- init moderm 
 *@return            0 success, other fail
 */
//extern char *PARAM_GetISP(void);		/* sys_param.h */ 
int GPRS_InitSet(void)
{
	int		status, res;
	
	res = 0;

	status = AT_SetEcho(0);
	if (status < 0) {
		output_at_error("+ATE0:");
		res = -1;
	}
	status = AT_SetSleep(0);
	status = AT_SetErrorNotify(2);
	if (status < 0) {
		output_at_error("+CMEE:");
		res = -2;
	}

#if 0
	status = AT_SetGPRSLink(1, 1, PARAM_GetISP());
	if (status < 0) {
		output_at_error("+CGDCONT:");
		res = -3;
	}
#endif

	status = AT_GPRSActive(1, 0);
	if (status < 0) {
		output_at_error("+CGACT:");
		res = -5;
	}
	
	return res;
}

