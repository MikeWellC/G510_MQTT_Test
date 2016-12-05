#include "cmd.h"
#include "stdio.h"
#include "sys_services.h"
#include "string.h"
#include "ctype.h"
#include "debug.h"
#include "app.h"
#include "sys_ext.h"
#define CMD_MAX_LENGTH 1024

typedef enum
{
	CMD_STATE_START,
	CMD_STATE_RECV_A,
	CMD_STATE_RECV_T,
	CMD_STATE_RECV_EXT
}CMD_STATE_T;

INT32 cmd_list(const INT8 *str);
INT32 cmd_sendmsg(const INT8 *str);
void  call_cmd(const INT8 *buff,UINT16 len);
INT32 cmd_memalloc(const INT8 *str);
INT32 cmd_fileopen(const INT8 *str);
INT32 cmd_fileread(const INT8 *str);
INT32 cmd_gb2UI(const INT8 *str);
INT32 cmd_filewrite(const INT8 *str);
INT32 cmd_gpiocfg(const INT8 *str);
INT32 cmd_gpioset(const INT8 *str);
INT32 cmd_gpioget(const INT8 *str);
INT32 cmd_lpgctrl(const INT8 *str);
INT32 cmd_fileseek(const INT8 *str);
INT32 cmd_fileclose(const INT8 *str);
INT32 cmd_pinirq(const INT8 *str);
INT32 cmd_ver(const INT8 *str);
INT32 cmd_pdp(const INT8 *str);
INT32 cmd_mqttpublish(const INT8 *str);//add by trento for mqtt send
INT32 cmd_tcp(const INT8 *str);
INT32 cmd_tcpsend(const INT8 *str);
INT32 cmd_udp(const INT8 *str);
INT32 cmd_udpsend(const INT8 *str);
INT32 cmd_at(const INT8 *str);
INT32 cmd_fileclear(const INT8 *str);

INT32 cmdcmp(INT32 i,const UINT8 *b,UINT16 b_len);


CMD_T cmd_table[] = 
{
	{"AT",cmd_list},
	{"+SENDMSG",cmd_sendmsg},
	{"+MALLOC",cmd_memalloc},
	{"+FILEOPEN",cmd_fileopen},
	{"+FILEREAD",cmd_fileread},
	{"+FILEWRITE",cmd_filewrite},
	{"+FILESEEK",cmd_fileseek},
	{"+FILECLOSE",cmd_fileclose},	
	{"+GB2UI",cmd_gb2UI},
    {"+gpiocfg",cmd_gpiocfg},
    {"+gpioset",cmd_gpioset},
    {"+gpioget",cmd_gpioget},
    {"+lpgctrl",cmd_lpgctrl},
	{"+PINIRQ",cmd_pinirq},
	{"+pdp",cmd_pdp},
	{"+tcp",cmd_tcp},
    {"+tcpsend",cmd_tcpsend},
    {"+udp",cmd_udp},
    {"+udpsend",cmd_udpsend},
	{"+VER",cmd_ver}, 
	{"+at",cmd_at},
	{"+fileclear",cmd_fileclear},
	{"+mqttpub",cmd_mqttpublish}
};

/*udp*/
INT32 cmd_udp_sock = -1;

INT32 cmd_udp(const INT8 *str)
{
    GAPP_TCPIP_ADDR_T tcpip_addr;
    INT32  ret,op;
    UINT16 port;

    ret = sget_response(str,"dd",&op,&port);

    DB(DB3,"!!");
    
    if(2 == ret && 1 == op)
    {
        if(0 <= cmd_udp_sock)
        {
            return -1;
        }

        cmd_udp_sock = sys_sock_create(GAPP_IPPROTO_UDP);

        if(GAPP_RET_OK > ret)
        {
            return ret;
        }

        tcpip_addr.sin_addr.addr = 0;
        tcpip_addr.sin_port = htons(port);

        ret = sys_sock_bind(cmd_udp_sock,&tcpip_addr);

        return ret;
    }
    else if(1 == ret && 0 == op)
    {
        ret = sys_sock_close(cmd_udp_sock);
        DB(DB3,"sock close %d",ret);
    }
    else
    {
        return -1;
    }
    
    return 0;    
}

