//
// Created by 汪洋 on 2019-08-22.
//

#ifndef GWSDK_TCPQUEUE_H
#define GWSDK_TCPQUEUE_H

#include "common.h"

int init_tcp_queue();
void delete_tcp_queue();

int popup(PACKAGE_HEAD* phead,DATA_HEAD *dhead,char* optbuff,char *buff,short* seq);
int push(char *buff, int length);

#endif //GWSDK_TCPQUEUE_H
