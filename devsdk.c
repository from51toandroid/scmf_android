//Created by 汪洋 on 2019-09-06.

#include "devsdk.h"
#include "protool.h"
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "common.h"
#include <errno.h>
#include <dirent.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netutils/ifc.h>
#include <netutils/dhcp.h>


extern PLUG_CONFIG local_cfg;
char FirmwareVer[128];





U16 get_dev_mac( U8 *mac )     
{       
    #if 0
    //获取MAC的功能厂商自己实现,本函数仅供参考    
    mac[0] = 0xFF;
    mac[1] = 0xFF;
    mac[2] = 0xFF;
    mac[3] = 0xFF;
    mac[4] = 0xFF;
    mac[5] = 0x00;  
    //please use your real MAC
        
    printf("get_dev_mac MAC = ");                   
    int i;                             
    for( i=0; i<5; i++ ){
         printf("%02X:",mac[i]);
    }
	printf( "%02X\n", mac[5] );
    return 0;       
    #endif
               
    char device[16] = "eth0";         //teh0是网卡设备名
    unsigned char macaddr[ETH_ALEN];  //ETH_ALEN（6）是MAC地址长度
    //AF_INET = 1;
    int i,s;
    s = socket(AF_INET,SOCK_DGRAM,0); //建立套接口
    struct ifreq req;
    int err;
    char *ret;

    ret = strcpy(req.ifr_name,device);       //将设备名作为输入参数传入
    err = ioctl(s, SIOCGIFHWADDR, &req );    //执行取MAC地址操作
    close( s );
    if( err != -1 ){
        memcpy(macaddr, req.ifr_hwaddr.sa_data, ETH_ALEN );    //取输出的MAC地址
        printf("MAC = ");
        for( i=0; i<ETH_ALEN-1; i++ ){
             printf("%02X:", macaddr[i]);
        }
		printf( "%02X\n", macaddr[i] );
        memcpy(mac, macaddr, ETH_ALEN);
    }
    return 0;
}


#define RESULT_MAX_BUFF_SIZE  4096
int exec_cmd_and_get_result( const char *cmd_str, char *buffer ) 
{    
    int cnt;    
    FILE *pf;    
    pf = popen(cmd_str, "r");    
    cnt = fread(buffer, 1,RESULT_MAX_BUFF_SIZE, pf);    
	buffer[cnt-1] = '\0';               
	printf( "fread cnt = %d\n", cnt );    
	//printf( "strlen(buffer) = %d\n", strlen(buffer) );    
	printf( "buffer = %s\n", buffer );        
	pclose(pf);    
	return 0;
}






//getprop |grep ro.serialno

U16 get_dev_sn( char *sn )
{      
    char buffer[RESULT_MAX_BUFF_SIZE];	  
    char cmd_str[RESULT_MAX_BUFF_SIZE];  
                                                  
    //获取SN号的功能厂商自己实现,本函数仅供参考
    char sndata[128] = "123455432112339";  
    memcpy( &sn[0], sndata, strlen(sndata) );
                  
    strcpy( cmd_str, "getprop |grep ro.serialno |awk '{print $2}' ");  
    exec_cmd_and_get_result(cmd_str, buffer);
    memcpy( &sn[0], &buffer[1], 32 );
    return 0;
};

















U16 get_dev_version(char * version)
{
    //获取固件版本信息的功能厂商自己实现，本函数仅供参考
    char buffer[RESULT_MAX_BUFF_SIZE];	  
    char cmd_str[RESULT_MAX_BUFF_SIZE];

    if (version == NULL){
        return -1;
    }
    char test_version[256] = "V0.0.1";
    sprintf(version, "%s", test_version);

	//[ro.build.version.incremental]: [180101512.1006000000.10010000024]

	
	strcpy( cmd_str, "getprop |grep ro.build.version.incremental |awk '{print $2}' ");  
	exec_cmd_and_get_result(cmd_str, buffer);
    memcpy( &version[0], &buffer[1], 32 );



	

	
    return 0;
}





