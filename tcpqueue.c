//
// Created by 汪洋 on 2019-08-22.
//

#include "tcpqueue.h"
#include <pthread.h>
#include "common.h"
#include <string.h>
#include <stdio.h>

struct QUEUE_PARA
{
    char data[1024*10];
    int  offset;
    pthread_mutex_t mutex_que;
} queue_para;

int init_tcp_queue()
{
    pthread_mutex_init(&queue_para.mutex_que, NULL);
    bzero(&queue_para.data, sizeof (queue_para.data));
    queue_para.offset = 0;
    return 0;
}
void delete_tcp_queue()
{
    pthread_mutex_destroy(&queue_para.mutex_que);
    bzero(&queue_para.data, sizeof (queue_para.data));
    queue_para.offset = 0;
}


int push(char *buff, int length)
{

    pthread_mutex_lock(&queue_para.mutex_que);
    if((queue_para.offset + length) > 10*1024)
    {
        printf("over flow\n");
        pthread_mutex_unlock(&queue_para.mutex_que);
        return -1;
    }
    memcpy(queue_para.data + queue_para.offset, buff, length);
    queue_para.offset += length;

    pthread_mutex_unlock(&queue_para.mutex_que);
    return 0;
}

int popup(PACKAGE_HEAD* phead, DATA_HEAD *dhead, char* optbuff, char *buff, short* seq)
{
    int i =0;
    pthread_mutex_lock(&queue_para.mutex_que);

    if((int)(queue_para.offset)>(int)(sizeof(PACKAGE_HEAD)))
    {
        for( i =0;i<queue_para.offset-2;i++)
        {
            *phead= *(PACKAGE_HEAD*)(queue_para.data+i);

            if((*phead).magic ==swapInt16(DX_MAGIC_HEAD))
            {
                //printf("found head\n");
                if((int)(queue_para.offset-i-3)>=(int)(sizeof(DATA_HEAD)))
                {

                    *dhead = *(DATA_HEAD*)(queue_para.data+i+3);
                    (*dhead).length = swapInt16((*dhead).length);

                    //printf("DATA_HEAD====%d,%d\n",(mOffset-i-3-sizeof(DATA_HEAD)),(*dhead).length+(*dhead).optLength+2);
                    int tmp =0;

                    if((int)(queue_para.offset-i-3-sizeof(DATA_HEAD))>=(int)((*dhead).length+(*dhead).optLength+tmp))
                    {
                        if((*dhead).optLength>0)
                        {
                            memcpy(optbuff,queue_para.data+i+3+sizeof(DATA_HEAD),(*dhead).optLength);
                        }
                        if((*dhead).length>0)
                        {
                            *seq =*(short*)(queue_para.data+i+3+sizeof(DATA_HEAD)+(*dhead).optLength);
                            *seq = swapInt16(*seq);
                            memcpy(buff , queue_para.data+i+3+sizeof(DATA_HEAD)+(*dhead).optLength,(*dhead).length);

                            // for(int j =i;j<(i+3+sizeof(DATA_HEAD)+(*dhead).optLength+(*dhead).length);j++)
                            //	printf("%02x ",mData[j]&0xff);
                            // printf("\n");
                        }
                        int tmplen = ((*dhead).length+(*dhead).optLength+tmp+i+3+sizeof(DATA_HEAD));
                        queue_para.offset -=tmplen;

                        char tmp [1024];
                        memcpy(tmp,queue_para.data+tmplen,queue_para.offset);
                        memcpy(queue_para.data,tmp,queue_para.offset);
                        //	for(int j =0;j<mOffset;j++)
                        //		 	printf("%02x ",mData[j]&0xff);
                        //		 printf("\n");
                        pthread_mutex_unlock(&queue_para.mutex_que);
                        return 0;
                    }
                    else
                    {
                        pthread_mutex_unlock(&queue_para.mutex_que);
                        return -1;
                    }
                }
                else
                {
                    pthread_mutex_unlock(&queue_para.mutex_que);
                    return -1;
                }

            }

        }
    }

    pthread_mutex_unlock(&queue_para.mutex_que);
    return -1;
}