INT32 cmd_udpsend(const INT8 *str)
{
    INT32 ret;
    UINT16 port;
    GAPP_TCPIP_ADDR_T tcpip_addr;
    UINT8 remote[100] = {0};
    UINT8 data[1024] = {0};
    
    ret = sget_response(str,"100sd1024s",remote,&port,data);

    DB(DB3,"%s %d %s",remote,port,data);

    if(3 == ret && 0 <= cmd_udp_sock)
    {
        ret = sys_getHostByName(remote,&tcpip_addr.sin_addr);
        if(GAPP_RET_OK != ret)
        {
            return -1;
        }

        tcpip_addr.sin_port = htons(port);
        ret = sys_sock_send2(cmd_udp_sock,data,strlen(data),&tcpip_addr);
        DB(DB3,"send %d",ret);

        return 0;
    }

    return -1;
}


/*tcp client*/
INT32 cmd_tcp_sock = -1;

INT32 cmd_tcp(const INT8 *str)
{
    UINT8 *p;
    GAPP_TCPIP_ADDR_T tcpip_addr;
    INT32  ret,op;
    UINT16 port;
    INT8   string[100] = {0};

    ret = sget_response(str,"d100sd",&op,string,&port);

    DB(DB3,"!!");
    
    if(3 == ret && 1 == op)
    {
        if(0 <= cmd_tcp_sock)
        {
            return -1;
        }
        
        ret = sys_getHostByName(string,&tcpip_addr.sin_addr);
        p = (UINT8*)&tcpip_addr.sin_addr;

        DB(DB2,"ret %d %d.%d.%d.%d",ret,p[0],p[1],p[2],p[3]);

        if(GAPP_RET_OK != ret)
        {
            return -1;
        }

        cmd_tcp_sock = sys_sock_create(GAPP_IPPROTO_TCP);

        if(GAPP_RET_OK > ret)
        {
            return ret;
        }

        tcpip_addr.sin_port = htons(port);
        ret = sys_sock_connect(cmd_tcp_sock,&tcpip_addr);

        return ret;
    }
    else if(1 == ret && 0 == op)
    {
        ret = sys_sock_close(cmd_tcp_sock);
        DB(DB3,"sock close %d",ret);
    }
    else
    {
        return -1;
    }
    
    return 0;
}

INT32 cmd_tcpsend(const INT8 *str)
{
    INT32 ret;
    UINT8 data[1024] = {0};

    ret = sget_response(str,"1024s",data);

    DB(DB3,"%d %s",ret,data);

    if(1 == ret && 0 <= cmd_tcp_sock)
    {
        ret = sys_sock_send(cmd_tcp_sock,data,strlen(data));
        DB(DB3,"send %d",ret);

        return 0;
    }

    return -1;
}

INT32 cmd_pdp(const INT8 *str)
{
    UINT8 *p;
    UINT32 ip;
    INT32 ret;
    INT32 op;
    INT8 string[256] = {0};
    
    ret = sget_response(str,"d256s",&op,string);

    DB(DB2,"ret %d %d %s",ret,op,string);

    if(2 == ret && 1 == op)
    {
        ip = 0;
        ret = sys_PDPActive(string,NULL,NULL,&ip);
        p = (UINT8 *)&ip;
        
        DB(DB2,"ret %d %d.%d.%d.%d",ret,p[0],p[1],p[2],p[3]);

        return ret;
    }
    else if(1 == ret && 0 == op)
    {
        ret = sys_PDPRelease();
        DB(DB2,"ret %d",ret);
        return ret;
    }

    return -1;
}

extern int32 mqtt_socketid;

INT32 cmd_mqttpublish(const INT8 *str)
{
    INT32 ret;
    UINT8 message_pub[512] = {0};
	UINT8 topic[32] = {0};
	int ssl_flag=0;

    ret = sget_response(str,"32s1024sd",topic,message_pub,&ssl_flag);


    DB(DB3,"ret:%d, topic:%s, message:%s, ssl_flag:%d",ret,topic,message_pub,ssl_flag);

    if(3 == ret && 0<=mqtt_socketid)
    {
        //ret = sys_sock_send(cmd_tcp_sock,data,strlen(data));
        Cloud_MqttPubProc(topic,message_pub,ssl_flag);
        //DB(DB3,"send %d",ret);

        return 0;
    }

    return -1;
}