//发送固件版本信息
int dev_version_report()
{                                  
    int ret = 0;                          
    data_device_mt_req report_msg;    
    report_msg.dataType = datatype_version_report;    
    cJSON * pJsonRoot = NULL;
    pJsonRoot = cJSON_CreateObject();
	
    if( NULL == pJsonRoot ){
        //error happend here
        return -1;
    }
                                                      
    memset( FirmwareVer, 0, sizeof(FirmwareVer) );                   
    ret = get_dev_version(FirmwareVer);                     
                 
    cJSON_AddStringToObject(pJsonRoot, "version", FirmwareVer);
    cJSON_AddStringToObject(pJsonRoot, "sdkVersion", SDKVERSION);
    char * pJson = cJSON_Print(pJsonRoot);
    if( NULL == pJson ){
        cJSON_Delete(pJsonRoot);
        return -2;
    }
    report_msg.dataByte = pJson;
    report_msg.length   = strlen(report_msg.dataByte);

	//macdbg_dmphex(report_msg.dataByte, report_msg.length);
  
	
    printf( "%s: len = [%d].\n pJson = %s\n", __func__, report_msg.length, report_msg.dataByte);
    send_management_up_data((void *)&report_msg, sizeof(report_msg)+report_msg.length);
    cJSON_Delete(pJsonRoot);
	free(pJson);
    //原来的版本缺少上面一句内存释放代码 by dp_mcu
    return 0;            
}












/*int dev_fwversion_report()
{
    int ret = 0;

    char dev_version[256] = {0};

    data_device_mt_req report_msg;

    report_msg.dataType = 14;

    cJSON * pJsonRoot = NULL;

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot)
    {
        //error happend here
        return -1;
    }

//	ret = get_dev_version(dev_version);
    memset(dev_version,0,sizeof(dev_version));
    memcpy(dev_version,"FW-V0.0.2",9);

    cJSON_AddStringToObject(pJsonRoot, "version", dev_version);

    char * pJson = cJSON_Print(pJsonRoot);
    if(NULL == pJson)
    {
        cJSON_Delete(pJsonRoot);
        return -2;
    }
    report_msg.dataByte = pJson;
    report_msg.length = strlen(report_msg.dataByte);

    printf("######## %s %d: len = [%d] pJson = \n%s\n\n", __func__, __LINE__, report_msg.length, report_msg.dataByte);

    send_management_up_data((void *)&report_msg, sizeof(report_msg)+report_msg.length);

    cJSON_Delete(pJsonRoot);

    return 0;
}*/

/// -------------设备---------------
// *设备数据下行*

// data：数据内容json字符串
// data_len：数据长度
///-- 备注：在不需要应答的情况下，response只需要回复下行数据中的sequence内容即可 --

// 举例：
//     下行数据data内容为：{"sequence":65535,"SongControlS":2}
//     应答数据response内容为：{"sequence":65535,"xxx1":"",......}，其中应答内容出了sequence外，根据具体情景可以包含多个参数
//     ---其中应答数据中的sequence为下行数据中的sequence的值
int dev_data_down(const char *data, int data_len){
    printf("######## data=%s len=%d \n", data, data_len);
    printf("######## %s OK. \n", __func__);
    int ret = -1;

    if(NULL == data)
    {

        return ret;
    }
    /// 可用一下方法处理数据，也可自行处理数据，供参考
    /*cJSON * pJsonData = NULL;
    cJSON * pJsonRes = NULL;
    pJsonData = cJSON_Parse(data);
    pJsonRes = cJSON_CreateObject(); //cJSON_Parse("{\"sequence\":0}");

    for(int i=0; i<cJSON_GetArraySize(pJsonData); i++)   //遍历最外层json键值对
    {
        cJSON * item = cJSON_GetArrayItem(pJsonData, i);

        *//* -- 打印json键值对 --
        if(cJSON_String == item->type)
        {
            printf("%s->", item->string);
            printf("%s\n", item->valuestring);
        }
        else if(cJSON_Number == item->type)
        {
            printf("%s->", item->string);
            printf("%s\n", item->valueint);
        }*//*

        if(!strcasecmp("sequence",item->string))
        {
            cJSON_AddNumberToObject(pJsonRes, item->string, item->valueint);
            ret = 0;
        }
        else if(!strcasecmp("xxx",item->string)) // example
        {
            //...

        }
        //...
    }

    /// 应答数据的添加举例
    // 值为字符串的处理方式
    //    cJSON_AddStringToObject(pJsonRes, string, string);
    // 值为数值的处理方式
    //    cJSON_AddNumberToObject(pJsonRes, string, int);


    if(-1 == ret)
    {
        cJSON_AddStringToObject(pJsonRes, "msg", "parser error");
    }

    char * pJson = cJSON_PrintUnformatted(pJsonRes);
    *response_len = strlen(pJson);
    memcpy(response, pJson, *response_len);

    cJSON_Delete(pJsonRes);
    cJSON_Delete(pJsonData);*/

    /*char response[1024*4] = "";
    int  responselen = 0;
    memcpy(response, data, data_len);
    responselen = data_len;
    send_business_ack_data(0, (unsigned char*) &response, responselen);
    send_business_up_data((unsigned char*) &response, responselen);*/

    return ret;
}
