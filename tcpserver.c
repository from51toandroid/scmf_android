//
// Created by 汪洋 on 2019-07-25.
//

#include "tcpserver.h"
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "common.h"
#include <sys/ioctl.h>
#include <errno.h>
//#include <netinet/tcp.h>
//#include <netinet/in.h>

extern int mQuitPro;
volatile int ServerRun;
volatile int ServerSock;


struct TCP_PARA
{
    char bind_ip[16];
    int  port;
    int  socket;
    int  quit;          // 0:未退出;1:退出
    int  connected;     //
    pthread_mutex_t mutex_tcp;
    RecCallback cb;
    void* powner;
} tcp_para;

int get_server_ip(const char *hostname, char *server_ip){
    struct hostent *hptr;
    if((hptr = gethostbyname(hostname)) == NULL)
    {
        printf("######## gethostbyname error for host:%s\n", hostname);
        return -1;
    }
    int i = 0;

    while(hptr->h_addr_list[i] != NULL)
    {
        //printf("hostname: %s\n",hptr->h_name);
        sprintf(server_ip, "%s", inet_ntoa(*(struct in_addr*)hptr->h_addr_list[i]));
        //printf("ip: %s\n", server_ip);
        break;
    }
    if (0 == strlen(server_ip)){
        printf("######## server_ip is empty.\n");
        return -2;
    }

    return 0;
};

int new_tcp_server(const char *host, int port)
{
    bzero(&tcp_para.bind_ip, sizeof (tcp_para.bind_ip));
    tcp_para.port = port;
    tcp_para.socket = 0;
    tcp_para.quit = 0;
    tcp_para.connected = 0;

    init_tcp_queue();

    char bind_ip[64] = "";
    int ret = 0;

    ret = get_server_ip(host, bind_ip);
    if (0 != ret){
        printf("######## ERROR: get_server_ip failed.\n\n");
        return ret;
    }

    snprintf(tcp_para.bind_ip, sizeof(tcp_para.bind_ip), "%s", bind_ip);

    ret = pthread_mutex_init(&tcp_para.mutex_tcp, NULL);
    if (0 != ret){
        printf("######## ERROR: pthread_mutex_init failed: errno: %d \n\n", ret);
        return ret;
    }
    return 0;
}

int init_tcp_server()
{

    int yes = 1, bufSize = 12800;//50*1024;

    struct sockaddr_in server_addr;
    socklen_t socklen = sizeof(server_addr);
    pthread_mutex_lock(&tcp_para.mutex_tcp);

    tcp_para.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_para.socket == -1) {
        tcp_para.socket = 0;
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        printf("Tcp socket() fail.");
        return -1;
    }

    printf("init_tcp_sender() - connecting to %s\n", tcp_para.bind_ip);

    if (setsockopt(tcp_para.socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&yes, sizeof(int)) == -1)
    {
        printf("init_tcp_sender(), setsockopt() - error\n");

        close(tcp_para.socket);
        tcp_para.socket = 0;
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        return -1;
    }

    if (setsockopt(tcp_para.socket, SOL_SOCKET, SO_SNDBUF, (char *)&bufSize, sizeof(int)) == -1)
    {
        printf("init_tcp_sender(), setsockopt() - error\n");
        close(tcp_para.socket);
        tcp_para.socket = 0;
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        return -1;
    }


    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(tcp_para.bind_ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(tcp_para.port);

    if(connect(tcp_para.socket, (struct sockaddr*)&server_addr, socklen) < 0)
    {
        printf("can not connect to %s:%d \n", tcp_para.bind_ip, tcp_para.port);
        close(tcp_para.socket);
        tcp_para.socket = 0;
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        return -1;
    }
    printf("init_tcp_sender() - connected to %s\n", tcp_para.bind_ip);
    U32 mode = 1;
    ioctl(tcp_para.socket, FIONBIO, &mode);

    tcp_para.connected = 1;
    pthread_mutex_unlock(&tcp_para.mutex_tcp);

    return 0;


}

int destroy_tcp_server()
{
    tcp_para.quit = 1;
    sleep(1);
    pthread_mutex_lock(&tcp_para.mutex_tcp);
    if(tcp_para.socket)
        close(tcp_para.socket);
    tcp_para.socket = 0;
    pthread_mutex_unlock(&tcp_para.mutex_tcp);
    delete_tcp_queue();
    return 0;
}

void delete_tcp_server()
{
    tcp_para.quit = 1;
    pthread_mutex_destroy(&tcp_para.mutex_tcp);
}