void irq0(void)
{
    DB(DB2,"irq !!",0);
}

void irq1(void)
{
    DB(DB2,"irq !!",0);
}

INT32 cmd_lpgctrl(const INT8 *str)
{
    INT32 ret;
    INT32 op = 0;

    ret = sget_response(str,"d",&op);

    if(1 == ret)
    {
        GAPP_OPT_LPG_CONTROL_T o;
        if(op)
        {
            o.op = TRUE;
        }
        else
        {
            o.op = FALSE;
        }

        ret = sys_set(GAPP_OPT_LPG_CONTROL_ID,&o,sizeof(o));

        DB(DB2,"ret %d",ret);

        if(!ret)
        {
            return 0;
        }
    }

    return -1;
}

INT32 cmd_ver(const INT8 *str)
{
    INT32 ret;
    GAPP_OPT_SYS_VERSION_T ver;

    ret = sys_get(GAPP_OPT_SYS_VERSION_ID,&ver,sizeof(ver));

    DB(DB2,"ret %d [%s] [0x%x]",ret,ver.sys_ver,ver.api_ver);

    return ret;
}

INT32 cmd_at(const INT8 *str)
{
    UINT16 len;
    INT32 ret;
    UINT8 data[1024] = {0};

    ret = sget_response(str,"1023s",data);

    if(1 == ret)
    {
        len = strlen(data);
        data[len++] = '\r';
        ret = sys_at_send(data,len);

        if(ret == len)
        {
            return 0;
        }

        return -1;
    }

    return -1;
}

INT32 cmd_pinirq(const INT8 *str)
{
    INT32 ret;
    UINT8 pin;
    UINT16 len;
    GAPP_OPT_PIN_CFG_T cfg;

    cfg.debounce = TRUE;
    cfg.falling  = FALSE;
    cfg.rising   = TRUE;
    cfg.level    = FALSE;
    len = sizeof(GAPP_OPT_PIN_CFG_T);
    
    ret = sget_response(str,"d",&pin);

    if(1 != ret)
    {
        return -1;
    }

    ret = -1;
    
    if(pin)
    {
        cfg.cb = irq0;
        ret = sys_set(GAPP_OPT_PIN41_IRQ_ID,&cfg,len);
    }
    else
    {
        cfg.cb = irq1;
        ret = sys_set(GAPP_OPT_PIN27_IRQ_ID,&cfg,len);
    }

    DB(DB2,"sys set ret %d",ret);

    return ret;
}

INT32 cmd_fileclose(const INT8 *str)
{
    INT32 ret;
    INT32 fd;
    
    
    ret = sget_response(str,"d",&fd);

    if(1 == ret)
    {
        ret = sys_file_close(fd);
        DB(DB2,"%d ret %d ",fd,ret); 

        if(!ret)
        {
            return 0;
        }
    }

    return -1;
}

INT32 cmd_fileclear(const INT8 *str)
{
    INT32 ret;

    ret = sys_file_clear();

    DB(DB2,"%d",ret);

    return 0;
}

INT32 cmd_gpiocfg(const INT8 *str)
{
    INT32 ret;
    INT32 id,cfg;

    ret = sget_response(str,"dd",&id,&cfg);

    if(2 == ret)
    {
        ret = sys_gpio_cfg(id,cfg);
        DB(DB2,"sys gpio cfg %d : id %d cfg %d",ret,id,cfg);
        if(!ret)
        {
            return 0;
        }
    }

    return -1;
}

INT32 cmd_gpioset(const INT8 *str)
{
    INT32 ret;
    INT32 id,level;

    level = 0;
    id = 100;
    
    ret = sget_response(str,"dd",&id,&level);

    if(2 == ret)
    {
        ret = sys_gpio_set(id,level);
        DB(DB2,"sys gpio set %d : id %d cfg %d",ret,id,level);
        if(!ret)
        {
            return 0;
        }
    }    

    return -1;
}

