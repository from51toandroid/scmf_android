//
// Created by 汪洋 on 2019-07-25.
//

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "protool.h"
#include "product.h"
#include "tcpserver.h"
#include <errno.h>

#ifdef DES_SUPPORT
#include <mbedtls/des.h>
#else

#include "openssl/aes.h"

#endif

#include "devsdk.h"
#include "plugsdk.h"
#include "plug.h"
#include "Md5.h"
#include "fwupgrade.h"
#include "plugupgrade.h"
#include "def_struct.h"


int mQuitPro;
long mHeartTime;
int mLogin;
char gProtoolKey[32] = {0};
extern PLUG_CONFIG local_cfg;
time_t opt_time;
volatile int runMode = 1;
extern PRO_PARA pro_para;
U8 gRoutemac[6];
volatile int ServerRun;


void init_para() {
    time(&opt_time);
    pro_para.login_status = 0;
    pro_para.link_status = 0;
    pro_para.protool_status = 0;
    pro_para.heart_interval = 10;
    pro_para.heart_time = 0;
    pro_para.link_port = 0;
    bzero(&pro_para.link_server, sizeof(pro_para.link_server));
    bzero(&pro_para.link_tocken, sizeof(pro_para.link_tocken));
    bzero(&pro_para.link_plugin_id, sizeof(pro_para.link_plugin_id));
    bzero(&pro_para.conf, sizeof(pro_para.conf));
}

#define PRINTHEX 1

void printhex(unsigned char *src, int len, const char *func_name) {
#ifdef PRINTHEX
    printf("######## func: %s len = %d\n", func_name, len);
    if (src == NULL) {
        return;
    }
    if (len > (1024 * 3 - 1)) {
        return;
    }
    char x[1024 * 3] = {0};
    int i = 0;
    for (i = 0; i < len; i++) {
        char tmp[10] = {0};
        {
            snprintf(tmp, 8, "%02X ", src[i]);
            strcat(x, tmp);
        }
    }
    printf("%s\n", x);
#endif
    return;
}

int get_link_status() {
    return pro_para.link_status;
}

unsigned char get_check_sum(unsigned char *pack, int pack_len) {
    unsigned char check_sum = 0;
    while (--pack_len >= 0) {
        check_sum += *pack++;
    }
    return check_sum;
}





void new_protool() 
{
    printf( "pthread_mutex_init::Protool.\n");
    printf( "&pro_para.protool_mutex = %p.\n", &pro_para.protool_mutex );
    pthread_mutex_init(&pro_para.protool_mutex, NULL);   
    //init_para();
    new_program_profile(CONFIG_FIE);
}





void delete_protool() 
{
    pthread_mutex_destroy(&pro_para.protool_mutex);

    init_para();

    delete_program_profile();

}








static void read_conf() 
{     
    get_key_value_str( APP_DEV, KEY_PIN, DEF_EMPTY, pro_para.conf.pin_code, sizeof(pro_para.conf.pin_code) );  
	//PIN码固定12位

	printf( "PIN = %s\n", pro_para.conf.pin_code );

	get_key_value_str(APP_DEV, "SN", DEF_EMPTY, pro_para.conf.sn_code, sizeof(pro_para.conf.sn_code));
	printf( "SN = %s\n", pro_para.conf.sn_code );

	
    get_key_value_str(APP_PORTOOL, KEY_KEY, DEF_EMPTY, pro_para.conf.protool_key, sizeof(pro_para.conf.protool_key));
	printf( "KEY = %s\n", pro_para.conf.protool_key );

	

	get_key_value_str(APP_PORTOOL, KEY_SERVER1, DEF_SERVER_IP, pro_para.conf.reg_server1, sizeof(pro_para.conf.reg_server1));
	//服务器的域名或IP地址
	printf( "SERVER1 = %s\n", pro_para.conf.reg_server1 );

	get_key_value_str(APP_PORTOOL, KEY_SERVER2, DEF_SERVER_IP, pro_para.conf.reg_server2, sizeof(pro_para.conf.reg_server2));
	printf( "SERVER2 = %s\n", pro_para.conf.reg_server2 );
	

	get_key_value_str(APP_PORTOOL, KEY_PORT, DEF_SERVER_PORT, pro_para.conf.reg_port, sizeof(pro_para.conf.reg_port));
	printf( "PORT = %s\n", pro_para.conf.reg_port );

	

    //printf("config para:----\n");
    //printf("pin_code-----%s----%d\n", pro_para.conf.pin_code, (int) strlen(pro_para.conf.pin_code));
    //printf("sn_code-----%s----%d\n", pro_para.conf.sn_code, (int) strlen(pro_para.conf.sn_code));

	memcpy( &gProtoolKey, pro_para.conf.protool_key, strlen(pro_para.conf.protool_key));

	//printf( "gProtoolKey-----%s----%d\n", pro_para.conf.protool_key, (int)strlen(pro_para.conf.protool_key) );
    //printf( "protool_key-----%s----%d\n", pro_para.conf.protool_key, (int)strlen(pro_para.conf.protool_key) );
    //printf("reg_server1-----%s----%d\n", pro_para.conf.reg_server1, (int) strlen(pro_para.conf.reg_server1));
    //printf("reg_server2-----%s----%d\n", pro_para.conf.reg_server2, (int) strlen(pro_para.conf.reg_server2));
    //printf("reg_port-----%s----%d\n", pro_para.conf.reg_port, (int) strlen(pro_para.conf.reg_port));

}

