#include "sys_callback.h"
#include "sys_services.h"
#include "sys_ext.h"
#include "string.h"
#include "cmd.h"
#include "app.h"
#include "debug.h"
#include "cmddef.h"
#include "manage.h"

//git test02
//bb
//
static UINT8 app_init(GAPP_TASK_T **tl);
void  uart_in(INT32 uid,UINT8 *data,UINT16 len);
void  at_in(UINT8 *rsp,UINT16 rsplen);
void  sig_in(GAPP_SIGNAL_ID_T sig,va_list arg);
void  at_handle(INT8 *data,UINT16 data_len);
UINT32 SecondCnt = 0;
UINT8 GprsAttched = 0;
UINT8 GprsActived = 0;
INT32 DataRcvSocket = -2;
int32 mqtt_socketid = -1;


SYS_CALLBACK_T sys_callback = 
{
	app_init,/*init function*/
	uart_in,/*uart input callback function*/
	at_in,/*at rsp callback function*/
	sig_in,/*system signal callback function*/
};

UINT32 TASK_ONE,TASK_TWO,TASK_THREE,TASK_FOUR,TASK_FIVE;

void  uart_in(INT32 uid,UINT8 *data,UINT16 len)
{
	UINT8 *d = (UINT8*)sys_malloc(len);

	if(d)
	{
		memcpy(d,data,len);
		/*send uart data to TASK 1 */
		if(GAPP_RET_OK != sys_taskSend(TASK_ONE,T1_UART_DATA_RECV_IND,(UINT32)uid,(UINT32)d,(UINT32)len))
		{
		    sys_free(d);
		}
	}
}


void  at_in(UINT8 *rsp,UINT16 rsplen)
{
	UINT8 *d = (UINT8*)sys_malloc(rsplen+1);
	if(d)
	{
		memcpy(d,rsp,rsplen);
		d[rsplen] = '\0';
		/*send data to TASK 2 , AT command respond*/
		if(GAPP_RET_OK != sys_taskSend(TASK_TWO,T2_EVENT_AT_RSP_IND,(UINT32)rsplen,(UINT32)d,0))
		{
		    sys_free(d);
		}
	}	
}

void  sig_in(GAPP_SIGNAL_ID_T sig,va_list arg)
{
    INT32 ret;
	DB(DB2,"sig: %d",sig);
	switch(sig)
	{
	    case GAPP_SIG_SOCK_CONNECT_RSP:/*tcp connect success event*/
	    {
	        UINT32 sock;
	        /*send msg to TASK 1 , we handle socket event there*/
	        sock = va_arg(arg,UINT32);
	        ret = sys_taskSend(TASK_ONE,T1_SOCK_CONNECT_RSP,sock,0,0);
	        va_end(arg);
	    }
	    break;
	    case GAPP_SIG_SOCK_ERROR_IND:/*socket error */
	    {
	        UINT32 sock;
	        sock = va_arg(arg,UINT32);
	        /*send msg to TASK 1 , we handle socket event there*/
	        ret = sys_taskSend(TASK_ONE,T1_SOCK_ERROR_IND,sock,0,0);
	        va_end(arg);	        
	    }
	    break;
	    case GAPP_SIG_SOCK_DATA_RECV_IND:/*peer data recving*/
	    {
	        UINT32 sock,len;
	        sock = va_arg(arg,UINT32);
	        len  = va_arg(arg,UINT32);
	         /*send msg to TASK 1 , we handle socket event there*/
	        ret = sys_taskSend(TASK_ONE,T1_SOCK_DATA_RECV_IND,sock,len,0);
	        va_end(arg);	        
	    }
	    break;
	    case GAPP_SIG_SOCK_CLOSE_IND:/*tcp peer close , maybe recv FIN*/
	    {
	        UINT32 sock;
	        sock = va_arg(arg,UINT32);
	         /*send msg to TASK 1 , we handle socket event there*/
	        ret = sys_taskSend(TASK_ONE,T1_SOCK_CLOSE_IND,sock,0,0);
	        va_end(arg);	    
	    }
	    break;
	    case GAPP_SIG_SOCK_CLOSE_RSP:/*close respond */
	    {
	        UINT32 sock;
	        sock = va_arg(arg,UINT32);
	         /*send msg to TASK 1 , we handle socket event there*/
	        ret = sys_taskSend(TASK_ONE,T1_SOCK_CLOSE_RSP,sock,0,0);
	        va_end(arg);	    
	    }
	    break;	    
	    default:
	    break;
	}
}

void gapp1_timer_hanlder(void *arg)
{
    DB(DB1,"!!!");
    /*send a msg to Task 3 to handle timer expire event*/
    sys_taskSend(TASK_ONE,T1_SEC_TIMER_IND,0,0,0); 
}