INT32 cmd_gpioget(const INT8 *str)
{
    INT32 ret;
    INT32 id;
    UINT8 level;

    id = 0;
    
    ret = sget_response(str,"d",&id);

    if(1 == ret)
    {
        ret = sys_gpio_get(id,&level);
        DB(DB2,"sys gpio get %d : id %d cfg %d",ret,id,level);
        if(!ret)
        {
            return 0;
        }
    }        

    return -1;
}


INT32 cmd_gb2UI(const INT8 *str)
{
    INT32 ret;
    UINT32 gb,ui;

    gb = 0;
    ui = 0;
    
    ret = sget_response(str,"4s",&gb);

    if(1 == ret)
    {
        ret = sys_GB2UNI((UINT16)gb,(UINT16*)&ui);

        DB(DB2,"%d : %x to %x",ret,gb,ui);
        if(0 == ret)
        {
            return 0;
        }
    }

    return -1;
}



INT32 cmd_fileread(const INT8 *str)
{
    INT32 fd,ret;
    UINT8 buff[130];
    ret = sget_response(str,"d",&fd);

    if(1 == ret)
    {
        ret = sys_file_read(fd,buff,128);
        if(0 < ret)
        {
            buff[ret] = '\0';
            DB(DB2,"%s",buff);
            return 0;
        }
    }

    return -1;
}

INT32 cmd_fileseek(const INT8 *str)
{
    INT32 fd,ret,offset,from;
    UINT8 opt;

    offset = 0;
    from = 0;
    
    ret = sget_response(str,"ddd",&fd,&offset,&from);

    if(3 == ret)
    {
        switch(from)
        {
            case 0:
            {
                opt = FS_SEEK_SET;
            }
            break;
            case 1:
            {
                opt = FS_SEEK_CUR;
            }
            break;
            case 2:
            {
                opt = FS_SEEK_END;
            }
            break;
            default:
            return -1;
        }
        
        ret = sys_file_seek(fd,offset,opt);
        DB(DB2,"seek %d from %d offset %d",ret,opt,from);

        if(0 <= ret)
        {
            return 0;
        }
    }

    return -1;
}


INT32 cmd_filewrite(const INT8 *str)
{
    INT32 fd,ret,len;
    UINT8 buff[1025] = {0};
    
    ret = sget_response(str,"ds",&fd,buff);

    if(2 == ret)
    {
        len = strlen(buff);
        ret = sys_file_write(fd,buff,len);
        DB(DB2,"%d %d write %d",fd,len,ret);
        if(0 < ret)
        {
            return 0;
        }
    }

    return -1;
}

INT32 cmd_fileopen(const INT8 *str)
{
    INT32 ret;
    UINT32 opt;
    INT8  name[13];

    ret = sget_response(str,"sd",name,&opt);

    if(2 == ret)
    {
        switch(opt)
        {
            case 0:
            {
                opt = FS_O_RDONLY;
            }
            break;
            case 1:
            {
                opt = FS_O_WRONLY;
            }
            break;
            case 2:
            {
                opt = FS_O_RDWR;
            }
            break;
            case 3:
            {
                opt = FS_O_RDWR | FS_O_CREAT;
            }
            break;
            case 4:
            {
                opt = FS_O_RDWR | FS_O_CREAT | FS_O_EXCL;
            }
            break;
            case 5:
            {
                opt = FS_O_RDWR | FS_O_CREAT | FS_O_TRUNC;
            }
            break;
            case 6:
            {
                opt = FS_O_RDWR | FS_O_CREAT | FS_O_APPEND;
            }
            break;
            default:
            return -1;
        }
        
        ret = sys_file_open(name,opt);

        DB(DB2,"open %d",ret);

        if(0 <= ret)
        {
            return 0;
        } 
    }

    return -1;
}

/*list all of commands*/
INT32 cmd_list(const INT8 *str)
{
	UINT16 i;
	
	for(i = 0;i < sizeof(cmd_table)/sizeof(CMD_T);i++)
	{
		sys_uart_output(0,cmd_table[i].cmd,(UINT16)strlen(cmd_table[i].cmd));
		sys_uart_output(0,"\r\n",2);
	}

	return 0;
}

