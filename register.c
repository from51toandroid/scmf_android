#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "RouterSdk.h"
#include "protool.h"
#include "register.h"
#include "devsdk.h"
extern char gProtoolKey[32];

typedef struct __SENDDATA_PARM_
{
	int sock;
	U8 optdata[16];
	U8 data[1024];
	int type;
	int isack;
	int optlen;
	int len;
	U8 checks;
}SendParm;

extern volatile int runMode;
extern  PLUG_CONFIG local_cfg;
extern  PRO_PARA pro_para;

extern U8 gRoutemac[6];
extern char server1_addr[32], server2_addr[32], server_port[16];
extern char mLoginServer[16], mLoginPluginId[32], mLoginTocken[32];
extern short mLoginPort;
RegisterObj Reg1Server, Reg2Server;
volatile int LoginFlag;
//int SendRouteUpData(U8 *optdata, U8 *data, PACKAGE_TYPE type, int isack,int optlen,int len,U8 checks)
static int RegisterSendUpData(SendParm *pSendParm)
{
	PACKAGE_HEAD pk_head;
	DATA_HEAD dt_head;
	//short dt_serial =0;
	
	memset(&pk_head,0,sizeof(pk_head));
	memset(&dt_head,0,sizeof(dt_head));
	
	dt_head.optLength = pSendParm->optlen;
		
	if( (pSendParm->len ==0)&&(pSendParm->optlen==0) ) return -1;
    pSendParm->isack =( pSendParm->isack|(ENCTYPE<<1))&0x7;
	pk_head.magic = swapInt16(DX_MAGIC_HEAD);
	pk_head.headp = ( pSendParm->type | (pSendParm->isack<<5) );
	dt_head.checksum=pSendParm->checks;
	dt_head.length = swapInt16(pSendParm->len);	
	

	char senddata[1024*4];
	memcpy( senddata,   &pk_head, 3 );
	memcpy( senddata+3, &dt_head, 4 );
	if( (pSendParm->optlen)&&(pSendParm->optdata) )
		memcpy(senddata+7, pSendParm->optdata, pSendParm->optlen);
	if( (pSendParm->len)&&(pSendParm->data) )
		memcpy(senddata+7+pSendParm->optlen, pSendParm->data, pSendParm->len);
	if( pSendParm->type ==bissness_pack_up){
		//d_log(D_LOG_DEBUG, "%d\n",7+pSendParm->optlen+pSendParm->len);
        printf("########%s:%d %d\n",__func__, __LINE__,7+pSendParm->optlen+pSendParm->len);
	}

    int sendrt = SendRegData(pSendParm->sock, senddata,7+pSendParm->optlen+pSendParm->len);
	if( sendrt == -1 ){
        //d_log(D_LOG_ERR, "mTcpConnectServer->sendData error\n");
        printf("########%s:%d mTcpConnectServer->sendData error\n",__func__, __LINE__);
        return -1;
    }            
    return 0;     
}      






//dp_mcu@163.com
//youhua
//yh123456YH



