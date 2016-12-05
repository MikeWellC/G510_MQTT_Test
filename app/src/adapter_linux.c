#include "gagent.h"
#include "platform.h"
//#include "http.h"
/*huihonglei modify*/
#include "sys_services.h"
#include "debug.h"
#include "manage.h"
#include "sys_ext.h"
//#include "cloud.h"
//#include "netevent.h"
#include "gprs.h" //add by liangzt
#include "MQTTPacket.h"


extern UINT32 SecondCnt;
extern UINT8 GprsAttched;
extern UINT8 GprsActived;
extern int32 mqtt_socketid;


uint32 GAgent_GetDevTime_MS()
{
//huihonglei modify
    UINT32 tick;
/*
    int32           ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return (s*1000)+ms;
    */

//huihonglei modify
#if 0
    struct timeval tv;
         
    gettimeofday(&tv, NULL);
    
    return (uint32)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000) ;
#else
    tick = sys_getSysTick();
    return (tick*1000)/16384;
#endif
}
uint32 GAgent_GetDevTime_S()
{
//huihonglei modify
        UINT32 tick;
#if 0
    struct timeval tv;
         
    gettimeofday(&tv, NULL);
    return (uint32)(tv.tv_sec) ;
#else
    tick = sys_getSysTick();
    return tick/16384;
#endif
}
/****************************************************************
FunctionName    :   GAgent_DevReset
Description     :   dev exit but not clean the config data                   
pgc             :   global staruc 
return          :   NULL
Add by Alex.lin     --2015-04-18
****************************************************************/
void GAgent_DevReset()
{
    GAgent_Printf( GAGENT_CRITICAL,"Please restart GAgent !!!\r\n");
//huihonglei modify
    //exit(0);
    sys_softReset();
}
void GAgent_DevInit( pgcontext pgc )
{
//huihonglei modify
    //signal(SIGPIPE, SIG_IGN);
}
int8 GAgent_DevGetMacAddress( uint8* szmac )
{
//huihonglei modify
#if 1
    int ret;
    ret = AT_GetSerialNumber(szmac);
    DB(DB2,"mac ret=%d",ret);
    return 0;
#else
    int sock_mac;
    struct ifreq ifr_mac;
    uint8 mac[6]={0};
    
    if(szmac == NULL)
    {
        return -1;
    }
    
    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_mac == -1)
    {
        perror("create socket falise...mac/n");
        return (-1); 
    }
    memset(&ifr_mac,0,sizeof(ifr_mac));
    
    /*get mac address*/

    strncpy(ifr_mac.ifr_name, NET_ADAPTHER , sizeof(ifr_mac.ifr_name)-1);
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
        perror("mac ioctl error:");
        return (-1);
    }

    mac[0] = ifr_mac.ifr_hwaddr.sa_data[0];
    mac[1] = ifr_mac.ifr_hwaddr.sa_data[1];
    mac[2] = ifr_mac.ifr_hwaddr.sa_data[2];
    mac[3] = ifr_mac.ifr_hwaddr.sa_data[3];
    mac[4] = ifr_mac.ifr_hwaddr.sa_data[4];
    mac[5] = ifr_mac.ifr_hwaddr.sa_data[5];
    
    /*
    mac[0] = 0x00;
    mac[1] = 0x01;
    mac[2] = 0x02;
    mac[3] = 0x03;
    mac[4] = 0x04;
    mac[5] = 0x05;
    */
    sprintf((char *)szmac,"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    close( sock_mac );
    return 0;
#endif
}
/****************************************************************
*   function    :   socket connect 
*   flag        :   1 block 
                    0 no block 
    return      :   0> connect ok socketid 
                :   other fail.
    add by Alex.lin  --2015-05-13
****************************************************************/
int32 GAgent_connect( int32 iSocketId, uint16 port,
                        int8 *ServerIpAddr,int8 flag)
{
    int8 ret=0;
//huihonglei modify
#if 1
    GAPP_TCPIP_ADDR_T Msocket_address;
    GAgent_Printf(GAGENT_INFO,"do connect ip:%s port=%d",ServerIpAddr,port );
    ret = sys_getHostByName(ServerIpAddr,&Msocket_address.sin_addr);
    Msocket_address.sin_port = htons(port);
    ret = sys_sock_connect(iSocketId,&Msocket_address);
#else
    struct sockaddr_in Msocket_address;
    GAgent_Printf(GAGENT_INFO,"do connect ip:%s port=%d",ServerIpAddr,port );

    Msocket_address.sin_family = AF_INET;
    Msocket_address.sin_port= htons(port);
    Msocket_address.sin_addr.s_addr = inet_addr(ServerIpAddr);
/*
    unsigned long ul = 1;
    ioctl(iSocketId, FIONBIO, &ul); //设置为非阻塞模式
*/
    ret = connect(iSocketId, (struct sockaddr *)&Msocket_address, sizeof( struct sockaddr_in));  
#endif
/*  
    if( 0==ret )
    {
        GAgent_Printf( GAGENT_INFO,"immediately connect ok !");
    }
    else
    {
        if( errno == EINPROGRESS )
        {
            int times = 0;
            fd_set rfds, wfds;
            struct timeval tv;
            int flags;
            tv.tv_sec = 10;   
            tv.tv_usec = 0;                 
            FD_ZERO(&rfds);  
            FD_ZERO(&wfds);  
            FD_SET(iSocketId, &rfds);  
            FD_SET(iSocketId, &wfds);    
            int selres = select(iSocketId + 1, &rfds, &wfds, NULL, &tv);
                switch( selres )
                {
                    case -1:
                        ret=-1;
                        break;
                    case 0:
                        ret = -1;
                        break;
                    default:
                        if (FD_ISSET(iSocketId, &rfds) || FD_ISSET(iSocketId, &wfds))
                        {
                            connect(iSocketId, (struct sockaddr *)&Msocket_address, sizeof( struct sockaddr_in));   
                            int err = errno;  
                            if  (err == EISCONN)  
                            {  
                                GAgent_Printf( GAGENT_INFO,"1 connect finished .\n");  
                                ret = 0;  
                            }
                            else
                            {
                                ret=-1;
                            } 
                            char buff[2];  
                            if (read(iSocketId, buff, 0) < 0)  
                            {  
                                GAgent_Printf( GAGENT_INFO,"connect failed. errno = %d\n", errno);  
                                ret = errno;  
                            }  
                            else  
                            {  
                                GAgent_Printf( GAGENT_INFO,"2 connect finished.\n");  
                                ret = 0;  
                            }  
                        }
                }
        }
    }
    //
    ioctl(iSocketId, FIONBIO, &ul); //设置为阻塞模式
    //
    */
    if( ret==0)
        return iSocketId;
    else 
    return  -1;
}
int32 GAgent_OpenUart( int32 BaudRate,int8 number,int8 parity,int8 stopBits,int8 flowControl )
{
//huihonglei modify
#if 1
    int32 uart_fd = -1;
    GAPP_OPT_UART_BAUDRATE_T baud_set;
    uart_fd = sys_hookUart(1,1);
    if( uart_fd<0 )
        return -1;

    baud_set.uart_id = 1;
    baud_set.baud = (UINT32)BaudRate;
    if(0 > sys_set(GAPP_OPT_UART_BAUDRATE_ID,&baud_set,sizeof(baud_set)))
        return -1;
    return 1;
#else
    int32 uart_fd=0;
    uart_fd = serial_open( UART_NAME,BaudRate,number,'N',stopBits );
    if( uart_fd<=0 )
        return (-1);
    return uart_fd;
#endif
}
void GAgent_LocalDataIOInit( pgcontext pgc )
{
	//liangzt mod 
    pgc->rtinfo.local.uart_fd = GAgent_OpenUart( 9600,8,0,0,0 );
    while( pgc->rtinfo.local.uart_fd <=0 )
    {
        DB(DB2,"open uart2 error");
		pgc->rtinfo.local.uart_fd = GAgent_OpenUart(9600, 8, 0, 0, 0);
//huihonglei modify
#if 1
        sys_taskSleep(1);
#else
        sleep(1);
#endif
    }
    DB(DB2,"open uart2 ok");
    //serial_write( pgc->rtinfo.local.uart_fd,"GAgent Start !!!",strlen("GAgent Start !!!") );
    return ;
}


