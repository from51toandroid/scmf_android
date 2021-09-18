// 此文件仅供参考
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "plug.h"
#include "fwupgrade.h"
#include "cJSON.h"
#include "product.h"
#include "protool.h"
#include "Md5.h"
#include "download.h"
#include "def_define.h"

extern char FirmwareVer[128];
char mFromVersion[128];
char mToVersion[128];
U8   mMac[6];
char mFileMD5[64];
int  mFileLength;  // delete
char mDownloadUrl[128];
char mDownloadServer[32];
int  mDownloadPort;

int gw_parse_fwupgrade_cmd(char * pMsg)
{
	int len;
	cJSON *root, *pJtmp;
	char *pFrom, *pTo;
	
	if( pMsg==NULL )
	{
		printf("pMsg is null\n");
		return 9;
	}
	printf("pMsg:\n%s\n",pMsg);

	root = cJSON_Parse(pMsg);
	if( root==NULL )                                                                                         
	{
		cJSON_Delete(root);
		return 9;
	}

	pJtmp=cJSON_GetObjectItem(root, "fromVersion");
	if( pJtmp==NULL )
	{
		cJSON_Delete(root);
		return 1;
	}
	pFrom=cJSON_Print(pJtmp);
	len=strlen(pFrom)-2;
	memcpy(mFromVersion, pFrom+1, len);
	mFromVersion[len]=0;
	printf("########fromVersion : %s\n", mFromVersion);
	printf("Current version is %s\n", FirmwareVer);

	if( strcmp(FirmwareVer, mFromVersion)!=0 )
	{
		printf("Current version is %s\n", FirmwareVer);
		printf("Current version is not equal fromVersion\n");
		cJSON_Delete(root);
		free(pFrom);
		return 1;
	}

	pJtmp = cJSON_GetObjectItem(root, "toVersion");
	if( pJtmp==NULL )
	{
		cJSON_Delete(root);
		free(pFrom);
		return 2;
	}
	pTo=cJSON_Print(pJtmp);
	len=strlen(pTo)-2;
	memcpy(mToVersion, pTo+1, len);
	mToVersion[len]=0;
	printf("########toVersion : %s\n", mToVersion);
	
	cJSON_Delete(root);
	free(pFrom);
	free(pTo);
	
	return 0;
};