static int RegistSendMsg( int sock )     
{          
    char optdata[16];
    data_regist data;
    int ret = -1;                     
    U8 checkkdata[1024];        
    SendParm parmInfo;
                
    memset( &parmInfo,  0, sizeof(SendParm) );   
    memset( checkkdata, 0, 1024 );
                                           
	if( pro_para.conf.pin_code==NULL || strlen(pro_para.conf.pin_code)==0 ){
		//d_log(D_LOG_ERR, "ERROR: PIN is empty, plese check your plug.conf!\n");
        printf("%s : %d ERROR: PIN is empty, plese check your plug.conf!\n",__func__, __LINE__);
		return -1; 
		//exit(-1);
	}

	if( gProtoolKey==NULL || strlen(gProtoolKey)==0 ){
		//d_log(D_LOG_ERR, "ERROR: KEY is empty, plese check your plug.conf!\n");
        printf("%s:%d KEY is empty, plese check your plug.conf!\n",__func__, __LINE__);
        return -1; 
		//exit(-1);
    }


    //printf("out pincode\n");
    //macdbg_dmphex(pro_para.conf.pin_code, 12);


	
	memcpy( &optdata[3], pro_para.conf.pin_code, 12 );       
	U8 mac[6] = {0};
	memset(mac, 0, sizeof(mac));
	if( 0 != memcmp(mac, local_cfg.mac, 6) ){
		memcpy(mac, local_cfg.mac, 6);
	}else{
        get_dev_mac(mac);
	}

	//d_log(D_LOG_DEBUG, "ROUTER MAC: %02x %02x %02x %02x %02x %02x\n",
	    //mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //printf("in %s SN: %s ROUTER MAC: %02x %02x %02x %02x %02x %02x.\n", __func__, 
      //  pro_para.conf.sn_code, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] ); 
    
	memcpy(gRoutemac,mac,6);
    
	//char sn[64];
	//memset(sn,0x0,64);
    //memcpy(&sn, pro_para.conf.sn_code, 64);
	if( pro_para.conf.sn_code==NULL || strlen(pro_para.conf.sn_code)==0 ){
	    get_dev_sn(pro_para.conf.sn_code);
	}
	
	//d_log(D_LOG_DEBUG, "SN: %s LEN: %d\n",sn,(int)strlen(sn));
    printf("%s:%d SN: %s LEN: %d.\n",__func__, __LINE__,pro_para.conf.sn_code,(int)strlen(pro_para.conf.sn_code));
             
	data.deviceSnLength=strlen(pro_para.conf.sn_code);
	if( data.deviceSnLength > 64 ){
        data.deviceSnLength = 64;
    }
                                   
    char decdata[1024];               
    memset(decdata,0,1024);        

    int datalen = 13 + data.deviceSnLength + 38;
	
    memcpy( &decdata[2], pro_para.conf.pin_code, 12 );        
    decdata[2+12] = data.deviceSnLength;
	
    memcpy(&decdata[15], pro_para.conf.sn_code, data.deviceSnLength);
    memcpy(&decdata[15+data.deviceSnLength],mac,6);

	/*int datalen = 39+data.deviceSnLength;
	decdata[2]=data.deviceSnLength;
	memcpy(&decdata[3],sn,data.deviceSnLength);
	memcpy(&decdata[3+data.deviceSnLength],mac,6);*/

	int dlen;
	char miwen_hex[1024];
    //printf("%s:%d aes_crypt_ecb encode: ----datalen %d---.\n",__func__, __LINE__,datalen+2);            
 	ret = aes_encrypt( (char*)gProtoolKey, decdata, miwen_hex, datalen+2, &dlen );                        
	if(ret !=0) return -1;
 	//d_log(D_LOG_DEBUG, "aes_crypt_ecb encode: ----len %d---\n",dlen);
    //printf("%s:%d aes_crypt_ecb encode: ----len %d---.\n",__func__, __LINE__,dlen);
	short dlen2 = dlen;
	dlen2 = swapInt16(dlen2);

	memcpy( checkkdata, &dlen2, 2 );
	memcpy( checkkdata+2, optdata, 15 );

	//printf("out optdata\n");
    //macdbg_dmphex(optdata, 15);

	
	memcpy(checkkdata+2+15,decdata,datalen+2);
	
	U8 checks = get_check_sum((unsigned char*)checkkdata,2+15+datalen+2);

	parmInfo.sock=sock;
	memcpy(parmInfo.optdata, optdata, 16);
	memcpy(parmInfo.data, miwen_hex, 1024);
	parmInfo.type = login_conform;
	parmInfo.isack=1;
	parmInfo.optlen=15;
	parmInfo.len=dlen;
	parmInfo.checks=checks;
	ret=RegisterSendUpData(&parmInfo);
	return ret;
	
}





static void *RegisterServer( void *args )
{                        
    int ret;         
    RegisterObj *pRegObj=(RegisterObj *)args;
         
    ret = Reg_ServerInit(pRegObj);
	if( ret != 0 ){
		//db_log(D_LOG_ERR, "Reg_ServerInit error\n");                   
		//Reg_DeleteServer(pRegObj);                
        printf("%s:%d Reg_ServerInit error...\n",__func__, __LINE__);
		return NULL;                
	}              
            
	ret = RegistSendMsg(pRegObj->sock);
	if( ret == 0 ){
		//db_log(D_LOG_DEBUG, "Send register info success.\n");                  
        printf( "%s Send register info success!\n\n", __func__ );
	}else{
		//db_log(D_LOG_ERR, "Send register info error.\n");          
        printf("%s:%d Send register info error...\n",__func__, __LINE__);
	}
	return NULL;            
}