int g_MCC = 0;
int g_MNC = 0;
INT8 g_APN[64];

INT32 GAgent_GetAPNFromCarrier()
{
#if 1

	CellInfo_t f_cellinfo[7];
	memset(f_cellinfo,0,sizeof(CellInfo_t)*7);
	if(0 != AT_GetCellInfo(f_cellinfo)){
		GAgent_Printf(GAGENT_ERROR, "GAgent_GetAPNFromCarrier:Get cellinfo error,apn set to cmnet.");
		sprintf(g_APN, "cmnet");
		return RET_FAILED;
	}else{
		g_MCC = f_cellinfo[0].iMcc;
		g_MNC = f_cellinfo[0].iMnc;
		GAgent_Printf(GAGENT_INFO, "GAgent_GetAPNFromCarrier:Get Carrier: %d %d", g_MCC, g_MNC);
		//GAgent_Printf(GAGENT_INFO, "Get Carrier: %d %d", f_cellinfo[0].iMcc, f_cellinfo[0].iMnc);
	}
	sprintf(g_APN, "cmnet");

	if(g_MCC == 460)
	{
		switch(g_MNC){
			case 0:
			case 2:
			case 7:
				sprintf(g_APN, "cmnet");
				break;
			case 01:
				sprintf(g_APN, "3gnet");
				break;
			case 06:
				sprintf(g_APN, "UNIM2M.GZM2MAPN");    //UNIM2M.GZM2MAPN
				break;
			default:
				GAgent_Printf(GAGENT_INFO, "GAgent_GetAPNFromCarrier: 460,but APN not in list ,pls add APN");
				return RET_FAILED;
		}
	}
	else{
		GAgent_Printf(GAGENT_INFO, "GAgent_GetAPNFromCarrier: APN not in list ,pls add APN");
		return RET_FAILED;
		}
#endif
	GAgent_Printf(GAGENT_INFO, "GAgent_GetAPNFromCarrier: Sent APN = %s", g_APN);
	return RET_SUCCESS;

}