void get_runmode() {

    int ret;
    char regIp[64], *pTmp;
    ret = get_server_ip(pro_para.conf.reg_server1, regIp);
    if (ret == 0) {
        pTmp = NULL;
        pTmp = strstr(regIp, "222.113");
        if (pTmp != NULL) {
            runMode = 0;//ceshi
            printf("info: check test mode 171.208.222.113.>>>\n\n");
        } else {
            runMode = 1;  //formal
            printf("info: check formal mode .>>>\n\n");
        }

    } else {
        printf("ERROR: check test mode failed.\n\n");
        runMode = 1;
    }
}


void login_process() 
{
    pthread_t login_thread_id;
              
    //new login tcp
    //int login_port = atoi(pro_para.link_port);

    //create login thread
    int ret = pthread_create(&login_thread_id, NULL, login_thread, 0);
    if( 0 != ret ){
        printf("Unable to create a thread for login_status thread %s error !!!!!", strerror(errno));
        printf("exit at [%s] %d \n\n", __func__, __LINE__);
        exit(-1);
    }
}





int init_protool() {
    int ret;
    /// init para
	REBOOT:
	
	init_para();
    new_protool(); 
	//read conf para
    read_conf();

    //obtain big end or litter end
    get_end_type();

    get_runmode();
    //register thread
    
	
	
	printf("exit.1\n");
    ServerRun = 0;  
     

	

    mQuitPro = 0;
    mLogin = 0;
    mHeartTime = 0;
    ret = RegisterStart();
    if (ret != 0) {
        //d_log(D_LOG_ERR, "RegisterStart error.\n");
        printf("exit at [%s] init_protool error!xxx\n", __func__ );

        delete_protool();

		//printf("exit.2\n");
        sleep(3);
        goto REBOOT;
    }
	
    //login process
    //过了前面的登录认证,才会走下面的流程
    login_process();

	printf("exit.3\n");
          
    return 0;
}

int destroy_protool() {
    pro_para.protool_status = 2;
    sleep(1);
    destroy_tcp_server();
    delete_tcp_server();

    return 0;
}


void *login_thread(void *arg) 
{
    login_handle();
    return 0;
}




   
void login_handle() 
{     
    int ret = 0;
    int i = 0, j = 0;
    pthread_t checkThreadId;
    printf("Login xxxx %d,%d\n", pro_para.login_status, pro_para.protool_status);
    printf("quit login_status,start link_status %s:%d\n", pro_para.link_server, pro_para.link_port);

    //new link tcp
    if( 0 != new_tcp_server(pro_para.link_server, pro_para.link_port) ){
        printf("######## ERROR: new_tcp_server failed.\n\n");
    }
    //connect to link tcp server, 5 times
    for( i=0; i<5; i++ ){
         ret = init_tcp_server();
         if( 0 != ret ){
             printf("mTcpConnectServer->Init() error\n");
             sleep(3);
             continue;
         }else{
             break;
         }
    }


    //printf( "i = %d\n", i);

    if (i == 5 || pro_para.protool_status) {
        printf("link_status failed !\n");
        destroy_tcp_server();
        delete_tcp_server();
		printf( "pro_para.protool_status = %d\n", pro_para.protool_status);
        if( pro_para.protool_status != 2 ){
            init_protool();
        }
        return;
    }

    ret = running();
    if (0 != ret) {
        printf("mTcpConnectServer->Running() error exit !!!!!\n");
        printf("########exit at [%s] %d \n\n", __func__, __LINE__);
        exit(-1);
    }

    /// register the callback function
    register_cb(0, rec_cb);

    for (i = 0; i < 3; i++) {


        link_service();

        for (j = 0; j < 60; j++) {
            if ((pro_para.protool_status) || (pro_para.login_status))
                break;
            sleep(1);
        }
        if ((pro_para.protool_status) || (pro_para.login_status))
            break;
    }
    //printf("link_status break i = %d\n",i);
    if ((i == 3) || (pro_para.protool_status)) {
        printf("link_status failed !\n");
        destroy_tcp_server();
        delete_tcp_server();
        //delete mTcpConnectServer;
        if (pro_para.protool_status != 2)
            init_protool();
        return;
    }

    ret = pthread_create(&checkThreadId, NULL, check_thread, 0);
    if (0 != ret) {

        printf("Unable to create a thread for checkThreadId thread %s exit !!!!!",
               strerror(errno));
        printf( "exit at [%s] \n", __func__ );
        exit(-1);
    }
    heart_beep();

    //固件版本信息上报
    for (i = 0; i < 3; i++ ) {
        ret = dev_version_report();
        if (ret == 0) {
            printf("dev_version_report done!\n");
            break;
        }
        sleep(1);
    }



    while (!pro_para.protool_status) {
        for (i = 0; i < pro_para.heart_interval; i++) {
            if (pro_para.protool_status) break;
            sleep(1);
        }
        heart_beep();
		//程序最后在这里循环跑
    }

	printf("net error!!! restart connect scmf server...\n");

    destroy_tcp_server();
    delete_tcp_server();

    if (pro_para.protool_status == 1) {
        sleep(10);
        init_protool();
    }

}





