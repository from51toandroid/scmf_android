
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <asm/types.h>
#include <sys/mman.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>




//#include "SysType.h"
//#include "Debug.h"
#include "protool.h"
#include "register.h"
//#include "RingQueue.h"
#include "tcpserver.h"

//#define RG_BUF_SIZE 1600
////////////////////////////////////////////////////
extern char gProtoolKey[32];
extern  PRO_PARA pro_para;

//int GetServerIp(const char * hostname, char *server_ip)




u8 g_debug_buff[256];
int g_debug_buff_index;
#define macdbg_prser Ser_Printf
                                                        
//#define Ser_WrStr do{ LOGI("%s", buffer); }while(0)
int g_printf_switch = 0x01;
int Ser_Printf (const char *format, ...)
{   
    unsigned char buffer[80 + 1];
    va_list  vArgs;
    if( g_printf_switch == 0x00 ){
        return 1;
    }
    va_start(vArgs, format);
    vsnprintf((char *)buffer, sizeof(buffer), (char const *)format, vArgs);
    va_end(vArgs);
    //Ser_WrStr;
    strcpy( &g_debug_buff[g_debug_buff_index], buffer );
    g_debug_buff_index = g_debug_buff_index + strlen(buffer);
    return 0;
}


int macdbg_dmphex(const char* buff, int len)
{
    int retval = 0; 
    int x, y, tot, lineoff;
    const char* curr;
    lineoff = 0;
    curr = buff;
    tot = 0;
               
    for( x = 0; len > x+16; ){                      
         macdbg_prser("0x%08x:  ", lineoff);           
         for( y = 0; y < 16; y++ ){
              macdbg_prser("%02x ", (unsigned char)*(curr + y));
         }
         macdbg_prser("  ");
         for( y = 0; y < 16; y++ ){
              char c;
              c = *(curr + y);
              if( c > 31 && c < 127 ){
                  macdbg_prser("%c", c);
              }else{
                  macdbg_prser("%c", '.');
              }
              tot++;
         }
         curr += 16;
         x += 16;
         lineoff+=16;
	    //syslog( LOG_INFO, "%s", g_debug_buff );

		printf(  "%s\n", g_debug_buff );
         memset( &g_debug_buff[0x00], 0x00, sizeof(g_debug_buff) );
         g_debug_buff_index = 0x00;
    }
                  
    //do last line
    //Ser_Printf("tot %d.\r\n", tot );
    //Ser_Printf("len %d.\r\n", len );
    if( tot < len ){
        curr = (buff + tot);
        macdbg_prser("0x%08x:  ", lineoff);
        for( y = 0; y < (len - tot); y++ ){
             macdbg_prser("%02x ", (unsigned char)*(curr + y));
        }
        //padding with spaces
        //Ser_Printf("(len - tot) %d.\r\n", (len - tot) );
        if( (len - tot) < 16 ){
            for( y = 0; y < (32 - ((len - tot)*2)); y++ ){
                 macdbg_prser(" ");
            }
        }
        for( y = 0; y < 16-(len - tot); y++ ){
             macdbg_prser(" ");
        }
        macdbg_prser("  "); 
	   //Ser_Printf("(len - tot) %d.\r\n", (len - tot) );
        for( y = 0; y < (len - tot); y++ ){
            char c;
            c = *(curr + y);
            if( c >31 && c < 127 ){
                macdbg_prser("%c", c);
            }else{
                macdbg_prser("%c", '.');
            }
        }
    }
   // syslog( LOG_INFO, "%s", g_debug_buff );

   printf( "%s\n", g_debug_buff );

   
    memset( &g_debug_buff[0x00], 0x00, sizeof(g_debug_buff) );
    g_debug_buff_index = 0x00;
    return retval;
}