int RegisterStart()
{
	int ret1, ret2, retry, regist_port;
	pthread_t Server1Pid, Server2Pid;

	regist_port = atoi(pro_para.conf.reg_port);
	if( regist_port<2048 || regist_port>65530 )
	{
		printf("server_port %d.\n\n",regist_port);
        printf("########%s:%d  reg_port error.\n",__func__, __LINE__);
		return -1;
	}

	memset(&Reg1Server, 0, sizeof(RegisterObj)); //init 清零
	memset(&Reg2Server, 0, sizeof(RegisterObj));
	LoginFlag=0;
	Reg1Server.port=regist_port;
	Reg2Server.port=regist_port;
	strcpy(Reg1Server.host, pro_para.conf.reg_server1);
	strcpy(Reg2Server.host, pro_para.conf.reg_server2);
	
    if( runMode == 0 ){   
		//测试模式
        Reg1Server.pRQ=(RegQueue *)calloc(1, sizeof(RegQueue));
		if( Reg1Server.pRQ == NULL ){          
            printf("########%s:%d  Malloc queue failed.\n",__func__, __LINE__);
			return -1;
		}
		
		ret1 = RegQueueInit(Reg1Server.pRQ, REG_BUFSIZE);
		if( ret1 !=0 ){
			//db_log(D_LOG_ERR, "RegQueueInit error\n");
            printf("########%s:%d  RegQueueInit error.\n",__func__, __LINE__);
			if( Reg1Server.pRQ !=NULL )
			{
				free(Reg1Server.pRQ);
				Reg1Server.pRQ=NULL;
			}
			return -1;
		}

		//LogPlugStatus(log_init);

		ret1=pthread_create(&Server1Pid, NULL, RegisterServer, &Reg1Server);
		if( ret1 !=0 )
		{
			//db_log(D_LOG_ERR,"Create RegisterServer error.\n");
            printf("########%s:%d  Create RegisterServer error..\n",__func__, __LINE__);
			RegQueueDestroy(Reg1Server.pRQ);
			if( Reg1Server.pRQ !=NULL )
			{
				free(Reg1Server.pRQ);
				Reg1Server.pRQ=NULL;
			}
			return -1;
		}

		retry=0;
		while(1)
		{
			usleep(10000);
			if( Reg1Server.RegisterOk==1 )
			{
				memcpy(pro_para.link_plugin_id, Reg1Server.mLoginPluginId, 32);
				memcpy(pro_para.link_server, Reg1Server.mLoginServer, 16);
				memcpy(pro_para.link_tocken, Reg1Server.mLoginTocken, 32);
				pro_para.link_port=Reg1Server.mLoginPort;
				break;
			}
			
			retry++;
			if( retry>1500 )  // 15s
			{
				//db_log(D_LOG_ERR, "Register timeout...\n");
                printf("########%s:%d  Register timeout...\n",__func__, __LINE__);
				break;
			}
		}

		pthread_join(Server1Pid, NULL);
		Reg_DeleteServer(&Reg1Server);
		RegQueueDestroy(Reg1Server.pRQ);
		if( Reg1Server.pRQ !=NULL )
		{
			free(Reg1Server.pRQ);
			Reg1Server.pRQ=NULL;
		}
		if( Reg1Server.RegisterOk==1)
		{
			//db_log(D_LOG_DEBUG, "Register server1 success.\n");
            printf("%s: Register vserver1 success.\n", __func__ );
			return 0;
		}
		
		//LogPlugStatus(log_init_err);
		//db_log(D_LOG_ERR, "Register failed.\n");
        printf("########%s:%d  Register failed.\n",__func__, __LINE__);
		return -1;
	}else if( runMode==1 ){
		Reg1Server.pRQ=(RegQueue *)calloc(1, sizeof(RegQueue));
		if( Reg1Server.pRQ==NULL ){
			//db_log(D_LOG_ERR, "Malloc queue failed.\n");
            printf("########%s:%d  Register failed.\n",__func__, __LINE__);
			return -1;
		}
		Reg2Server.pRQ=(RegQueue *)calloc(1, sizeof(RegQueue));
		if( Reg2Server.pRQ==NULL ){
			//db_log(D_LOG_ERR, "Malloc queue failed.\n");
            printf("########%s:%d  malloc queue failed.\n",__func__, __LINE__);
			if( Reg1Server.pRQ !=NULL ) free(Reg1Server.pRQ);
			return -1;
		}

		ret1=RegQueueInit(Reg1Server.pRQ, REG_BUFSIZE);
		if( ret1 !=0 ){
			//db_log(D_LOG_ERR, "RegQueueInit error\n");
            printf("########%s:%d  RegQueueInit error.\n",__func__, __LINE__);
			if( Reg1Server.pRQ !=NULL ){
				free(Reg1Server.pRQ);
				Reg1Server.pRQ=NULL;
			}
			if( Reg2Server.pRQ !=NULL ){
				free(Reg2Server.pRQ);
				Reg2Server.pRQ=NULL;
			}
			return -1;
		}
		ret2=RegQueueInit(Reg2Server.pRQ, REG_BUFSIZE);
		if( ret2 !=0 ){
			//db_log(D_LOG_ERR, "RegQueueInit error\n");
            printf("########%s:%d  RegQueueInit error.\n",__func__, __LINE__);
			RegQueueDestroy(Reg1Server.pRQ);
			if( Reg1Server.pRQ !=NULL ){
				free(Reg1Server.pRQ);
				Reg1Server.pRQ=NULL;
			}
			if( Reg2Server.pRQ !=NULL ){
				free(Reg2Server.pRQ);
				Reg2Server.pRQ=NULL;
			}
			return -1;
		}

		//LogPlugStatus(log_init);
		
		ret1 = pthread_create( &Server1Pid, NULL, RegisterServer, &Reg1Server );

		printf( "ret1 = %d\n", ret1 );

		//ret2=pthread_create(&Server2Pid, NULL, RegisterServer, &Reg2Server);
		//if( (ret1 !=0) && (ret2 !=0)  )
        //by dp_mcu
        //整2个线程 嫌不够复杂吗?
        
		if( ret1 != 0 ){
			//db_log(D_LOG_ERR,"Create RegisterServer error.\n");
			//线程创建失败是不大可能的事
            printf( "%s : %d  Create RrgisterServer error.\n",__func__, __LINE__);
			RegQueueDestroy(Reg1Server.pRQ);
			RegQueueDestroy(Reg2Server.pRQ);
			if( Reg1Server.pRQ !=NULL ){
				free(Reg1Server.pRQ);
				Reg1Server.pRQ=NULL;
			}
			if( Reg2Server.pRQ !=NULL ){
				free(Reg2Server.pRQ);
				Reg2Server.pRQ=NULL;
			}
			return -1;
		}

		retry = 0;
		while( 1 ){
			usleep(10000);
			if( Reg1Server.RegisterOk==1 ){
				memcpy(pro_para.link_plugin_id, Reg1Server.mLoginPluginId, 32);
				memcpy(pro_para.link_server, Reg1Server.mLoginServer, 16);
				memcpy(pro_para.link_tocken, Reg1Server.mLoginTocken, 32);
				pro_para.link_port=Reg1Server.mLoginPort;
				LoginFlag=1;
				break;
			}
			if( Reg2Server.RegisterOk==1 ){
				memcpy(pro_para.link_plugin_id, Reg2Server.mLoginPluginId, 32);
				memcpy(pro_para.link_server, Reg2Server.mLoginServer, 16);
				memcpy(pro_para.link_tocken, Reg2Server.mLoginTocken, 32);
				pro_para.link_port=Reg2Server.mLoginPort;
				LoginFlag=2;
				break;
			}
		
			retry++;
			if( retry>1500 ){
				//15s
                printf( "Register timeout\n" );
				break;
			}
		}

		pthread_join(Server1Pid, NULL);
		pthread_join(Server2Pid, NULL);
		
		Reg_DeleteServer(&Reg1Server);
		Reg_DeleteServer(&Reg2Server);
		RegQueueDestroy(Reg1Server.pRQ);
		RegQueueDestroy(Reg2Server.pRQ);
		if( Reg1Server.pRQ !=NULL )
		{
			free(Reg1Server.pRQ);
			Reg1Server.pRQ=NULL;
		}
		if( Reg2Server.pRQ !=NULL )
		{
			free(Reg2Server.pRQ);
			Reg2Server.pRQ=NULL;
		}

		if( Reg1Server.RegisterOk==1)
		{
            printf("########%s:%d  Register xxxserver1 success...\n",__func__, __LINE__);
			return 0;
		}
		else if( Reg2Server.RegisterOk==1 )
		{

            printf("########%s:%d  Register server2 success...\n",__func__, __LINE__);
			return 0;
		}
		
		//LogPlugStatus(log_init_err);
		//db_log(D_LOG_ERR, "Register failed.\n");
        printf("Register failed in mode 1.\n" );
		return -1;
	}
	

    printf("########%s:%d  Register failed,runMode error...\n",__func__, __LINE__);
	return -1;
}