int heart_beep() 
{                                                
    data_heartbeat_opt optdata = {0};
    time_t curtime;
    time(&curtime);
    //optdata.timestamp = swapInt32(curtime);
    optdata.timestamp = swapInt32(0x5ECF0001);
    U8 checks = get_check_sum((unsigned char *) &optdata, sizeof(optdata));

    int ret = send_up_data((unsigned char *) &optdata,
                           (unsigned char *) 0,
                           heart,
                           ACK_YES,
                           sizeof(optdata),
                           0,
                           checks);
    return ret;
}








//
int login_service()  //register process
{

    char optdata[LOGIN_OPT_LENGTH + 1]; // 可选部数据
    char decdata[1024];       // 待加密数据
    int datalen = 0;          // 数据部长度
    char miwen_hex[1024];     // 加密后数据
    int milen = 0;            // 加密后数据长度
    U8 checkkdata[1024];      // 校验和数据内容

    U8 deviceSnLength = 0;    // sn长度

    memset(optdata, 0, LOGIN_OPT_LENGTH + 1);
    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);

    /// 判断pinc和key
    if ((pro_para.conf.pin_code == NULL) || (strlen(pro_para.conf.pin_code) == 0)) {
        printf("!\n!\n!\n!\n########ERROR: PIN is empty, plese check your plug.conf!\n!\n!\n!\n");
        exit(-1);
    }
    if ((pro_para.conf.protool_key == NULL) || (strlen(pro_para.conf.protool_key) == 0)) {
        printf("!\n!\n!\n!\n########ERROR: KEY is empty, plese check your plug.conf!\n!\n!\n!\n");
        exit(-1);
    }

    /// 获取sn
    if (0 != get_dev_sn(pro_para.conf.sn_code)) {
        printf("!\n!\n!\n!\n########ERROR: SN is empty!\n!\n!\n!\n");
        exit(-1);
    };
    // 赋值sn长度
    deviceSnLength = strlen(pro_para.conf.sn_code);

    /// 获取mac
    U8 mac[6] = {0};//{0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    if (0 != memcmp(mac, local_cfg.mac, 6)) {
        memcpy(mac, local_cfg.mac, 6);
    } else {
        get_dev_mac(mac);
    }
    printf("######## gwdev mac: %02x %02x %02x %02x %02x %02x \n\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /// ------- 可选部 --------
    optdata[0] = 2;
    memcpy(&optdata[3], pro_para.conf.pin_code, strlen(pro_para.conf.pin_code));


    /// ------- 待加密数据 --------
    // 前两个字节是包序号seq
    // 赋值pincode
    memcpy(&decdata[PACKAGE_SEQ_LENGTH], pro_para.conf.pin_code, strlen(pro_para.conf.pin_code));
    // 赋值sn长度
    decdata[PACKAGE_SEQ_LENGTH + 12] = deviceSnLength;
    // 赋值sn
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 12 + 1], pro_para.conf.sn_code, deviceSnLength);
    // 赋值mac
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 13 + deviceSnLength], mac, 6);
    // 计算数据部长度，包头 + pincode + sn长度 + sn + mac + deviceID
    datalen = PACKAGE_SEQ_LENGTH + 13 + deviceSnLength + 6 + 32;
#ifdef DES_SUPPORT
    int ret = des_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#else
    int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
    if (ret != 0)
        return ret;
    //printf("aes_crypt_ecb encode: ----len %d---\n",dlen);

    /// ------- 校验和 --------
    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, LOGIN_OPT_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH + LOGIN_OPT_LENGTH, decdata, datalen);

    U8 checksum = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + LOGIN_OPT_LENGTH + datalen);

    return send_up_data((unsigned char *) optdata,
                        (unsigned char *) miwen_hex,
                        login_conform,
                        ACK_YES,
                        LOGIN_OPT_LENGTH,
                        midatalen,
                        checksum);

}

int link_service() {
    data_link_opt optdata;    // 可选部
    data_link data;           // 数据部的包数据
    char decdata[1024];       // 待加密数据
    int datalen = 0;          // 数据部长度
    char miwen_hex[1024];     // 加密后数据
    int milen = 0;            // 加密后数据长度
    U8 checkkdata[1024];      // 校验和数据内容

    memset(&data, 0, sizeof(data));
    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);
    printf("########%s: %d start login_service\n\n", __func__, __LINE__);
    if ((!pro_para.login_status) && (!pro_para.link_status)) {
        printf("########%s: %d start login_service error\n\n", __func__, __LINE__);
        return -1;

    }


    /// ------- 可选部 --------

    memset(optdata.devicePin, 0, sizeof(optdata.devicePin));
    memcpy(optdata.devicePin, pro_para.conf.pin_code, 12);

    memset(optdata.token, 0, sizeof(optdata.token));
    memcpy(optdata.token, pro_para.link_tocken, 32);

    /// ------- 数据部 --------
    data.protocol = 0;
    data.deviceVersion[0] = VERSION_0;
    data.deviceVersion[1] = VERSION_1;
    data.deviceVersion[2] = VERSION_2;
    data.deviceVersion[3] = VERSION_3;

    // 计算数据部长度
    datalen = PACKAGE_SEQ_LENGTH + sizeof(data_link);

    /// ------- 待加密数据 --------
    memcpy(&decdata[2], &data, sizeof(data_link));