static int RegisterParse(RegisterObj *pRegObj, PACKAGE_HEAD *phead, DATA_HEAD *dhead, char *optbuff, char *buff, short seq)
{
    char out[2048];
    data_regist_response *res;
    int ret, outlen;
                 
    if( seq == -1 ){                              
        //force logout
        //db_log(D_LOG_ERR, "net error reconnect\n");
        printf("%s:%d net error reconnect..\n",__func__, __LINE__);
        //mQuitPro = 1;
        //exit(-1);
        return 0;
    }
          
    switch( phead->headp & 0x1f )
    {          
      case force_logout:
        //db_log(D_LOG_INFO, "force disconnect exit !!!!!\n");
        printf("%s:%d force disconnect exit!!.\n",__func__, __LINE__);
        //LogPlugStatus(log_forcequit);
        //exit( -1);
      break;
                                           
      case heart_ack:
        //db_log(D_LOG_INFO, "Register: heart beat ack\n");
        printf("########%s:%d force disconnect exit!!.\n",__func__, __LINE__);
        /*time_t curtime;
          time(&curtime);
          mHeartTime = curtime;*/
      break;
			
      case management_pack_ack:
			//d_log(D_LOG_INFO, "Register: packtype_dev_bissness_pack_ack\n");
            printf("########%s:%d Register: packtype_dev_bissness_pack_ack\n",__func__, __LINE__);
      break;

      case management_pack_down:
			//d_log(D_LOG_DEBUG, "Register: packtype_dev_bissness_pack_down\n");
            printf("########%s:%d Register: packtype_dev_bissness_pack_down\n",__func__, __LINE__);
      break;
		
      case bissness_pack_ack:
			//d_log(D_LOG_DEBUG, "Register: packtype_dev_bissness_pack_ip_ack\n");
            printf("########%s:%d Register: packtype_dev_bissness_pack_ip_ack\n",__func__, __LINE__);
      break;
			
      case bissness_pack_down:
			//d_log(D_LOG_DEBUG, "Register: packtype_dev_bissness_pack_ip_down\n");
            printf("########%s:%d Register: packtype_dev_bissness_pack_ip_down\n",__func__, __LINE__);
      break;
                                             
      case ack:
        //if(!mRegist)
        {				
        //d_log(D_LOG_DEBUG, "regist ack ===len %d\n",dhead->length);
        printf( "regist ack len = %d.\n", dhead->length );
        if( dhead->optLength > 0 ){
            //d_log(D_LOG_DEBUG, "optlen %d, error code = [%d]\n",dhead->optLength,*(short*)optbuff);
            printf( "optlen %d, error code = [0x%x] \n", dhead->optLength, *(short*)optbuff );   
        }		

		//macdbg_dmphex(optbuff, dhead->optLength);
		//macdbg_dmphex(buff, dhead->length);

		
        if( dhead->length > 0 ){
            ret = aes_decrypt( (char*)gProtoolKey, buff, out, dhead->length, &outlen );   
            if( ret != 0 ){
                break;
            }	
			res = (data_regist_response*)(out + 2);                        
			memcpy( pRegObj->mLoginPluginId, res->pluginId, 32 );                                     
                                   			
			//d_log(D_LOG_DEBUG, "regist ack======ip:%d,%d,%d,%d,port:%d\n",
			//res->ip[0],res->ip[1],res->ip[2],res->ip[3],res->port);
            printf( "%s: regist ack======ip:%d,%d,%d,%d,port:%d,PluginId:%s\n",
                    __func__, res->ip[0],res->ip[1],res->ip[2],res->ip[3], res->port, pRegObj->mLoginPluginId );   
            pro_para.link_status=1;
			sprintf(pRegObj->mLoginServer,"%d.%d.%d.%d",res->ip[0],res->ip[1],res->ip[2],res->ip[3]);
			pRegObj->mLoginPort = swapInt16(res->port);
			pRegObj->RegisterOk=1;
			memcpy(pRegObj->mLoginTocken, res->token, 32);
			//d_log(D_LOG_DEBUG, "regist ok tocken ,port %d\n", pRegObj->mLoginPort);
            printf( "%s: regist ok tocken, port %d\n",__func__, pRegObj->mLoginPort );
        }else{
            //d_log(D_LOG_ERR, "regist ack error\n");
            printf("########%s:%d regist ack error\n",__func__, __LINE__);
		}
        }
      break;
                  	
      default:
      break;
    }
    return 0;
}

