int gw_parse_fwupgrade_param(char * pMsg)
{
	cJSON *root, *pJtmp;
	char fromVer[128], *pFrom, *pTo, *pUrl, *pMD5;
	int len;

	if( pMsg==NULL )
	{
		printf("pMsg is null\n");
		return 9;
	}
	printf("param is \n%s\n", pMsg);
	
	root = cJSON_Parse(pMsg);
	if( root==NULL )                                                                                         
	{
		cJSON_Delete(root);
		return 9;
	}

	pJtmp = cJSON_GetObjectItem(root, "fromVersion");
	if(NULL == pJtmp)
	{
		cJSON_Delete(root);
		return 1;
	}
	pFrom=cJSON_Print(pJtmp);
	len=strlen(pFrom)-2;
	memcpy(fromVer, pFrom+1, len);
	fromVer[len]=0;
	printf("########fromVersion : %s\n", fromVer);
/*
	snprintf(currVersion, 19, "%d.%d.%d.%d", VERSION_0, VERSION_1, VERSION_2, VERSION_3);
	if( strcmp(currVersion,mFromVersion)!=0 )
	{
		printf("Current version is %s\n", currVersion);
		printf("Current version is not equal fromVersion\n");
		cJSON_Delete(root);
		free(pFrom);
		return 1;
	}
*/
	pJtmp = cJSON_GetObjectItem(root, "toVersion");
	if(NULL == pJtmp)
	{
		cJSON_Delete(root);
		free(pFrom);
		return 2;
	}
	pTo=cJSON_Print(pJtmp);
	len=strlen(pTo)-2;
	memcpy(mToVersion, pTo+1, len);
	mToVersion[len]=0;
	printf("########toVersion : %s\n", mToVersion);
    
    pJtmp = cJSON_GetObjectItem(root, "url");
    if(NULL == pJtmp)
    {
        cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
        return 3;
    }
    char mDownloadUrl2[128];
    memset(mDownloadUrl2,0,128);

	pUrl=cJSON_Print(pJtmp);
	len=strlen(pUrl)-2;
	memcpy(mDownloadUrl2, pUrl+1, len);
	mDownloadUrl2[len]=0;
	printf("########Url : %s\n", mDownloadUrl2);

    char *p1,*p2,*p3;
    int serverlen=0;
    p1 = strstr(mDownloadUrl2, "://");
    
    if(p1)
    {
        p2 = strstr(p1+3, ":");
        if(p2)
        {
            serverlen =p2-p1-3;
            p3=strstr(p2+1, "/");
            if(p3)
            {
                char port[8];
                int portlen = p3-p2-1;
                printf("########portlen %d\n",portlen);
                if(portlen<8)
                {
                    memset(port,0,8);
                    memcpy(port,p2+1,portlen);
                    mDownloadPort = atoi(port);
                }
                else
                {
                    cJSON_Delete(root);
			free(pFrom);
	 		free(pTo);
			free(pUrl);
                    return 3;
                }
                
                memcpy(mDownloadUrl,p3,strlen(p3));
            }
            else
            {
                cJSON_Delete(root);
		  	free(pFrom);
	 		free(pTo);
			free(pUrl);
                return 3;
            }
        }
        else
        {
            mDownloadPort =80;
            p3=strstr(p1+3, "/");
            if(p3)
            {
                serverlen =p3-p1-3;
                memcpy(mDownloadUrl,p3,strlen(p3));
            }
            else
            {
                cJSON_Delete(root);
		  free(pFrom);
	 	  free(pTo);
		  free(pUrl);
                return 3;
            }
        }
    }
    else
    {
        cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
	 free(pUrl);
        return 3;
    }
	
    memset(mDownloadServer,0,sizeof(mDownloadServer));
    printf("########server len  %d\n",serverlen);
    //serverlen =20;
    if(serverlen<32)
        memcpy(mDownloadServer,p1+3,serverlen);
    else
    {
        cJSON_Delete(root);
	 free(pFrom);
	 		free(pTo);
			free(pUrl);
        return 3;
    }

    printf("########port %d;mDownloadUrl : %s; mDownloadServer: %s \n", mDownloadPort,mDownloadUrl,mDownloadServer);
    pJtmp = cJSON_GetObjectItem(root, "fileLength");
    if(NULL == pJtmp)
    {
        cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
	 free(pUrl);
        return 5;
    }
    printf("########fileLength : %d\n", pJtmp->valueint);
    mFileLength =pJtmp->valueint;

    pJtmp = cJSON_GetObjectItem(root, "checkMethod");
    if(NULL == pJtmp)
    {
        cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
	 free(pUrl);
        return 4;
    }
    printf("########checkMethod : %d\n", pJtmp->valueint);
    if( pJtmp->valueint !=0 )  // not MD5
    {
    	printf("########This only support MD5.\n");
	cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
	 free(pUrl);
        return 4;
    }
	
    pJtmp = cJSON_GetObjectItem(root, "chkSum");
    if(NULL == pJtmp)
    {
        cJSON_Delete(root);
	 free(pFrom);
	 free(pTo);
	 free(pUrl);
        return 7;
    }
	pMD5=cJSON_Print(pJtmp);
	len=strlen(pMD5)-2;
	memcpy(mFileMD5, pMD5+1, len);
	mFileMD5[len]=0;
	printf("########MD5 chkSum : %s\n", mFileMD5);

	cJSON_Delete(root);
	free(pFrom);
	free(pTo);
	free(pUrl);
	free(pMD5);

    return 0;
};

int gw_parse_do_fwupgrade(void)
{
	// 此函数仅供参考
	// 请厂家自己实现固件刷新
	int ret =0;
	char senddata[128];
	data_device_mo_req gwdev_msg;

	printf("plug update url :%s\n",mDownloadUrl);
	ret = DownLoadFIle(mDownloadServer, mDownloadPort,mDownloadUrl,(char*)FW_DOWNLOAD_FILENAME);
	gwdev_msg.dataType = datatype_upgrade_process;

	if(ret ==0)
	{
		const char *file_path = FW_DOWNLOAD_FILENAME;
		char md5_str[MD5_STR_LEN + 1] = "";
		ret = compute_file_md5(file_path, md5_str);
		printf("ComputeMD5: %s\n", md5_str);
		if (strcmp(md5_str,mFileMD5)==0)
		{
			printf("update success\n");
			snprintf(senddata, 127, "{\"result\":0,\"resultDesc\":\"success\"}");
			gwdev_msg.length = strlen(senddata);
			gwdev_msg.dataByte = senddata;
			send_management_up_data((void *)&gwdev_msg, 3 + gwdev_msg.length);
			sleep(3);
			printf("########update success please exit.\n\n");
		}
		else
		{
			printf("update MD5 error\n");
			snprintf(senddata, 127, "{\"result\":7,\"resultDesc\":\"chkSum error\"}");
			gwdev_msg.length = strlen(senddata);
			gwdev_msg.dataByte = senddata;
			send_management_up_data((void *)&gwdev_msg, 3 + gwdev_msg.length);
		}

	}
	else
	{
		printf("update download failed\n");
		snprintf(senddata, 127, "{\"result\":6,\"resultDesc\":\"Download failed\"}");
		gwdev_msg.length = strlen(senddata);
		gwdev_msg.dataByte = senddata;
		send_management_up_data((void *)&gwdev_msg, 3 + gwdev_msg.length);
	}
	return 0;
}

