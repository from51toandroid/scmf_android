//
// Created by 汪洋 on 2019-07-25.
//

#ifndef GWSDK_PROTOOL_H
#define GWSDK_PROTOOL_H

#include "common.h"

int get_link_status();
void new_protool();
void delete_protool();
int init_protool();
int destroy_protool();

int heart_beep();

int login_service();
int link_service();

void* login_thread(void *arg);
void  login_handle();

void* check_thread(void *arg);
void  check_handle();


int rec_cb(void *pOwner, PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq);
int cb_handle(PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq);

int send_up_data(unsigned char *optdata, unsigned char *data, PACKAGE_TYPE type, int isack, int optlen, int len, U8 checks);

int send_business_up_data(void *data, int len);
int send_management_up_data(void *data, int len);

int send_business_ack_data(short errcode, unsigned char *buff, int len);
int send_management_ack_data(short errcode, unsigned char *buff, int len);
#ifdef DES_SUPPORT
int des_encrypt(char *pKey, char *pIn, char *pOut, int inLen, int *outLen);
int des_decrypt(char *pKey, char *pIn, char *pOut, int inLen, int *outLen);
#else
int aes_decrypt(char* key, char* in, char* out, int inlen, int* outlen);
int aes_encrypt(char* enckey, char* encbuf, char* decbuf, int inlen, int* outlen);
#endif
int compute_file_md5(const char *file_path, char *md5_str);
int compute_file_len(const char *file_path, int filelen);
int LogPlugStatus(ENUM_PLUG_LOG code);

#endif //GWSDK_PROTOOL_H
typedef struct{
    U8 loginType;             //保留
    U16 reserved;             //保留
    //U8  len;/
    U8* devicePin;//[];            //路由ID( SN),不定长  ???
}__attribute__((packed)) data_regist_opt;

typedef struct{
    U8  devicePin[12];
    U8  deviceSnLength;	    //表示路由 SN 长度
    U8*  deviceSn;	          //不定长数据，表示路由ID( SN)
    U8  deviceMac[6];
    //U8 version[4];	          //表示路由插件版本
    U8 pluginId[32];          //32位字符插件授权ID,首次为空
    //U8 pluginIdVersion [4];   //插件版本号
}__attribute__((packed)) data_regist;

//如何区分正确失败？？？
typedef struct {
    U16 random;	   //16 位定长随机数
} __attribute__((packed)) data_regist_response_opt;

typedef struct{
    U16 code;	//错误代码，具体参考附录
}__attribute__((packed)) data_regist_response_err_opt;

typedef struct{
    // U16     random;        //16 位定长随机数
    U8      token[32];      //32 字节定长登录 session
    //唯一标识一次登录过程
    U8      ip[4];               //4 字节定长 ip 地址
    U16     port;                //端口号
    U8      pluginId [32];    //32位字符插件ID
} __attribute__((packed)) data_regist_response;