int SendRegData(int sock, char *data, int size)
{
	int sLen;
	
	if( size>4100 ) return -1;
	if( size<=0 ) return -1;
	if( data==NULL ) return -1;

	if( sock<=0 )
	{
		return -1;
	}
	sLen=0;
	sLen=send(sock, data, size, 0);

	//printf("SendRegData: \n");
    //macdbg_dmphex(data, size);
	

	if( sLen==size ) return 0;
	return -1;
}











static void *RegisterHandle( void *args )
{      
    int ret;
    char buffer[1508];
    HeadInfo HDinfo;
    char optbuff[1024];
    short seq;
    RegisterObj *pRegObj = (RegisterObj *)args;
         
    while( pRegObj->ServerRun ){

		seq = 5;
      ret = RegPop(pRegObj->pRQ, &HDinfo, optbuff,buffer,&seq);
      if( ret == 0 ){
          if( (HDinfo.pkgHead.headp & 0x1f) == 11 ){
              //db_log(D_LOG_DEBUG, "regist or connnect ack len = %d,%d\n",HDinfo.dataHead.length, HDinfo.dataHead.optLength);
              //printf("%s regist or connnect ack len = %d, %d.\n",__func__, HDinfo.dataHead.length, HDinfo.dataHead.optLength ); 
          }

		  printf( "seq = %d\n", seq );
          RegisterParse(pRegObj, &HDinfo.pkgHead, &HDinfo.dataHead, optbuff, buffer, seq); 
      }
      usleep(10000);
	}
    return NULL;
}




















static void *RegisterRecv( void *args )
{
	char buffer[1508];
	int skt, ret, rxLen, rxErrCnt, netErrCnt, errCnt;
	fd_set rdfd;
	struct timeval tv;
	RegisterObj *pRegObj=(RegisterObj *)args;

	skt = pRegObj->sock;
	netErrCnt=0; rxErrCnt=0; errCnt=0;
	while( pRegObj->ServerRun )
	{
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		FD_ZERO(&rdfd);
		FD_SET(skt, &rdfd);
		ret = select(skt+1, &rdfd, NULL, NULL, &tv );
		if( pRegObj->ServerRun==0 ) break;
		if( ret<0 )
		{
			if( netErrCnt<5 )
			{
				netErrCnt++;
				//d_log(D_LOG_ERR, "Net error.\n");
                printf("########%s:%d Net error...\n",__func__, __LINE__);
			}
			else
			{
				//ServerRun=0;
//				mQuitPro=1;
				break;
			}
		}
		else if( ret>0 )
		{
			netErrCnt=0;
			if(FD_ISSET(skt, &rdfd))
			{ 
				rxLen = recv(skt, buffer, 1500, 0);
				if( rxLen>0 )
				{
					rxErrCnt=0;
					errCnt=0;
					netErrCnt=0;
					RegPush(pRegObj->pRQ, buffer, rxLen);
				}
				else if( rxLen==0 )
				{
					if( rxErrCnt<1000 )
					{
						rxErrCnt++;
						usleep(10000);
					}
					else
					{
						//d_log(D_LOG_ERR, "Close link.\n");
                        printf("########%s:%d Close link..\n",__func__, __LINE__);
//						mQuitPro=1;
						break;
					}
				}
				else if( rxLen<0 )
				{
					if( errCnt<100 )
					{
						errCnt++;
						usleep(100000);
					}
					else
					{
//						mQuitPro=1;
						//d_log(D_LOG_ERR, "receive error.\n");
                        printf("########%s:%d receive error.\n",__func__, __LINE__);
                        break;
					}
				}
			}
			else
			{
				//d_log(D_LOG_ERR, "unknow err.\n");
                printf("########%s:%d unknow err...\n",__func__, __LINE__);
			}
		}
	}

	close(skt);
	
	return NULL;
}