#ifdef DES_SUPPORT

xxxx
    int ret = des_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#else
    int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
    if (ret != 0)
        return ret;

    /// ------- 校验和 --------
    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, &optdata, sizeof(optdata));
    memcpy(checkkdata + PACKAGE_DATA_LENGTH + sizeof(optdata), &decdata, datalen);

    U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + sizeof(data_link_opt) + datalen);

    return send_up_data((unsigned char *) &optdata,
                        (unsigned char *) miwen_hex,
                        link_conform,
                        ACK_YES,
                        sizeof(optdata),
                        milen,
                        checks);

}

void *check_thread(void *arg) {
    //Protool *pServer = (Protool *)arg;
    check_handle();
    return 0;
}

void check_handle() {
    while (!pro_para.protool_status) {
        time_t curtime;
        time(&curtime);
        long updateTime = curtime;

        //此处对心跳ack监测

        if ((pro_para.heart_time != 0) && (updateTime - pro_para.heart_time) > pro_para.heart_interval * 3) {
            pro_para.protool_status = 1;
            printf("heart beep error\n");
        }

        sleep(1);
    }
}

int rec_cb(void *powner, PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq) 
{
    return cb_handle(phead, dhead, optbuff, buff, seq);
}

int cb_handle( PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq ) 
{
    char out[1024], tmpA[1024];
    int i = 0;
    int outlen;
	time_t curtime;
          
    //printf(">>>>>> cb_handle seq %d\n", seq);
        
    if( seq == -1 ){
        //force logout
        printf("net error reconnect\n");
        pro_para.protool_status = 1;
        return 0;
    }
                   
    switch( (phead.headp & 0x1f) ) 
    {  
      case force_logout:
        printf("force disconnect exit !!!!!\n");
        printf("exit at [%s].\n", __func__ );
        exit(-1);
      break;
      case heart_ack:
        time( &curtime );   
        pro_para.heart_time = curtime;
		//printf( "heart beep ack, pro_para.heart_time = %ld.\n", pro_para.heart_time );

		printf("Starting heart beep on %s", ctime(&curtime));
		
		//macdbg_dmphex(optbuff, dhead.optLength);
		//macdbg_dmphex(buff, dhead.length);
      break;
       
      case management_pack_ack: 
	  {
        //printf( "management_pack_ack %d\n", dhead.length );
		//macdbg_dmphex(optbuff, dhead.optLength);
		//macdbg_dmphex(buff, dhead.length);
		
        if( dhead.length > 0 ){
            #ifdef DES_SUPPORT
            int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #else
            int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #endif
            if( ret != 0 ){
                printf("descrypt error\n");
                break;
            }
            /* for( int i=2; i<outlen; i++ ){
                    printf("%c ",out[i]);
               }
               printf("\n");
             */
        }
      }
	  break;
          
      case management_pack_down:
      {   
        printf("management_pack_down\n");
        if( dhead.length > 0 ){
            #ifdef DES_SUPPORT
            int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #else
            int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #endif
            if( ret != 0 ){
                printf("descrypt error\n");
                break;
            }
            data_device_mt_req *data = (data_device_mt_req *) (out + 2);
            data->length = swapInt16(data->length);
            printf( "management_pack_down datatype %d,len %d\n", data->dataType, data->length );          
            short len = data->length;
            char *pData = (out + 5);
            printf("len %d,outlen = %d,%d\n", dhead.length, outlen, out[outlen - 1]);
            /* for( int i=2; i<outlen; i++ ){
                    printf("%c ",out[i]);  
               }
               printf("\n");
             */
            memset(tmpA, 0, 1024);
            memcpy(tmpA, &dhead.length, 2);
            if( dhead.optLength ){
                memcpy(tmpA + 2, optbuff, dhead.optLength);
            }       
            if( outlen > 0 ){
                memcpy(tmpA + 2 + dhead.optLength, out, outlen);
            }
            U8 checks = get_check_sum((unsigned char *) tmpA, 2 + dhead.optLength + outlen);
            printf("checks = %x, %x\n", checks, dhead.checksum);
            if( checks != dhead.checksum ){
                printf("checksum err\n");
                send_management_ack_data(checksum_err, 0, 0);
            }else{
                int r;
                memset(tmpA, 0, sizeof(tmpA));
                switch( data->dataType )  
                {    
                  case datatype_upgrade_cmd:
                    //下发fw升级指令
                    memcpy(tmpA, pData, data->length);
                    r = gw_parse_fwupgrade_cmd(tmpA);
                    if( r == 0 ){
                        send_management_ack_data(0, 0, 0);  //(unsigned char *) response, responselen);
                        printf(">>>>>>>>>>>>>>fwupgrade_cmd ack.\n");
                    }else{
                        char msg[] = "parse fw upgrade cmd error";
                        printf("parse fw upgrade cmd error\n");
                        send_management_ack_data(r, (unsigned char *) msg, strlen(msg));
                    }
                  break;
                  case datatype_upgrade_param:
                    //下发fw升级数据
                    memcpy(tmpA, pData, data->length);
                    r = gw_parse_fwupgrade_param(tmpA);
                    if( r == 0 ){
                        send_management_ack_data(0, 0, 0); 
                        //(unsigned char *) response, responselen);
                        printf(">>>>>>>>>>>>>>upgrade_fwparam ack.\n");
                        gw_parse_do_fwupgrade();
                    }else{
                        char msg[] = "parse fw upgrade param error";
                        printf("parse fw upgrade param error\n");
                        send_management_ack_data(r, (unsigned char *) msg, strlen(msg));
                    }
                  break;
                  case datatype_plug_update_cmd:
                    //下发plug升级指令
                    memcpy(tmpA, pData, data->length);
                    r = gw_parse_plugupgrade_cmd(tmpA);
                    if( r == 0 ){
                        printf(">>>>>>>>>>>>>>plug upgrade cmd ack success.\n");
                    }else{
                        printf("parse plug upgrade cmd error##ret=%d\n",r);
                    }
                  break;
                  case datatype_plug_update_file:
				  	//下发plug升级数据
                    memcpy(tmpA, pData, data->length);
                    r = gw_parse_plugupgrade_param(tmpA);
                    if( r == 0 ){
                        printf(">>>>>>>>>>>>>>upgrade plug param ack ok.\n");
                        gw_parse_do_plugupgrade();
                    }else{
                        char msg[] = "parse fw upgrade param error";
                        printf("parse fw upgrade param error\n");
                        send_management_ack_data(r, (unsigned char *) msg, strlen(msg));
                    }
                  break;
                  default:
                    send_management_ack_data(0, 0, 0); //(unsigned char *) response, responselen);
                  break;
                }
            }
        }
      }
	  break;
                                         
      case bissness_pack_ack:
      {
        printf("bissness_pack_ack %d\n", dhead.length);
        if( dhead.length > 0 ){
            #ifdef DES_SUPPORT
            int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out,dhead.length, &outlen);
            #else
            int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #endif
            if( ret != 0 ){
                printf("descrypt error\n");
                break;
            }
        }
      }
      break;
      case bissness_pack_down:
      {  
        printf("bissness_pack_down\n");
        if( dhead.length > 0 ){
            #ifdef DES_SUPPORT
            int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #else
            int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
            #endif
            if( ret != 0 ){
                printf("descrypt error\n");
                break;
            }
            char *pData = (out + 2);
            short len = outlen - 2;
            pData[len] = '\0';
            printf("len %d,outlen = %d,%02X\n", dhead.length, outlen, (unsigned char) out[outlen - 1]);
            memset(tmpA, 0, 1024);
            memcpy(tmpA, &dhead.length, 2);
            if( dhead.optLength ){
                memcpy(tmpA + 2, optbuff, dhead.optLength);
            }
            if( outlen > 0 ){
                memcpy(tmpA + 2 + dhead.optLength, out, outlen);
            }
            //printhex(checkkdata, 2+dhead.optLength+outlen, __func__);
            U8 checks = get_check_sum((unsigned char *) tmpA, 2 + dhead.optLength + outlen);
            printf("checks = %x, %x\n", checks, dhead.checksum);
            if( checks != dhead.checksum ){
                send_business_ack_data(checksum_err, "", 0);
            } else {
                /* char msgadd[] = ",\"sequence\":123}\0";
                   memcpy(&pData[len-1], msgadd, strlen(msgadd));
                   len = len + strlen(msgadd) - 1;
                 */
                int ret = 0;
                char response[1024 * 4] = "";
                int responselen = 0;
                ret = dev_data_down(pData, len);
                /* sleep(2);
                   char updataT[100] = "{\"sequence\":0,\"SongControlS\":2}";
                   int data_len = swapInt16(strlen(updataT));
                   send_dev_data_up(updataT, data_len);
                 */
            }
        }
      }
	  break;
           
      case ack:
      {               
        if( !pro_para.link_status ){
            short errerno = *(short *) optbuff;
            if( dhead.optLength > 0 ){
                printf("########login optlen %d, err code or radom [%d]\n\n", dhead.optLength, swapInt16(errerno));
            }
            printf("login ack ===len %d\n", dhead.length);
            if( errerno == 0 ){
                #ifdef DES_SUPPORT
                int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                #else
                int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                #endif
                if( ret != 0 ){
                    printf("descrypt error\n");
                    break;
                }
                data_login_response *res = (data_login_response *) (out + PACKAGE_SEQ_LENGTH);
                //session
                memcpy(pro_para.link_tocken, res->token, 32);
                //link IP
                sprintf(pro_para.link_server, "%d.%d.%d.%d", res->ip[0], res->ip[1], res->ip[2], res->ip[3]);
                //link port
                pro_para.link_port = swapInt16(res->port);
                //插件ID
                memcpy(pro_para.link_plugin_id, res->deviceId, 32);
                pro_para.login_status = 1;
                printf( "login ack======ip:%d,%d,%d,%d,port:%d,pluginId %s\n",
                         res->ip[0], res->ip[1], res->ip[2], res->ip[3], res->port, pro_para.link_plugin_id);
                printf("login ok tocken ,port %d\n", pro_para.link_port);
            }else{
                printf("login ack error\n");
            }
        }else{
            //printf("login ack\n");
			
			//macdbg_dmphex(optbuff, dhead.optLength);
		    //macdbg_dmphex(buff, dhead.length);
                      
            short errerno = *(short *)optbuff;
            if( dhead.optLength > 0 ){
                //printf("optlen %d, err code or radom [%d]\n", dhead.optLength, errerno);
            }    
            if( errerno == 0 ){
                #ifdef DES_SUPPORT
                int ret = des_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                #else
                int ret = aes_decrypt((char *) pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                #endif
                if( ret != 0 ){
                    printf("descrypt error\n");
                    break;
                }
                data_link_response *res = (data_link_response *)(out + PACKAGE_SEQ_LENGTH);
                pro_para.heart_interval = swapInt16(res->heartbeat);
                if( pro_para.heart_interval < 5 ){
                    pro_para.heart_interval = 5;
                }
                printf( "login ack heartbeep interval %d\n", swapInt16(res->heartbeat) );
                pro_para.login_status = 1;
            }else{
                printf("link ack error\n");
                //判断错误码,重新启动登录
                pro_para.protool_status = 1;
            }
        }  
      }
	  break;
      default:
      break;
    }
    return 0;             
}



















