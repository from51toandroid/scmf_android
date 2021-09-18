#include <stdio.h>
#include "plug.h"
#include "plugsdk.h"
#include <unistd.h>
#include "common.h"
#include <string.h>
#include <stdlib.h>

PLUG_CONFIG local_cfg = {0};
PRO_PARA pro_para = {0};



int config_line_proc( PLUG_CONFIG *cfg, int argc, char *argv[] )
{    
    int i;

	//printf("argc = %d\n", argc);
    for( i=1; i<argc; i++ ){
         if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mac")){
            if(i==argc-1){
                fprintf(stderr, "########Error: -m argument given but no mac specified.\n\n");
                return -1;
            }
            else if(strlen(argv[i+1]) != 12){
                fprintf(stderr, "########Error: mac format must be \"aabbccddeeff\".\n\n");
                return -2;
            }else{
                sscanf(argv[i+1], "%02x%02x%02x%02x%02x%02x",
                       (unsigned int*)&cfg->mac[0],
                       (unsigned int*)&cfg->mac[1],
                       (unsigned int*)&cfg->mac[2],
                       (unsigned int*)&cfg->mac[3],
                       (unsigned int*)&cfg->mac[4],
                       (unsigned int*)&cfg->mac[5]);
            }
            i++;
        }
        else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")){
            if(i==argc-1){
                fprintf(stderr, "########Error: -f argument given but no file specified.\n\n");
                return -3;
            }else{
                if((access(strdup(argv[i+1]),F_OK))!=-1)
                {
                    printf("file %s exist.\n", strdup(argv[i+1]));
                    sprintf(cfg->config_file, "%s", strdup(argv[i+1]));
                }
                else
                {
                    printf("file %s not exist.\n", strdup(argv[i+1]));
                }
            }
            i++;
        }else{
            goto unknown_option;
        }
    }
    return 0;
    unknown_option:
    fprintf(stderr, "Error: Unknown option '%s'.\n",argv[i]);
    return 1;
}




 
#define MONTH_PER_YEAR   12   // 一年12月
#define YEAR_MONTH_DAY   20   // 年月日缓存大小
#define HOUR_MINUTES_SEC 20   // 时分秒缓存大小
 
void GetCompileTime( void )
{   
    const char year_month[MONTH_PER_YEAR][4] =
        { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char compile_date[YEAR_MONTH_DAY] = {0}, compile_time[HOUR_MINUTES_SEC] = {0}, i;
    char str_month[4] = {0};
    int year, month, day, hour, minutes, seconds;
     
    sprintf(compile_date, "%s", __DATE__);  //"Aug 23 2016"
    sprintf(compile_time, "%s", __TIME__);  //"10:59:19"
    sscanf(compile_date, "%s %d %d", str_month, &day, &year);
    sscanf(compile_time, "%d:%d:%d", &hour, &minutes, &seconds);
    for( i=0; i<MONTH_PER_YEAR; ++i ){
         if( strncmp(str_month, year_month[i], 3) == 0 ){
             month = i + 1;
             break;
         }
    }
    printf("Compile time is = %d-%d-%d %d:%d:%d\n", year, month, day, hour, minutes, seconds);
}



#include <cutils/klog.h>
#include <cutils/list.h>
#include <cutils/misc.h>
#include <cutils/uevent.h>


#define LOGE(x...) do { KLOG_ERROR("scmf", x); } while (0)
#define LOGI(x...) do { KLOG_INFO("scmf", x); } while (0)
#define LOGV(x...) do { KLOG_DEBUG("scmf", x); } while (0)


int main( int argc, char * argv[] )
{     
    printf( "IpDev Version = %d.%d.%d.%d-%s\n", VERSION_0, VERSION_1, VERSION_2, VERSION_3, RELEASETIME ); 

	time_t start = time(NULL);
	printf( "Starting scmf on %s", ctime(&start) );
	GetCompileTime();
	printf("argc = %d\n", argc);


	LOGE("\n");
    LOGE("*************** debug scmf ***************\n");
    LOGE("\n");


	
             
    PLUG_CONFIG *p_cfg = &local_cfg;
    if( config_line_proc(p_cfg, argc, argv) != 0x00 ){  
        fprintf(stderr, "Error wrong param(s).\n");              
        return -1;
    }
    //初始化插件
    if( 0 != init_plug_sdk() ){
        //printf("gNetDump init error\n");
        destroy_plug_sdk();
		printf("exit at [%s].\n", __func__ );
        return -1;
    }
        
    #ifdef DEBUG
    plug_sdk_test();
    return 0;
    #else
    while( 1 ){
      //printf("loop on here.\n");
      sleep(10);
    }
    #endif
}












