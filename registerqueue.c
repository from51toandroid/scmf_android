#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "protool.h"
#include "register.h"

int RegPush(RegQueue *pRQ, char *buff, int length)
{
	if( pRQ==NULL ) return -1;
	if( buff==NULL ) return -1;
	if( length<=0 ) return -1;
	
	pthread_mutex_lock(&pRQ->qLock);
	if( pRQ->offset+length>REG_BUFSIZE )
	{
		pthread_mutex_unlock(&pRQ->qLock);
		return -2;
	}

	memcpy(pRQ->Buffer+pRQ->offset, buff, length);
	pRQ->offset += length;
	
	pthread_mutex_unlock(&pRQ->qLock);
	return 0;
}

int RegPop(RegQueue *pRQ, HeadInfo *pHDinfo, char *optbuff, char *buff, short *seq)
{
	int i, pkgHeadSize, dataHeadSize, total;
	char tmp [8192];
	short nSeq;
	PACKAGE_HEAD *phead;
	DATA_HEAD *dhead;

	pkgHeadSize=sizeof(PACKAGE_HEAD);
	dataHeadSize=sizeof(DATA_HEAD);
	pthread_mutex_lock(&pRQ->qLock);

	if( pRQ->offset>pkgHeadSize )
	{
		for(i =0; i<pRQ->offset-2; i++)
		{
			phead= (PACKAGE_HEAD *)(pRQ->Buffer+i);
			if( phead->magic == swapInt16(DX_MAGIC_HEAD) )
			{
				if( (pRQ->offset-i-3) >= dataHeadSize )
				{
					dhead = (DATA_HEAD *)(pRQ->Buffer+i+3);
					dhead->length = swapInt16(dhead->length);

					if( (pRQ->offset-i-3-dataHeadSize) >= (dhead->length+dhead->optLength) )	
					{
						if( dhead->optLength>0 )
						{
							 memcpy(optbuff, pRQ->Buffer+i+3+dataHeadSize, dhead->optLength);
						}
						
						if(dhead->length>0)
						{
							 nSeq =*(short *)(pRQ->Buffer+i+3+dataHeadSize+dhead->optLength);
							 *seq = swapInt16(nSeq);
							 memcpy(buff, pRQ->Buffer+i+3+dataHeadSize+dhead->optLength, dhead->length);
						}

						memcpy(&pHDinfo->pkgHead, phead, pkgHeadSize);
						memcpy(&pHDinfo->dataHead, dhead, dataHeadSize);
						
						total = (dhead->length+dhead->optLength+i+3+dataHeadSize);
						pRQ->offset -=total;
						
						memcpy(tmp,pRQ->Buffer+total,pRQ->offset);
						memcpy(pRQ->Buffer,tmp,pRQ->offset);
					
						pthread_mutex_unlock(&pRQ->qLock);
						return 0;
					}
					else
					{
						pthread_mutex_unlock(&pRQ->qLock);
						return -1;
					}
				}
				else
				{
					pthread_mutex_unlock(&pRQ->qLock);
					return -1;
				}
					
			}
			
		}
	}
		
	pthread_mutex_unlock(&pRQ->qLock);
	return -1;
}

int RegQueueInit(RegQueue *pRQ, int TotalSize)
{
	int ret;
	
	if( pRQ==NULL ) return -1;
	if( TotalSize>REG_BUFSIZE ) return -1;
	if( TotalSize<100 ) return -1;

	ret=0;
	ret=pthread_mutex_init(&pRQ->qLock, NULL);
	if( ret!=0 ) return -1;

	pRQ->Buffer=(char *)calloc(TotalSize, 1);
	if( pRQ->Buffer==NULL ) 
	{
		pthread_mutex_destroy(&pRQ->qLock);
		return -1;
	}

	pRQ->offset=0;

	return 0;
}

int RegQueueDestroy(RegQueue *pRQ)
{
	if( pRQ->Buffer !=NULL )
	{
		free(pRQ->Buffer);
		pRQ->Buffer=NULL;
	}
	pthread_mutex_destroy(&pRQ->qLock);

	printf("RegQueueDestroy\n");
	return 0;
}