int send_up_data( unsigned char *optdata, unsigned char *data, PACKAGE_TYPE type, int isack, int optlen, int len, U8 checks ) 
{                              
    time_t curtime;              
    time( &curtime );                                   
    long opt = opt_time;                
    long cur = curtime;                       
   
    if (cur - opt == 0) {
        usleep(500000);
    }
    opt_time = curtime;

    PACKAGE_HEAD pk_head;
    DATA_HEAD dt_head;

    memset(&pk_head, 0, sizeof(pk_head));
    memset(&dt_head, 0, sizeof(dt_head));

    dt_head.optLength = optlen;

    if ((len == 0) && (optlen == 0))
        return -1;
    isack = ( isack|(ENCTYPE<<1))&0x7;
    pk_head.magic = swapInt16(DX_MAGIC_HEAD);
    pk_head.headp = (type | (isack<<5));
#ifdef DES_SUPPORT
    pk_head.headp |=0xC0;
#endif

    //printf("===%x\n",pk_head.headp);
    dt_head.checksum = checks;
    dt_head.length = swapInt16(len);


    char senddata[1024];
    memcpy(senddata, &pk_head, PACKAGE_HEAD_LENGTH);
    memcpy(senddata + PACKAGE_HEAD_LENGTH, &dt_head, DATA_HEAD_LENGTH);
    if ((optlen) && (optdata))
        memcpy(senddata + PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH, optdata, optlen);
    if ((len) && (data))
        memcpy(senddata + PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH + optlen, &data[0], len);

    int sendrt = send_data((unsigned char *) senddata, PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH + optlen + len);
    if (sendrt == -1) {
        printf("mTcpConnectServer->sendData error\n");
        return -1;
    }

    return 0;
}