/*alloc memory function
 * AT+MALLOC=<size>,<n> 
*/
INT32 cmd_memalloc(const INT8 *str)
{
	INT32 size,n,i,ret;
	UINT8 *p;
	
	size = 0;
	n = 0;

	ret = sget_response(str,"dd",&size,&n);

    if(2 == ret)
    {
    	for(i = 0;i < n;i++)
    	{
    		p = sys_malloc(size);	
    		DB(DB2,"malloc a block %x size of %d bytes !! %s",p,size,p ? "success":"fail");
    		sys_taskSleep(10);
    	}

    	return 0;
	}
	else
	{
	    return -1;
	}
}

/*send a msg to a task
 * AT++SENDMSG=<tid>,<msg id>,<n1>,<n2>,<n3> 
*/
INT32 cmd_sendmsg(const INT8 *str)
{
	UINT32 id,n1,n2,n3;
	INT32 tid,n;
	
	n = sget_response(str,"ddddd",&tid,&id,&n1,&n2,&n3);
	
	DB(DB1,"[cmd_sendmsg] n[%d] tid[%d] id[%d] n1[%d] n2[%d] n3[%d]",n,tid,id,n1,n2,n3);

	switch(tid)
	{
		case 1:
		{
			tid = TASK_ONE;
		}
		break;
		case 2:
		{
			tid = TASK_TWO;
		}
		break;
		case 3:
		{
			tid = TASK_THREE;
		}
		break;
		case 4:
		{
			tid = TASK_FOUR;
		}
		break;
		case 5:
		{
			tid = TASK_FIVE;
		}
		break;
		default:
		return -1;
	}
	
	sys_taskSend(tid,id,n1,n2,n3);

	return 0;
}


struct 
{
	CMD_STATE_T state;
	UINT16      cmd_len;
	INT8 cmd[CMD_MAX_LENGTH];
}cmd_parser = {CMD_STATE_START,0,{0}};

/*cmd proccess
*must start with "AT"*/
void cmd_proccess(const INT8 *str,UINT16 len)
{
	UINT16 i;

	for(i = 0;i < len; i++)
	{
		switch(cmd_parser.state)
		{
			case CMD_STATE_START:
			{
				if('a' == str[i] ||
				   'A' == str[i])
				{
					cmd_parser.state = CMD_STATE_RECV_A;
				}
			}
			break;
			case CMD_STATE_RECV_A:
			{
				if('t' == str[i] ||
				   'T' == str[i])
				{
					cmd_parser.state = CMD_STATE_RECV_T;
				}
				else if('a' == str[i] ||
				        'A' == str[i])
				{
					cmd_parser.state = CMD_STATE_RECV_A;
				}
				else
				{
					memset(&cmd_parser,0,sizeof(cmd_parser));
				}
			}
			break;
			case CMD_STATE_RECV_T:
			{
				if('\r' == str[i])
				{
					call_cmd("AT",2);
					memset(&cmd_parser,0,sizeof(cmd_parser));
				}
				else
				{
					cmd_parser.state = CMD_STATE_RECV_EXT;
					cmd_parser.cmd[cmd_parser.cmd_len++] = str[i];
				}
			}
			break;
			case CMD_STATE_RECV_EXT:
			{
				if('\r' == str[i])
				{
					call_cmd(cmd_parser.cmd,cmd_parser.cmd_len);
					memset(&cmd_parser,0,sizeof(cmd_parser));
				}
				else
				{
					cmd_parser.cmd[cmd_parser.cmd_len++] = str[i];
				}
			}
		}
		
	}
}


void call_cmd(const INT8 *buff,UINT16 len)
{
    INT32 ret;
	UINT16 i;
	UINT16 cmdlen;
    UINT8 str[64];

    ret = -1;
    
	for(i = 0;i < sizeof(cmd_table)/sizeof(CMD_T);i++)
	{
	    cmdlen = strlen(cmd_table[i].cmd);
		if(0 == cmdcmp(i,(UINT8 *)buff,len))
		{
			if(cmd_table[i].cmd_handler)
			{
				sys_uart_output(0,"\r\n",2);
                
				ret = cmd_table[i].cmd_handler(&buff[cmdlen]);
				if(0 == ret)
				{
				    goto OK;
				}
				else
				{
				    goto ERROR;
				}
			}
			else
			{
				DB(DB1,"[call_cmd] command found but no handler !!");
				goto ERROR;
			}
			break;
		}
	}

	if(sizeof(cmd_table)/sizeof(CMD_T) == i)
	{
		DB(DB1,"[call_cmd] command not found !!");
		goto ERROR;
	}

	return;

OK:
	sys_uart_output(0,"\r\nOK\r\n",6);
	return;
ERROR:
    len = sprintf(str,"\r\nERROR: %ld\r\n",ret);
    
	sys_uart_output(0,str,len);
}

