
#ifndef __REGISTER_H__
#define __REGISTER_H__

#define REG_BUFSIZE 4096

typedef struct _REG_QUEUE_
{
	char *Buffer;
	int offset;
	pthread_mutex_t qLock;
}RegQueue;

typedef struct __REGISTER_OBJ_
{
	int sock;
	int ServerRun;
	int ServerState;
	int RegisterOk;
	char host[64];
	U16 port;
	RegQueue *pRQ;
	pthread_t RecvPid;
	pthread_t HandlePid;

	char mLoginServer[16];
	char mLoginTocken[32];
	char mLoginPluginId[32];
	short mLoginPort;
} RegisterObj;

typedef struct __HEAD_INFO_
{
	PACKAGE_HEAD pkgHead;
	DATA_HEAD dataHead;;
}HeadInfo;

int SendRegData(int sock, char *data, int size);
int RegPush(RegQueue *pRQ, char *buff, int length);
int RegPop(RegQueue *pRQ, HeadInfo *pHDinfo, char *optbuff, char *buff, short *seq);
int RegQueueInit(RegQueue *pRQ, int TotalSize);
int RegQueueDestroy(RegQueue *pRQ);
int Reg_ServerInit(RegisterObj *pRegObj);
int Reg_DeleteServer(RegisterObj *pRegObj);
int RegisterStart();
int RegisterAgain(char *pServerAddr);

#endif  // __REGISTER_H__