int send_business_up_data(void *buff, int len) {
    int datalen;
    if (!pro_para.link_status)
        return -1;
    if ((len > 1022) || (len <= 0))
        return -1;

    char decdata[1024];
    char miwen_hex[1024];
    char checkkdata[1024];

    int milen = 0;

    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);

    memcpy(&decdata[PACKAGE_SEQ_LENGTH], buff, len);
    datalen = PACKAGE_SEQ_LENGTH + len;
#ifdef DES_SUPPORT
    int ret = des_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#else
    int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
    if (ret != 0)
        return ret;

    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, decdata, datalen);

    U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + datalen);

    return send_up_data((unsigned char *) 0,
                        (unsigned char *) miwen_hex,
                        bissness_pack_up,
                        ACK_YES,
                        0,
                        milen,
                        checks);

}

int send_management_up_data(void *buff, int len) {
    int datalen;
    if (!pro_para.link_status)
        return -1;
    if ((len > 1022) || (len <= 0))
        return -1;

    data_device_mo_req *p_dev_msg = (data_device_mo_req *) buff;

    char decdata[1024];
    char miwen_hex[1024];
    char checkkdata[1024];

    int milen = 0;

    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);


    memcpy(&decdata[PACKAGE_SEQ_LENGTH], &p_dev_msg->dataType, 1);
    U16 tmp = swapInt16(p_dev_msg->length);
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 1], &tmp, 2);
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 3], p_dev_msg->dataByte, p_dev_msg->length);
    datalen = PACKAGE_SEQ_LENGTH + 3 + p_dev_msg->length;

    p_dev_msg->length = swapInt16(p_dev_msg->length);

#ifdef DES_SUPPORT
    int ret = des_encrypt((char*)pro_para.conf.protool_key,decdata,miwen_hex,datalen,&milen);
#else
    int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
    if (ret != 0)
        return ret;

    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, decdata, datalen);

    U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + datalen);

    return send_up_data((unsigned char *) 0,
                        (unsigned char *) miwen_hex,
                        management_pack_up,
                        ACK_YES,
                        0,
                        milen,
                        checks);

}