/****************************************************************
        Function        :   Cloud_InitSocket
        Description     :   init socket connect to server.
        iSocketId       :   socketid.
        p_szServerIPAddr:   server ip address like "192.168.1.1"
        port            :   server socket port.
        flag            :   =0 init socket in no block.
                            =1 init socket in block.
        return          :   >0 the cloud socket id 
                            <=0 fail.
****************************************************************/
int32 Cloud_InitSocket( int32 iSocketId,int8 *p_szServerIPAddr,int32 port,int8 flag )
{
    int32 ret=0;
    int32 tempSocketId=0;
    ret = strlen( p_szServerIPAddr );
    
    //if( ret<=0 || ret> 17 )
     //   return RET_FAILED;

    GAgent_Printf(GAGENT_DEBUG,"socket connect cloud ip:%s .",p_szServerIPAddr);
//huihonglei modify
    //if( iSocketId > 0 )
    if( iSocketId >= 0 )
    {
        GAgent_Printf(GAGENT_DEBUG, "Cloud socket need to close SocketID:[%d]", iSocketId );
        //huihonglei modify
        //close( iSocketId );
        sys_sock_close(iSocketId);
        iSocketId = INVALID_SOCKET;
        sys_taskSleep(500);
    }

//huihonglei modify
    //if( (iSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<=0)
    if((iSocketId = sys_sock_create(GAPP_IPPROTO_TCP))<0)
    {
        GAgent_Printf(GAGENT_ERROR," Cloud socket init fail");
        return RET_FAILED;
    }
    if(iSocketId>=3)
    {
        sys_softReset();
    }
    tempSocketId = iSocketId;
    GAgent_Printf(GAGENT_DEBUG, "New cloud socketID [%d]",iSocketId);
//huihonglei modify
    //iSocketId = GAgent_connect( iSocketId, port, p_szServerIPAddr,flag );
    ret = GAgent_connect( iSocketId, port, p_szServerIPAddr,flag );

    if ( ret <0 )
    {
    //huihonglei modify
        //close( tempSocketId );
        sys_sock_close(tempSocketId);
        iSocketId=INVALID_SOCKET;
        GAgent_Printf(GAGENT_ERROR, "Cloud socket connect fail with:%d", ret);
        return -3;
    }
    return iSocketId;
}


UINT8 Cloud_MqttTryConnect(UINT8 *name,UINT8 *password, int aliveint)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	char buf[200];
	int buflen = sizeof(buf);
	int len = 0;

	DB(3,"Connect to MQTT server.");

	data.clientID.cstring = "me";
	data.keepAliveInterval = aliveint;
	data.cleansession = 1;
	data.username.cstring = name;//"509fb4ebef664a2197b7685394598c7e/ledctrl";
	data.password.cstring = password;//"WHyl6AaWaKKIHuSQGrAD3C9mTZoFm1AwIZOklWW9DsI=";
	data.MQTTVersion = 4;

	len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);

	rc = sys_sock_send(mqtt_socketid, buf, len);
	if (rc == len)
		DB(3,"Successfully connect to MQTT server,len=%d\n",len);
	else
		DB(3,"Connection failed,%d,%d\n",rc,len);
	//transport_close(mysock);
	return rc;
	//return 0;
}