void app1(UINT32 id,UINT32 n1,UINT32 n2,UINT32 n3)
{
	
	DB(DB1,"[app1] id[%u] n1[%u] n2[%u] n3[%u]",id,n1,n2,n3);
	
	switch(id)
	{
		case GAPP_TASK_INITED:
		{
			TASK_ONE = n1;//save task id
			/*hook uart so we , we can get data form uart input*/
			sys_hookUart(0,1);
			TASK_ONE = n1;//save task id
			//wait_ack = sys_sem_new(0);
			/*hook uart so we , we can get data form uart input*/
			sys_hookUart(0,1);
			sys_timer_new(1000,gapp1_timer_hanlder,NULL);
		}
		break;
		case T1_UART_DATA_RECV_IND:/*uart data recv event*/
		{
			if(0 == n1)
			{
				sys_uart_output(0,(UINT8*)n2,(UINT16)n3);/*uart echo*/
				cmd_proccess((INT8*)n2,(UINT16)n3);/*proccess uart data*/
				sys_free((void*)n2);/*free memory*/
			}
		}
		break;
		case T1_SOCK_CONNECT_RSP:
		{
            cmd_sock_connect_handle(n1,n2,n3);/*connect success */		
			sys_taskSleep(3000);//try add timeout for connect to mqtt
			Cloud_MqttTryConnectProc();
			Cloud_MqttPubReady();
			Cloud_MqttSubSetup();
		}
		break;
		case T1_SOCK_ERROR_IND:
		{
		    cmd_sock_error_handle(n1,n2,n3);/*socket error , so me must close the socket*/
		}
		break;
		case T1_SOCK_DATA_RECV_IND:
		{
		    cmd_sock_data_recv_handle(n1,n2,n3);/*socket recv data */
		}
		break;
		case T1_SOCK_CLOSE_IND:
		{
		    cmd_sock_close_ind_handle(n1,n2,n3);/*peer close , recv FIN*/
		}
		break;
		case T1_SOCK_CLOSE_RSP:
		{
		    cmd_sock_close_rsp_handle(n1,n2,n3);/*close resp*/
		}
		break;
		case T1_SEC_TIMER_IND:
		{
			SecondCnt++;
			GAgent_DevTick();
			//GAgent_Tick( pgContextData );
			//GAgent_Local_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );
            //GAgent_Cloud_Handle( pgContextData, pgContextData->rtinfo.Rxbuf, GAGENT_BUF_LEN );
			sys_timer_new(1000,gapp1_timer_hanlder,NULL);
		}
		break;
		default:
		{
		}
		break;
	}
}

void app2(UINT32 id,UINT32 n1,UINT32 n2,UINT32 n3)
{
	DB(DB1,"[app2] id[%u] n1[%u] n2[%u] n3[%u]",id,n1,n2,n3);
	
	switch(id)
	{
		case GAPP_TASK_INITED:
		{
			TASK_TWO = n1;//save task id
		}
		break;
		case T2_EVENT_AT_RSP_IND:
		{
		    DB(DB1,"at rsp : %s",(INT8*)n2);/*print string*/
            at_handle((INT8*)n2,(UINT16)n1);/*handle at respond*/
            sys_free((void*)n2);
		}
		break;
		default:
		{
		}
		break;
	}	
}

void gapp3_timer_hanlder(void *arg)
{
    DB(DB1,"!!!");
    /*send a msg to Task 3 to handle timer expire event*/
    sys_taskSend(TASK_THREE,T3_CSQ_TIMER_IND,0,0,0); 
}

void app3(UINT32 id,UINT32 n1,UINT32 n2,UINT32 n3)
{
	DB(DB1,"[app3] id[%u] n1[%u] n2[%u] n3[%u]",id,n1,n2,n3);
	
	switch(id)
	{
		case GAPP_TASK_INITED:
		{
			TASK_THREE = n1;//save task id
            init_sending_command(NULL);/*init*/
            GPRS_InitSet();/*configure */
            /*start a 10 sec timer*/
			sys_timer_new(10000,gapp3_timer_hanlder,NULL);
		}
		break;
		case T3_CSQ_TIMER_IND:
		{
		    DB(DB1,"csq !!!");
		    int csq = AT_GetSignal();
		    DB(DB3,"csq %d",csq);
		    sys_timer_new(10000,gapp3_timer_hanlder,NULL);
		}
		break;
		default:
		{
		}
		break;
	}	
}

void app4(UINT32 id,UINT32 n1,UINT32 n2,UINT32 n3)
{
	DB(DB1,"[app4] id[%u] n1[%u] n2[%u] n3[%u]",id,n1,n2,n3);
	
	switch(id)
	{
		case GAPP_TASK_INITED:
		{
			TASK_FOUR = n1;//save task id
		}
		break;
		case 3:
		{

		}
		break;
		default:
		{
		}
		break;
	}	
}

void app5(UINT32 id,UINT32 n1,UINT32 n2,UINT32 n3)
{
	DB(DB1,"[app5] id[%u] n1[%u] n2[%u] n3[%u]",id,n1,n2,n3);
	
	switch(id)
	{
		case GAPP_TASK_INITED:
		{
			TASK_FIVE = n1;//save task id

		}
		break;
		case 3:
		{

		}
		break;
		default:
		{
		}
		break;
	}	

}


GAPP_TASK_T app_task[] =
{
	{
		"cmd task",/*task name*/
		GAPP_THREAD_PRIO_0,/*priority*/
		4096,/*stack size*/
		app1/*msg handle function*/
	},
	{
		"at task",/*task name*/
		GAPP_THREAD_PRIO_1,/*priority*/
		4096,/*stack size*/
		app2/*msg handle function*/
	},
	{
		"net task",/*task name*/
		GAPP_THREAD_PRIO_2,/*priority*/
		4096,/*stack size*/
		app3/*msg handle function*/
	},
	{
		"app4",/*task name*/
		GAPP_THREAD_PRIO_3,/*priority*/
		4096,/*stack size*/
		app4/*msg handle function*/
	},
	{
		"app5",/*task name*/
		GAPP_THREAD_PRIO_4,/*priority*/
		4096,/*stack size*/
		app5/*msg handle function*/
	}
};

static UINT8 app_init(GAPP_TASK_T **tl)
{
	*tl = app_task;/*so we can got the task list at the modem side*/
	return sizeof(app_task)/sizeof(GAPP_TASK_T);/*must return task list size*/
}