static int Reg_SocketInit( const char *pServerIp, RegisterObj *pRegObj )  
{      
    int skt, ret, reuseFlag, tryCnt, sndBufSize;
	U32 mode;
	fd_set writeFd;
	struct timeval to;
	int chkErr;
	socklen_t errLen;

	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(pRegObj->port); 
	server_addr.sin_addr.s_addr = inet_addr(pServerIp);

	skt = socket(AF_INET, SOCK_STREAM, 0);
	if( skt<0 ){
		//db_log(D_LOG_ERR,"Create socket failed!\n");
        printf( "%s : %d Create socket failed!..\n",__func__, __LINE__);
        return -1;
	}
     
	reuseFlag = 1;
	if( setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseFlag, sizeof(reuseFlag))< 0 ){
		//db_log(D_LOG_ERR,"setsockopt(SO_REUSEADDR) Failed!\n");
        printf("%s:%d setsockopt(SO_REUSEADDR) Failed!\n",__func__, __LINE__);
        close(skt);
		skt = 0;   
		return -1;
	}

	sndBufSize = 16384;
	if( setsockopt(skt, SOL_SOCKET, SO_SNDBUF, (void*)(&sndBufSize), sizeof(int)) < 0 ){
		//db_log(D_LOG_ERR,"setsockopt(SO_SNDBUF) Failed!\n");
        printf("########%s:%d setsockopt(SO_SNDBUF) Failed!!\n",__func__, __LINE__);
		close(skt);
		skt=0;
		return -1;
	}
        
	mode = 1;
	ioctl(skt, FIONBIO, &mode);
	//db_log(D_LOG_DEBUG, "Connect server ......\n");
    printf("%s : %d Connect server .\n", __func__, __LINE__ );         

    /////////////////////////////////////////
	tryCnt=0;
	ret = connect(skt, (struct sockaddr*)&server_addr, sizeof(server_addr)); 
    //printf( "connect ret = %d.\n", ret ); 
	if( ret == 0 ){    
		//db_log(D_LOG_DEBUG, "connected.\n");
        printf("%s:%d connected!!!\n",__func__, __LINE__);
        return 0;
	}else{
		if( (errno == EINPROGRESS) || (errno==EWOULDBLOCK) ){
			//db_log(D_LOG_DEBUG, "connecting...\n");
            //printf( "%s : %d connecting.. \n", __func__, __LINE__ );         
			for( tryCnt=0; tryCnt<120; tryCnt++ ){        
				 FD_ZERO(&writeFd);
				 FD_SET(skt, &writeFd);
				 to.tv_sec = 0;
				 to.tv_usec = 100000;
				 ret = select(skt + 1, NULL, &writeFd, NULL, &to );
				 if( ret < 0 ){
					 usleep(100000);
					 continue;
				 }
				 if( ret == 0 ){
					 continue;
				 }else{
					 if( !FD_ISSET(skt, &writeFd) ){
						 //db_log(D_LOG_ERR, "err, no events found!\n");
                         printf("########%s:%d err, no events found....\n",__func__, __LINE__);
						 usleep(100000);
						 continue;
					 }else{
						 //db_log(D_LOG_DEBUG, "Terminal device may success\n");
						 chkErr=1;
						 errLen = sizeof(int);
						 ret = getsockopt(skt, SOL_SOCKET, SO_ERROR, &chkErr, &errLen);
						 if( ret<0 || chkErr!=0 ){
							 tryCnt=121;  // failed
						 }else{
							 //db_log(D_LOG_DEBUG, "Connect success.\n");
                             printf("in %s: tryCnt = %d, Connect success.\n", __func__, tryCnt );    
						 }
						 break;
					 }
				 }
			}
		}else{
			tryCnt=121;
		}
	}

	if( tryCnt<120 ){
		pRegObj->sock=skt;
		return 0;
	}
	
	close(skt);
	pRegObj->sock=0;
	//db_log(D_LOG_ERR, "Connect failed.\n");
    printf("########%s:%d Connect failed.xxxx.\n",__func__, __LINE__);
	return -1;
}