UINT8 Cloud_MqttPubProc(UINT8 *topic,UINT8 *msg_payload, int ssl_f)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	char buf[200];
	int buflen = sizeof(buf);
	MQTTString topicString = MQTTString_initializer;
	char *payload; //"mypayload_test_test";
	int payloadlen = 0;//strlen(payload);
	int len = 0;
	//char *host = "m2m.eclipse.org";
	//int port = 1883;
	int i=0;
	//char prtBuf[768];

	DB(3,"Sending to hostname");

	data.clientID.cstring = "me";
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	data.username.cstring = "509fb4ebef664a2197b7685394598c7e/ledctrl";
	data.password.cstring = "WHyl6AaWaKKIHuSQGrAD3C9mTZoFm1AwIZOklWW9DsI=";
	data.MQTTVersion = 4;

	payload = msg_payload;
	payloadlen = strlen(payload);
	topicString.cstring = topic; // "topic01";
	//sprintf(topicString.cstring ,"%s",topic);

	//len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);

	len = MQTTSerialize_publish((unsigned char *)(buf + len), buflen - len, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);

	//len += MQTTSerialize_disconnect((unsigned char *)(buf + len), buflen - len);
	//DB(3,"sock_id=%d,len=%d",mqtt_socketid,len);
	/*
    for(i=0;i<len;i++)
    {
        sprintf(prtBuf+i*2,"%02x",buf[i]);
    }
    DB(DB3,"rcvdata=%s",prtBuf);
	*/
	rc = sys_sock_send(mqtt_socketid, buf, len);
	if (rc == len)
		DB(3,"Successfully published,pub_len=%d\n",len);
	else
		DB(3,"Publish failed,%d,%d\n",rc,len);
	//transport_close(mysock);
	return rc;
	//return 0;
}

int socket_getdata(unsigned char* buf, int count)
{
	int rc = sys_sock_recv(mqtt_socketid, buf, count);
	//printf("received %d bytes count %d\n", rc, (int)count);
	return rc;
}


void Cloud_MqttSubSetup()
	{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	int mysock = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	int req_qos = 0;
	char* payload = "mypayload";
	int payloadlen = strlen(payload);
	int len = 0;
	//MQTTTransport mytransport;


	//mytransport.sck = &mysock;
	//mytransport.getfn = transport_getdatanb;
	//mytransport.state = 0;

	DB(DB3,"Subcribing to topic01\n");
	//sys_taskSleep(2);

	/* subscribe */
	topicString.cstring = "topic01";
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	rc = sys_sock_send(mqtt_socketid, buf, len);
	//rc = transport_sendPacketBuffer(mysock, buf, len);


}


void Cloud_MqttPubReady()
{
	UINT8 rc=-1;
	UINT8 topic[32];
	UINT8 msg_pub[512];
	sprintf(topic,"topic01");
	sprintf(msg_pub,"Ready to Work!");
	Cloud_MqttPubProc(topic,msg_pub, 0);
	//Cloud_MqttPubProc("topic01","Ready to Work!", 0);
}
void Cloud_MqttTryConnectProc()
{
	UINT8 rc=-1;
	UINT8 name[256];
	UINT8 password[256];
	sprintf(name,"509fb4ebef664a2197b7685394598c7e/ledctrl");
	sprintf(password,"WHyl6AaWaKKIHuSQGrAD3C9mTZoFm1AwIZOklWW9DsI=");
	Cloud_MqttTryConnect(name,password, 60);
	//Cloud_MqttPubProc("topic01","Ready to Work!", 0);
}



void GAgent_DevTick()
{
//huihonglei modify
    int GprsStatus = 0;
    UINT32 ip = 0;
    INT32 ret = 0;
    UINT8* p;
    static UINT8 attchErrCnt = 0;
	int isck_id=-1;
    //fflush(stdout);
    if(SecondCnt < 10)
        return;
    if(0 == (SecondCnt%5))
    {
        GprsStatus = AT_GetGRegister();
	if(1 == GprsStatus)
	{
		GprsAttched = 1;
		attchErrCnt = 0;
	}
	else
	{
		GprsAttched = 0;
		attchErrCnt++;
		if(attchErrCnt > 24)
		{
		    sys_softReset();
		}
	}
    }

    if(1 == GprsAttched)
    {
        if(0 == GprsActived)
        {
            ip = 0;
			if(RET_FAILED == GAgent_GetAPNFromCarrier())
			{
				DB(DB2,"Get APN error!!!check network And Sim card!!");
				return;
				}
			else{
	            //ret = sys_PDPActive("CMNET",NULL,NULL,&ip);
				ret = sys_PDPActive(g_APN,NULL,NULL,&ip);
	            p = (UINT8 *)&ip;
	        
	            DB(DB2,"ret %d %d.%d.%d.%d",ret,p[0],p[1],p[2],p[3]);
			    if((0 == ret)&&(0 != ip))
			    {
			    	GprsActived = 1;
					sys_taskSleep(2000);
					DB(3,"trying to connect to mqtt cloud");
					mqtt_socketid=Cloud_InitSocket(isck_id,"509fb4ebef664a2197b7685394598c7e.mqtt.iot.gz.baidubce.com",1883,0);
					//mqtt_socketid=Cloud_InitSocket(isck_id,"szgps.xicp.net",9010,0);

			    }
			}
        }
    }
}
void GAgent_DevLED_Red( uint8 onoff )
{

}
void GAgent_DevLED_Green( uint8 onoff )
{

}