INT32 cmdcmp(INT32 i,const UINT8 *b,UINT16 at_len)
{
    UINT16 ati = 0;
    UINT16 cmd_len = 0;
    const UINT8 *pcmd = cmd_table[i].cmd;

    cmd_len = (UINT16)strlen(cmd_table[i].cmd);

    while(ati < at_len)
    {
        if(toupper(*pcmd) != toupper(*b))
        {
            return ati+1;
        }

        if('\0' == *pcmd && '\0' == *b)
        {
            return 0;
        }
        
        ati++;
        pcmd++;
        b++;

        if('\0' == *pcmd)
        {
            if('=' == *b)
            {
                return 0;
            }
            else if('\0' == *b)
            {
                return 0;
            }
            else
            {
                return ati+1;
            }
        }        
    }

    return ati;
}

/*socket connect success*/
void cmd_sock_connect_handle(UINT32 n1,UINT32 n2,UINT32 n3)
{
    if((INT32)n1 == cmd_tcp_sock)
    {
        DB(DB3,"socket %d connect success %d %d!!",cmd_tcp_sock,(INT32)n2,(INT32)n3);
    }
}

/*socket error , close socket*/
void cmd_sock_error_handle(UINT32 n1,UINT32 n2,UINT32 n3)
{
    if((INT32)n1 == cmd_tcp_sock)
    {
        DB(DB3,"socket %d error %d %d !!",cmd_tcp_sock,(INT32)n2,(INT32)n3);
        sys_sock_close(cmd_tcp_sock);
        cmd_tcp_sock = -1;
    }    
}

/*socket recv data*/
void cmd_sock_data_recv_handle(UINT32 n1,UINT32 n2,UINT32 n3)
{
    INT32 ret;
    UINT8 buff[1400] = {0};
	UINT8 prtBuf[768] = {0};
	int i=0;
    DB(DB3,"mqtt_socket %d read %d %d !!",mqtt_socketid,(INT32)n2,(INT32)n3);
    ret = sys_sock_recv(mqtt_socketid,buff,256);
    //DB(DB3,"mqtt_data len %d : %s",ret,buff);

	for(i=0;i<ret;i++)
    {
        sprintf(prtBuf+strlen(prtBuf),"%02x ",buff[i]);
    }
    DB(DB3,"mqtt_data len %d : %s",ret,prtBuf);
    
    if((INT32)n1 == cmd_tcp_sock)
    {
        DB(DB3,"socket %d read %d %d !!",cmd_tcp_sock,(INT32)n2,(INT32)n3);
        ret = sys_sock_recv(cmd_tcp_sock,buff,1300);
        DB(DB3,"data len %d : %s",ret,buff);
    }
    else if((INT32)n1 == cmd_udp_sock)
    {
        GAPP_TCPIP_ADDR_T addr;
        UINT8 *pip = (UINT8*)&addr.sin_addr.addr;
        ret = sys_sock_recvFrom(cmd_udp_sock,buff,1300,&addr);
        DB(DB3,"recv %d data from [%d.%d.%d.%d:%d] : %s",ret,pip[0],pip[1],pip[2],pip[3],ntohs(addr.sin_port),buff);
    }
}

/*peer send a FIN in tcp*/
void cmd_sock_close_ind_handle(UINT32 n1,UINT32 n2,UINT32 n3)
{
    INT32 ret;
    
    if((INT32)n1 == cmd_tcp_sock)
    {
        ret = sys_sock_close(cmd_tcp_sock);
        DB(DB3,"ret %d",ret);
    } 
}
/*close a socket success*/
void cmd_sock_close_rsp_handle(UINT32 n1,UINT32 n2,UINT32 n3)
{    
    if((INT32)n1 == cmd_tcp_sock)
    {
        cmd_tcp_sock = -1;
    }     
}


