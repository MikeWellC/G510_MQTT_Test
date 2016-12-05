/******************************************************************************
  Copyright (C), 2010, Shenzhen G&T Industrial Development Co.,Ltd

  File:        manage.h
  Author:	  shumin
  Version:  1.0
  Date:      2004.8
  
  Description:    AT command about  network
******************************************************************************/
#ifndef  GSM_MANAGE_H
#define  GSM_MANAGE_H


typedef struct module_date {
	int    date;   /**<date, 0xYYYYMMDD, YYYY --year, MM -- month, DD -- day*/
	int    time;   /**< time, 0x00HHMMSS, HH -- hour, MM -- minute, SS -- second*/
} module_date_t;


#define ID_PIN1      1
#define ID_PIN2      2

#define PIN_OK       1
#define PIN_PIN1     2
#define PIN_PIN2     3
#define PIN_PUK1     4
#define PIN_PUK2     5
#define PIN_LOCK     6
#define PIN_NETWORK  7

#define FAC_SIM_LOCK     0 /**< "PS"    SIM lock facility with a 8 digits password. */
#define FAC_PIN_STATUS   1 /**< "SC"    PIN enabled / disabled */
#define FAC_ALL_OUT      2 /**< "AO"    barring of all outgoing calls*/
#define FAC_INTERNATION  3 /**< "OI"     barring of International calls (incoming or outgoing)*/
#define FAC_OUT_ONLY_IN  4 /**< "OX"    barring of International calls (outgoing)*/
#define FAC_ALL_IN       5 /**< "AI"     barring of all incoming calls*/
#define FAC_ROAM_IN      6 /**< "IR"     No roaming calls.*/
#define FAC_ALL_CALL     7 /**< "AB"    call barring*/
#define FAC_CALL_OUT     8 /**< "AG"    outgoing call  barring*/
#define FAC_CALL_IN      9 /**< "AC"    incoming call barring*/
#define FAC_NET_LOCK    10 /**< "PN"    Network lock with a 8 digits password (NCK).  */
#define FAC_SUB_LOCK    11 /**< "PU"    Network Subset lock with a 8 digits password (NSCK). */
#define FAC_SERVICE     12 /**< "PP"    Service Provider lock with a 8 digits password (SPCK). */
#define FAC_CORPORATE   13 /**< "PC"    Corporate lock with a 8 digits password (CCK). */
#define FAC_FIXED       14 /**< "FD"    SIM Fixed Dialing Numbers (FDN) memory */

typedef struct CellInfo{
	UINT16 iMcc;
	UINT16 iMnc;
	UINT16 iLac;
	UINT16 iCellID;
	UINT8 nBsic;
	UINT16 iArfcn;
	UINT8 nRxLev;
}CellInfo_t;


int AT_InitSet(void);
int GPRS_InitSet(void);

int AT_GetCellInfo(CellInfo_t *pCellInfo);

int AT_GetManufactory(char *name);
int AT_GetSerialNumber(char *name);	
int AT_GetIMSI(char *name);

int AT_SetModuleTime(module_date_t *); 
int AT_GetModuleTime(module_date_t *);

int AT_GetSignal(void);
int AT_GetRegister(void);
int AT_GetGRegister(void);

int AT_SetBaudrate(int);	
int AT_SetComPara(int, int);
int AT_CheckPin(void);
int AT_InputPin(char*, char*);
int AT_ChangePwd(char*, char*);
int AT_RestorePara(void);
int AT_SetErrorNotify(int);
int AT_SetGPRSLink(int, int, char*);
int AT_GPRSAttach(int, int);
int AT_GPRSActive(int, int);
int AT_StartMux(void);
int AT_SetEcho(int id);	
int AT_SetAutoGPRSReg(void);
int AT_SetAutoGSMReg(void);
int AT_SetSleep(int id);

int AT_SwitchToCmd(void); 
int AT_SwitchToData(void);

int AT_CheckSIM(void);

#endif

