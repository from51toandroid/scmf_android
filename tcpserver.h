//
// Created by 汪洋 on 2019-07-25.
//

#ifndef GWSDK_TCPSERVER_H
#define GWSDK_TCPSERVER_H
#include "tcpqueue.h"
#include "common.h"

int  new_tcp_server(const char *bindIp, int port);
int  init_tcp_server();
int get_server_ip(const char *hostname, char *server_ip);
int  destroy_tcp_server();
void delete_tcp_server();

int  running();
int  send_data(unsigned char *data, int size);
void * response_thread( void * arg );
void * recv_thread( void * arg );
void response_handle();
void recv_handle();


// 接收数据回调处理
typedef int (*RecCallback)(void* pOwner, PACKAGE_HEAD phead, DATA_HEAD dhead, char* optbuff, char *buff, short seq);

int  register_cb(void *powner, RecCallback cb);

#endif //GWSDK_TCPSERVER_H
