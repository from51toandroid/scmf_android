//
// Created by 汪洋 on 2019-09-08.
//
#include <pthread.h>
#ifndef GWSDK_DEF_STRUCT_H
#define GWSDK_DEF_STRUCT_H






typedef struct
{
	//char foo,bar;

	pthread_mutex_t protool_mutex;

    int   login_status;          // 登录状态标识 0:未登录（初始化状态）；1:登录成功
    int   protool_status;        // 协议状态标识 0:初始化；1:重新登录；2:退出
    int   heart_interval;        // 心跳间隔
    long  heart_time;            // 当前心跳时间

    int   link_status;           // 连接/接入状态标识 0:未连接（初始化状态）；1:连接成功
    int   link_port;             // 连接/接入服务器端口
    char  link_server[16];       // 连接/接入服务器IP
    char  link_tocken[32];       //
    char  link_plugin_id[32];    //

    struct CONFIG_PARA
    {
        char  pin_code[64];
        char  sn_code[128];
        char  protool_key[32];
        //char  login_server[32];  // 登录服务器IP
        char  reg_server1[32];   //注册IP1
        char  reg_server2[32];   //注册IP2
        //char  login_port[16];    // 登录服务器端口
        char  reg_port[16];
    } conf; // 配置文件参数
} PRO_PARA;  
  

//__attribute__((packed))  PRO_PARA;                




#pragma pack(1)



typedef struct {
    U8 mac[6];
    char config_file[128];
}__attribute__((packed))  PLUG_CONFIG;







/* 数据包格式 = 头部 + 可选部 + 数据部
 * 头部 = 包头 + 数据头
 * * 包头 = 标识magic + 数据包类型type + 回复ack + 保留reserved
 * -- 标识magic：固定为 DDCC，长度 u16
 * -- 数据包类型type：长度 u8:5,即u8的低5位
 * -- 回复ack：长度 u8:1,即u8的第6位
 * -- 保留reserved：长度 u8:2,即u8的高2位
 *
 * * 数据头 = 可选区长度optLength + 数据校验和checksum + 包数据长度length
 * -- 可选区长度optLength：数据可选区的长度，没有则为 0，长度 u8
 * -- 数据校验和checksum：包括的内容为包数据长度length + 可选部 + 明文的数据部，长度 u8
 * -- 包数据长度length：不含头部和可选部，只有数据部，长度 u16
 *
 * 数据部 = 包序号 + 包数据
 *  -- 包序号：一个递增的整数，长度为两个字节
*/

typedef struct
{
    U16 magic;   //插件和后台软件数据包交互标识符
    U8  headp;
    // U8 type;//:5;    //低5位标识数据包类型
    // U8 ack;// :1 ;    //第6位标识数据包是否需要回复
    // U8 reserved;//:2; //高两位保留
}__attribute__((packed)) PACKAGE_HEAD;

typedef struct
{
    U8   optLength; //可选包长度
    U8   checksum;  //checksum后的所有字节总和
    U16  length;    //包数据的长度
}__attribute__((packed))  DATA_HEAD;

/// ----------------------------------------登录
typedef struct{
    U8 loginType;             //保留
    U16 reserved;             //保留
    U8 devicePin[12];         //设备pincode
}__attribute__((packed)) DATA_LOGIN_OPT;


typedef struct{
    U8 devicePin[12];         //设备的PIN码
    U8 deviceSnLength;	      //表示设备 SN 长度
    U8* deviceSN;			  //不定长数据，表示设备ID( SN)

    U8 deviceMac[6];          //设备的MAC地址
    U8 deviceId[32];          //32位设备ID
}__attribute__((packed)) DATA_LOGIN;

typedef struct {
    U16 random;	   //16 位定长随机数
} __attribute__((packed)) data_login_response_opt;

typedef struct{
    U16 code;	//错误代码，具体参考附录
}__attribute__((packed)) data_login_response_err_opt;

typedef struct{
    U8      token[32];           //32 字节定长接入 session

    U8      ip[4];               //4 字节定长 ip 地址
    U16     port;                //端口号
    U8      deviceId [32];       //32位字符插件ID
} __attribute__((packed)) data_login_response;

/// ----------------------------------------接入

typedef struct{
    U8 devicePin[12];			  //设备的PIN码
    U8 token[32];	//32 个字节的定长 token 信息
}__attribute__((packed)) data_link_opt;

typedef struct{
    U8 protocol;  //插件支持的数据协议
    U8 deviceVersion[4];			//4 字节定长设备版本号 头字节定义：1=正式版 A=Alpha版 B=Beta版
}__attribute__((packed)) data_link;


typedef struct{
    U16 code;	//错误代码，具体参考附录
}__attribute__((packed)) data_link_response_opt_err;

typedef struct{
    U16 random;	  //16 位定长随机数
}__attribute__((packed)) data_link_response_opt;


typedef struct{
    // U16 random;
    U16 heartbeat;    //服务器要求的心跳间隔时间
    U8  timezone; ;   //当前所在的时区
    U32 timestamp;    //基于 0 时区的时间秒数值
}__attribute__((packed)) data_link_response;

/// ----------------------------------------心跳

typedef struct{
    U32 timestamp;	//基于 0 时区的时间秒数值
}__attribute__((packed)) data_heartbeat_opt;


typedef struct{
    U32 timestamp;	//基于 0 时区的时间秒数值
}__attribute__((packed)) data_heartbeat_respon_opt;

/// ---------------------------------------插件业务包(上下行)
//仅用于IP设备和平台的数据交互19,20
typedef struct{
    U16 code;	//错误代码，具体参考附录
}__attribute__((packed)) data_plug_respont_opt;


/// 管理数据上行----------------------------------------------------
typedef  struct{
    U8		dataType;			//数据类型
    U16		length;             //管理数据长度
    U8		*dataByte;			//管理数据
}__attribute__((packed))  data_device_mo_req;

typedef struct{
    U16 code;	         //错误码
}__attribute__((packed)) data_device_mo_response_opt;

/// 管理数据下行----------------------------------------------------

typedef  struct{
    U8      dataType;	 //数据类型
    U16     length;                 //管理数据长度
    U8      *dataByte;           //管理数据
}__attribute__((packed)) data_device_mt_req;

typedef struct{
    U16 code;	         //错误码
}__attribute__((packed)) data_device_mt_response_opt;

typedef  struct{
    U8    dataType;	    //数据类型
    U16  length;          // 返回的数据长度
    U8   *dataByte;   // 返回的数据
}__attribute__((packed))  data_device_mt_response;

#endif //GWSDK_DEF_STRUCT_H