int Reg_ServerInit(RegisterObj *pRegObj)
{
	int ret;
	char ServerIpAddr[64];

	if( pRegObj->ServerRun==1 ){
		//db_log(D_LOG_ERR, "Client running.\n");
        printf("########%s:%d Client running...\n",__func__, __LINE__);
        return -1;
	}
	pRegObj->ServerRun = 1;

	//ret=GetServerIp(pRegObj->host, ServerIpAddr);
    ret=get_server_ip(pRegObj->host, ServerIpAddr);
	if( ret !=0 ){
		//db_log(D_LOG_ERR, "GetHostIp error\n");
        printf("########%s:%d GetHostIp error..\n",__func__, __LINE__);
        return -1;
	}
	
	ret = Reg_SocketInit(ServerIpAddr, pRegObj);
	if( ret != 0 ){
		//db_log(D_LOG_ERR,"SocketInit error\n");
        printf("%s:%d SocketInit errorr..\n",__func__, __LINE__);
        return -1;
	}

	ret = pthread_create( &pRegObj->HandlePid, NULL, RegisterHandle, pRegObj);
	if( ret !=0 ){
		//db_log(D_LOG_ERR, "Create HandleRegProc failed\n");
        printf("%s:%d Create HandleRegProc failed...\n",__func__, __LINE__);
        return -1;
    }
                                   	
	pRegObj->ServerState = 1;                                                                 
	ret=pthread_create(&pRegObj->RecvPid, NULL, RegisterRecv, pRegObj);             
	if( ret !=0 ){               
        //db_log(D_LOG_ERR,"Create RecvRegProc error.\n");
        printf("%s:%d Create RecvRegProc error..\n",__func__, __LINE__);
        return -1;              
    }
	pRegObj->ServerState = 2;             
    return 0;                     
}















// must pair call with ServerInit
int Reg_DeleteServer(RegisterObj *pRegObj)
{
	int ret;

	ret=0;
//	if( ServerRun==1 )
	{
		pRegObj->ServerRun = 0;
		sleep(1);
		switch( pRegObj->ServerState )
		{
			case 1:
				pRegObj->ServerState=0;
				ret=pthread_join(pRegObj->HandlePid, NULL);
				if( ret!=0 )
				{
					//d_log(D_LOG_ERR,"Stop response thread error...\n");
                    printf("########%s:%d Stop response thread error....\n",__func__, __LINE__);
				}
				pRegObj->HandlePid=0;
				break;
				
			case 2:
				pRegObj->ServerState=0;
				ret=pthread_join(pRegObj->HandlePid, NULL);
				if( ret!=0 )
				{
					//d_log(D_LOG_ERR,"Stop response thread error...\n");
                    printf("########%s:%d Stop response thread error....\n",__func__, __LINE__);
				}
				pRegObj->HandlePid=0;
				
				ret=pthread_join(pRegObj->RecvPid, NULL);
				if( ret!=0 )
				{
					//d_log(D_LOG_ERR,"Stop rtsp client stream error...\n");
                    printf("########%s:%d Stop rtsp client stread error....\n",__func__, __LINE__);
				}
				pRegObj->RecvPid=0;
				break;
		}
		
		if( pRegObj->sock ) close(pRegObj->sock);
		pRegObj->sock =0;
	}

	return ret;
}