int send_business_ack_data(short errcode, unsigned char *buff, int len) {
    int datalen = 0;
    if (!pro_para.link_status)
        return -1;
    if (len > 1022)
        return -1;

    char optdata[2];
    errcode = swapInt16(errcode);
    memcpy(optdata, &errcode, 2);

    if (len > 0) {
        char decdata[1024];
        char miwen_hex[1024];
        char checkkdata[1024];
        int milen = 0;

        memset(decdata, 0, 1024);
        memset(miwen_hex, 0, 1024);
        memset(checkkdata, 0, 1024);

        memcpy(&decdata[PACKAGE_SEQ_LENGTH], buff, len);
        datalen = len;
#ifdef DES_SUPPORT
        int ret = des_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#else
        int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
        if (ret != 0)
            return ret;

        short midatalen = milen;
        midatalen = swapInt16(midatalen);
        memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH + 2, decdata, datalen);

        U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + 2 + datalen);

        return send_up_data((unsigned char *) optdata,
                            (unsigned char *) miwen_hex,
                            bissness_pack_ack,
                            ACK_NO,
                            2,
                            milen,
                            checks);
    } else {
        int dlen = 0;
        char checkkdata[1024];
        memset(checkkdata, 0, 1024);

        short datalen = dlen;
        datalen = swapInt16(datalen);
        memset(checkkdata, 0, 1024);
        memcpy(checkkdata, &datalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);


        U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + 2);
        return send_up_data((unsigned char *) optdata,
                            (unsigned char *) 0,
                            bissness_pack_ack,
                            ACK_NO,
                            2,
                            0,
                            checks);
    }
}

int send_management_ack_data(short errcode, unsigned char *buff, int len) {

    if (!pro_para.link_status)
        return -1;
    if (len > 1022)
        return -1;
    char optdata[2];
    errcode = swapInt16(errcode);
    memcpy(optdata, &errcode, 2);

    if (len > 0) {
        char decdata[1024];
        char miwen_hex[1024];
        char checkkdata[1024];
        int milen = 0;

        memset(decdata, 0, 1024);
        memset(miwen_hex, 0, 1024);
        memset(checkkdata, 0, 1024);

        memcpy(&decdata[PACKAGE_SEQ_LENGTH], buff, len);

        int datalen = PACKAGE_SEQ_LENGTH + len;
#ifdef DES_SUPPORT
        int ret = des_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#else
        int ret = aes_encrypt((char *) pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
#endif
        if (ret != 0)
            return ret;

        short midatalen = milen;
        midatalen = swapInt16(midatalen);
        memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH + 2, decdata, datalen);

        U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + 2 + datalen);

        return send_up_data((unsigned char *) optdata,
                            (unsigned char *) miwen_hex,
                            management_pack_ack,
                            ACK_NO,
                            2,
                            milen,
                            checks);
    } else {
        int dlen = 0;
        char checkkdata[1024];
        memset(checkkdata, 0, 1024);

        short datalen = dlen;
        datalen = swapInt16(datalen);
        memcpy(checkkdata, &datalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);

        U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + 2);

        return send_up_data((unsigned char *) optdata,
                            (unsigned char *) 0,
                            management_pack_ack,
                            ACK_NO,
                            2,
                            dlen,
                            checks);
    }
}

/*
 * enckey：密钥
 * encbuf：加密数据
 * decbuf：加密后的数据
 * inlen：带加密数据长度
 * outlen：加密后数据长度
 */

#ifdef DES_SUPPORT
int des_encrypt(char *pKey, char *pIn, char *pOut, int inLen, int *outLen)
{
    int ret, nNumber, nTotal;
    U8 iv[16] = "";
    U8 *pPkcs=NULL;
    mbedtls_des_context ctx;

    if((!pIn)||(!pOut)) return -1;

    nTotal = inLen/8 + 1;
    nTotal = nTotal * 8;
    pPkcs = (U8 *)malloc(nTotal);
    if( pPkcs==NULL ) return -1;

    if( (inLen%8) > 0) nNumber = nTotal - inLen;
    else nNumber = 8;

    memset(pPkcs, nNumber, nTotal);
    memcpy(pPkcs, pIn, inLen);

    mbedtls_des_init( &ctx );
    mbedtls_des_setkey_enc(&ctx, (U8 *)pKey);
    ret=mbedtls_des_crypt_cbc(&ctx, MBEDTLS_DES_ENCRYPT, nTotal, iv, pPkcs, (U8 *)pOut);
    if( ret !=0 )
    {
        free(pPkcs);
        mbedtls_des_free( &ctx );
        return -1;
    }
    *outLen=nTotal;

    free(pPkcs);
    mbedtls_des_free( &ctx );
    return 0;
}

int des_decrypt(char *pKey, char *pIn, char *pOut, int inLen, int *outLen)
{
    int ret, nNumber, nTotal;
    U8 iv[16] = "";
    U8 *pPkcs=NULL;
    mbedtls_des_context ctx;

    if((!pIn)||(!pOut)) return -1;

    nTotal = inLen/8 + 1;
    nTotal = nTotal * 8;
    pPkcs = (U8 *)malloc(nTotal);
    if( pPkcs==NULL ) return -1;

    if( (inLen%8) > 0) nNumber = nTotal - inLen;
    else nNumber = 8;

    memset(pPkcs, nNumber, nTotal);
    memcpy(pPkcs, pIn, inLen);

    mbedtls_des_init( &ctx );
    mbedtls_des_setkey_dec(&ctx, (U8 *)pKey);
    ret=mbedtls_des_crypt_cbc( &ctx, MBEDTLS_DES_DECRYPT, nTotal, iv, pPkcs, (U8 *)pOut);
    if( ret !=0 )
    {
        free(pPkcs);
        mbedtls_des_free( &ctx );
        return -1;
    }

    nTotal -=8;
    nNumber=pOut[nTotal-1];
    *outLen=nTotal-nNumber;

    free(pPkcs);
    mbedtls_des_free( &ctx );
    return 0;

}

