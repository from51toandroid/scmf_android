#ifndef __download_h__
#define __download_h__
#include   <stdio.h> 

#include   <stdlib.h> 

#include   <string.h> 

#include   <sys/types.h> 

#include   <sys/socket.h> 

#include   <errno.h> 

#include   <unistd.h> 

#include   <netinet/in.h> 

#include   <limits.h> 

#include   <netdb.h> 

#include   <arpa/inet.h> 

#include   <ctype.h> 

//#include <string>

#if _MSC_VER > 1000

#pragma once

#endif 

//class  CHttpSocket  

//{

//public:
typedef char bool;
#define true 1
#define false 0
    int GetServerState();

	int GetField(const char* szSession,char *szValue,int nMaxLength);

	int GetResponseLine(char *pLine,int nMaxLength);

	char* GetResponseHeader(int *Length);

	char *FormatRequestHeader(const char *pServer,const char *pObject,long *Length,

	char* pCookie,char *pReferer,

	long nFrom,long nTo,

	int nServerType);

	int GetRequestHeader(char *pHeader,int nMaxLength) ;

	bool SendRequest(const char* pRequestHeader,long Length);


	void NewCHttpSocket();

	void DeleteCHttpSocket();

	bool SetTimeout(int nTime,int nType);

	long ReceiveHttp(char* pBuffer,long nMaxLength);

	bool ConnectHttp(const char* szHostName,int nPort);

	bool SocketHttp();

	bool CloseSocket();

//	protected:

	int DownLoadFIle(char* serverName, int port,char* URL,char* localDirectory);

//};
#endif