int RegisterAgain(char *pServerAddr)
{
	int ret, retry, regist_port;
	pthread_t Server1Pid;

	regist_port = atoi(pro_para.conf.reg_port);
	if( regist_port<2048 || regist_port>65530 )
	{
		//d_log(D_LOG_ERR, "server_port error.\n\n");
        printf("########%s:%d  server_port error..\n",__func__, __LINE__);
		return -1;
	}


	
	Reg1Server.port=regist_port;
	strcpy(Reg1Server.host, pServerAddr);
	
/////////////////////////////////////////////////////////////////////
	Reg1Server.pRQ=(RegQueue *)calloc(1, sizeof(RegQueue));
	if( Reg1Server.pRQ==NULL )
	{
		//d_log(D_LOG_ERR, "Malloc queue failed.\n");
        printf("########%s:%d  Malloc queue failed.\n",__func__, __LINE__);
		return -1;
	}
	
	ret=RegQueueInit(Reg1Server.pRQ, REG_BUFSIZE);
	if( ret !=0 )
	{
		//d_log(D_LOG_ERR, "RegQueueInit error\n");
        printf("########%s:%d  RegQueueInit error.\n",__func__, __LINE__);
		if( Reg1Server.pRQ !=NULL )
		{
			free(Reg1Server.pRQ);
			Reg1Server.pRQ=NULL;
		}
		return -1;
	}

//	LogPlugStatus(log_init);

	ret=pthread_create(&Server1Pid, NULL, RegisterServer, &Reg1Server);
	if( ret !=0 )
	{
		//d_log(D_LOG_ERR,"Create RegisterServer error.\n");
        printf("########%s:%d  Create RegisterServer error.\n",__func__, __LINE__);
		RegQueueDestroy(Reg1Server.pRQ);
		if( Reg1Server.pRQ !=NULL )
		{
			free(Reg1Server.pRQ);
			Reg1Server.pRQ=NULL;
		}
		return -1;
	}

	retry=0;
	while(1)
	{
		usleep(10000);
		if( Reg1Server.RegisterOk==1 )
		{
			memcpy(pro_para.link_plugin_id, Reg1Server.mLoginPluginId, 32);
			memcpy(pro_para.link_server, Reg1Server.mLoginServer, 16);
			memcpy(pro_para.link_tocken, Reg1Server.mLoginTocken, 32);
			pro_para.link_port=Reg1Server.mLoginPort;
			break;
		}
		
		retry++;
		if( retry>1500 )  // 15s
		{
			//d_log(D_LOG_ERR, "Register timeout...\n");
            printf("########%s:%d  Register timeout....\n",__func__, __LINE__);
			break;
		}
	}

	pthread_join(Server1Pid, NULL);
	Reg_DeleteServer(&Reg1Server);
	RegQueueDestroy(Reg1Server.pRQ);
	if( Reg1Server.pRQ !=NULL )
	{
		free(Reg1Server.pRQ);
		Reg1Server.pRQ=NULL;
	}
	if( Reg1Server.RegisterOk==1)
	{
		//d_log(D_LOG_DEBUG, "Register again success.\n");
        printf("########%s:%d  Register again success...\n",__func__, __LINE__);
		return 0;
	}
		
	LogPlugStatus(log_init_err);
	//d_log(D_LOG_ERR, "Register failed.\n");
    printf("########%s:%d  Register failed...\n",__func__, __LINE__);

    return -1;
}