#else


int aes_encrypt(char* enckey,char* encbuf,char* decbuf,int inlen,int* outlen)
{

    AES_KEY aes;
//	pthread_mutex_lock(&gProtoolMutex);
#if 1
    if(ENCTYPE == 0)
    {
        char key[16]="";// = "12345678";
        memcpy(key,enckey, 16);
        char iv[16] = "";

        int nLen = inlen;//input_string.length();
        int nBei;

        if((!encbuf)||(!decbuf))
            return -1;
        nBei = nLen / AES_BLOCK_SIZE + 1;

        int nTotal = nBei * AES_BLOCK_SIZE;
        char *enc_s = (char*)malloc(nTotal);
        int nNumber;
        if (nLen % 16 > 0)
            nNumber = nTotal - nLen;
        else
            nNumber = 16;
        memset(enc_s, nNumber, nTotal);
        memcpy(enc_s, encbuf, nLen);

        if (AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0) {
            fprintf(stderr, "Unable to set encryption key in AES error !!!!!\n");
            free(enc_s);
            return -1;
        }

        AES_cbc_encrypt((unsigned char *)enc_s, (unsigned char*)decbuf, nBei * 16, &aes, (unsigned char*)iv, AES_ENCRYPT);
        //for(int i =0;i<(nBei * 16);i++)
        //	printf("%02X ", decbuf[i]&0xff);
        //printf("\n");

        * outlen = nBei * 16;
        free(enc_s);
    }
    else
#endif
    {
        memcpy(decbuf,encbuf,inlen);
        *outlen = inlen;
    }
//	pthread_mutex_unlock(&gProtoolMutex);

    return 0;
}


int aes_decrypt(char* enckey,char* encbuf,char* decbuf,int inlen,int* outlen)
{


    AES_KEY aes;
//	pthread_mutex_lock(&gProtoolMutex);
#if 1
    if(ENCTYPE ==0)
    {

	//xxxxx
        char key[16]="";// = "12345678";
        memcpy(key, enckey, 16);
        char iv[16] = "";

        int nLen = inlen;//input_string.length();
        int nBei;

        if((!encbuf)||(!decbuf))
            return -1;
        nBei = nLen / AES_BLOCK_SIZE + 1;


        int nTotal = nBei * AES_BLOCK_SIZE;
        char *enc_s = (char*)malloc(nTotal);
        int nNumber;
        if (nLen % 16 > 0)
            nNumber = nTotal - nLen;
        else
            nNumber = 16;
        memset(enc_s, nNumber, nTotal);
        memcpy(enc_s, encbuf, nLen);

        if (AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0) {
            fprintf(stderr, "Unable to set decryption key in AES error !!!!!\n");
            free(enc_s);
            return -1;
        }

        AES_cbc_encrypt((unsigned char *)enc_s, (unsigned char*)decbuf, nBei * 16, &aes, (unsigned char*)iv, AES_DECRYPT);
        //for(int i =0;i<(nBei * 16);i++)
        //	printf("%02X ", decbuf[i]&0xff);
        //printf("\n");

        * outlen = nBei * 16-16;
        free(enc_s);
    }
    else
#endif
    {
        memcpy(decbuf,encbuf,inlen);
        *outlen = inlen;
    }
//	pthread_mutex_unlock(&gProtoolMutex);

    return 0;

}

#endif

int compute_file_md5(const char *file_path, char *md5_str) {
    int i;

    int ret;
    unsigned char data[READ_DATA_SIZE];
    unsigned char md5_value[MD5_SIZE];
    MD5_CTX md5;

    FILE *fd = fopen(file_path, "rb");
    if (fd <= 0) {
        perror("fopen");
        return -1;
    }

    // init md5
    MD5Init(&md5);

    while (1) {
        // void * ptr, size_t size, size_t count, FILE * stream
        ret = fread(data, 1, READ_DATA_SIZE, fd);
        if (-1 == ret) {
            perror("fread");
            return -1;
        }

        MD5Update(&md5, data, ret);

        if (0 == ret || ret < READ_DATA_SIZE) {
            break;
        }
    }

    fclose(fd);

    MD5Final(&md5, md5_value);

    for (i = 0; i < MD5_SIZE; i++) {
        snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
    }
    md5_str[MD5_STR_LEN] = '\0'; // add end

    return 0;
}


int compute_file_len(const char *file_path, int filelen){

    FILE *fd = fopen(file_path, "w+");
    if (fd <= 0) {
        perror("fopen");
        printf("########%s:%d fopen file error...\n",__func__, __LINE__);
        return -1;
    }
    fseek(fd,0,SEEK_END);
    filelen = ftell(fd);
    printf("##############plug file len=%d\n",filelen);
    return  0;

}

int LogPlugStatus(ENUM_PLUG_LOG code) {
    char log[8];

    sprintf(log, "%03d", code);
    FILE *fp = fopen(LOG_FILE, "wb");
    if (fp <= 0)
        return -1;
    log[strlen(log)] = 0x0a;
    fwrite(log, 1, 4, fp);
    fclose(fp);
    return 0;
}