int running()
{
    int ret = -1;
    printf("TcpServer::running\n");

    pthread_t accept_thread_id;
    pthread_t recv_thread_id;

    ret = pthread_create( &accept_thread_id, NULL, response_thread, NULL );

    if( 0 == ret ) {
        printf("Thread #%ld has been created to accept on port [%d] \n", accept_thread_id, tcp_para.port);
    } else {
        printf("Unable to create a thread for TCP server accept on port [%d], %s \n", tcp_para.port, strerror( errno ) ) ;
        return ret;
    }

    ret = pthread_create( &recv_thread_id, NULL, recv_thread, NULL );
    if( 0 == ret ) {
        printf("Thread #%ld has been created to recv on port [%d] \n", recv_thread_id, tcp_para.port);
    } else {
        printf("Unable to create a thread for TCP server recv on port [%d], %s \n", tcp_para.port, strerror( errno ) ) ;
    }
    return ret;
}

int send_data(unsigned char *data, int size)
{
    int counter = 0;
    int counter1 = 0;
    int num_bytes, network_bytes;
    int total_bytes = 0;
    int file_bytes = size;
    unsigned char *pBuf = data;

    if(tcp_para.socket <= 0)
        return -1;

    while (file_bytes > 0)
    {
        if (file_bytes > 12800)
            network_bytes = 12800;
        else
            network_bytes = file_bytes;
        pthread_mutex_lock(&tcp_para.mutex_tcp);
        num_bytes = send(tcp_para.socket, pBuf, network_bytes, MSG_DONTWAIT);
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        if (num_bytes == -1)
        {
            if(errno == 104 || errno == 88 || errno == 32)
            {
                printf("send error no =%d", errno);
                PACKAGE_HEAD phead;
                DATA_HEAD dhead;

                tcp_para.cb(tcp_para.powner, phead, dhead, 0, 0, 0xffff);
                return -1;
            }

            if(errno != EWOULDBLOCK)
            {
                counter++;
            }
            else
            {
                counter1++;
                usleep(10);
            }
            num_bytes = 0;
            if (counter > 1000) {

                return -1;
            }
            if (counter1 > 500) {

                return 0;
            }
        }
        else
        {
            counter1 = 0;
            counter = 0;
        }

        if ( num_bytes != 0 )
        {
            file_bytes -= num_bytes;
            total_bytes += num_bytes;
            pBuf += num_bytes;
        }
    }

    return 0;
}

void * response_thread( void * arg )
{
    response_handle();
    return 0;
}

void * recv_thread( void * arg )
{
    recv_handle();
    return 0;
}




void response_handle()
{                               
    int ret;                        
    char buffer[1024];                  
    PACKAGE_HEAD phead;
    DATA_HEAD dhead;
    char  optbuff[1024];
    short seq;
      
    pthread_detach(pthread_self());                    
    while( !tcp_para.quit ){
      ret = popup(&phead, &dhead, optbuff, buffer, &seq);
      if( ret == 0 ){
          //printf("=====pop data\n");
          if( (phead.headp&0x1f) == 11 ){
              //printf("login_status or connnect ack len = %d,%d\n",dhead.length,dhead.optLength);
          }
          tcp_para.cb(tcp_para.powner, phead, dhead, optbuff, buffer, seq);
          continue;
      }else{
          sleep(1);
      }
    }
    printf( "pthread_exit at [%s]\n", __func__ );               
    pthread_exit(NULL);                        
}










void recv_handle()
{
    int rlen = 0;
    char buffer[1024];
    int size = 1022;

    pthread_detach(pthread_self());
    while (!tcp_para.quit)
    {
        pthread_mutex_lock(&tcp_para.mutex_tcp);
        rlen = recv(tcp_para.socket, buffer, size, 0);
        pthread_mutex_unlock(&tcp_para.mutex_tcp);
        if(rlen>0)
        {
            push(buffer, rlen);
        }
        else if(rlen == 0)
        {
            usleep(10000);
        }
        else if(rlen ==-1)
        {
            if(errno != EWOULDBLOCK)
            {
                PACKAGE_HEAD phead;
                DATA_HEAD dhead;
                printf("net error errno %d\n", errno);
                tcp_para.cb(tcp_para.powner, phead, dhead, 0, 0, 0xffff);
                break;
            }
            usleep(100000);
        }
        // usleep(1);

    }
    pthread_mutex_lock(&tcp_para.mutex_tcp);

    close(tcp_para.socket);
    tcp_para.socket = 0;
    pthread_mutex_unlock(&tcp_para.mutex_tcp);
    printf("########pthread_exit at [%s] %d \n\n", __func__, __LINE__);
    pthread_exit(NULL);

}

int register_cb(void *powner, RecCallback cb){
    tcp_para.cb = cb;
    tcp_para.powner = powner;
    return 0;
}