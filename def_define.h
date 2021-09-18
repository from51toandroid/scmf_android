//
// Created by 汪洋 on 2019-09-08.
//

#ifndef GWSDK_DEF_DEFINE_H
#define GWSDK_DEF_DEFINE_H







#define BIG_END 1
#define LITTLE_END 0

#define ENCTYPE 2    //不加密
//默认不加密 拿到代码就是这样的




#define FALSE 0
#define TRUE 1

#define DX_MAGIC_HEAD 0xddcc

#define SDKVERSION "2.0.0.0"
#define APP_DEV "Dev"
#define APP_DEV_PIN "PIN"

#define APP_PORTOOL "Protool"
#define APP_PORTOOL_KEY "KEY"
#define APP_PORTOOL_SERVER1 "SERVER1"
#define APP_PORTOOL_SERVER2 "SERVER2"
#define APP_PORTOOL_PORT "PORT"

#define KEY_PIN APP_DEV_PIN
#define KEY_KEY APP_PORTOOL_KEY
#define KEY_SERVER1 APP_PORTOOL_SERVER1
#define KEY_SERVER2 APP_PORTOOL_SERVER2
#define KEY_PORT APP_PORTOOL_PORT

#define DEF_EMPTY       ""
#define DEF_SERVER_IP   "171.208.222.113" // 测试环境IP  
#define DEF_SERVER_PORT "6037"            // 测试环境PORT


#define PACKAGE_HEAD_LENGTH 3
#define DATA_HEAD_LENGTH 4
#define PACKAGE_SEQ_LENGTH 2
#define PACKAGE_DATA_LENGTH 2
#define LOGIN_OPT_LENGTH 15
#define LINK_OPT_LENGTH 44
#define ACK_YES 1
#define ACK_NO 0

typedef unsigned char   U8;
typedef unsigned char   u8;

typedef unsigned short  U16;
typedef unsigned int    U32;
typedef unsigned long long    U64;



#endif //GWSDK_DEF_DEFINE_H
