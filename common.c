//
// Created by 汪洋 on 2019-07-25.
//

#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



extern PLUG_CONFIG local_cfg;


char end_type;
int get_end_type()
{
    union w
    {
        int a;
        char b;
    }c;
    c.a = 1;
    if (c.b == 1)
    {
        //printf("xiaoduan\n");
        end_type = LITTLE_END;
    }
    else
    {
        printf("daduan\n");
        end_type= BIG_END;

    }
    return 0;
}
U16 swapInt16(U16 value)
{
    if(end_type == LITTLE_END)
        return value;

    return ((value &0x00FF)<<8)|
           ((value&0xFF00)>>8);
}

U32 swapInt32(U32 value)
{
    if(end_type == LITTLE_END)
        return value;
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)|
           ((value & 0x00FF0000) >> 8)|
           ((value&0xFF000000) >> 24);
}

U64 swapInt64(U64 value)
{
    if(end_type == LITTLE_END)
        return value;

    return ((value & 0x00000000000000FF) << 56)
          |((value & 0x000000000000FF00) << 40)
          |((value & 0x0000000000FF0000) << 24)
          |((value & 0x00000000FF000000) << 8)
          |((value & 0x000000FF00000000) >> 8)
          |((value & 0x0000FF0000000000) >> 24)
          |((value & 0x00FF000000000000) >> 40)
          |((value & 0xFF00000000000000) >> 56);
}





char *p_file_path = NULL;    //程序配置文件路径

void new_program_profile( const char *file_path )   
{     
    int len;        

	printf( "local_cfg.config_file = %s\n", local_cfg.config_file );
    if( strlen(local_cfg.config_file) > 0 ){  
        len = strlen(local_cfg.config_file);
        p_file_path = (char *)malloc(len + 1);
        strcpy(p_file_path, local_cfg.config_file);
    }else{
        len = strlen(file_path);
        p_file_path = (char *)malloc(len + 1);
        strcpy(p_file_path, file_path);
    }
    p_file_path[len] = '\0';
}















void delete_program_profile()
{
    printf( "del (\"%s\").\n", p_file_path );  
    if( p_file_path ){                
        free(p_file_path);                 
    }                             
    p_file_path = NULL;                   
}            








int get_key_value_str(const char *app, const char *key, const char *default_str, char *ret_str, unsigned int ret_str_size)
{
    /// 读取配置文件
    FILE *file = fopen(p_file_path, "r");
    /// 如果为空，则使用默认值
    if (file == NULL) {

        printf("Can not fopen(\"%s\").\n", p_file_path);
        if (default_str == NULL) {
            printf("Not find Key=\"%s\" and no default_str to use.", key);
            return -1;
        }
        int len = strlen(default_str);
        len = ((unsigned int)len < (ret_str_size - 1) ? len : (ret_str_size - 1));
        strncpy(ret_str, default_str, len);
        ret_str[len] = '\0';

        return -1;
    }
    char buffer[512], *pstr;
    char app_found = FALSE;
    char use_default = TRUE;
    while (fgets(buffer, sizeof(buffer), file)) {
        pstr = buffer;
        while (*pstr == '\t' || *pstr == ' ')
            pstr ++;
        if (*pstr == '#') {
            continue;
        } else if (strstr(buffer, "[") != NULL) {
            if (app_found)
                break;
            else if (strstr(buffer, app) != NULL)
                app_found = TRUE;
        } else {
            if (!app_found)
                continue;
            if (strstr(buffer, key) == NULL)
                continue;
            pstr = strstr(buffer, "=");
            if (pstr == NULL)
                break;
            pstr ++;
            while (*pstr == ' ')
                pstr ++;

            unsigned int len = strlen(buffer);
            do {
                len --;
            } while (buffer[len] == '\r' || buffer[len] == '\n'
                     || buffer[len] == '\t' || buffer[len] == ' ');
            len ++;

            if (pstr < buffer + len) {
                len = len - (pstr - buffer);
                len = (len < (ret_str_size - 1) ? len : (ret_str_size - (unsigned int)1));
                strncpy(ret_str, pstr, len);
                ret_str[len] = '\0';
                use_default = FALSE;
            }
        }
    }
    if (use_default) {
        if (default_str == NULL) {
            printf("Not find Key=\"%s\" and no default_str to use.", key);
            fclose(file);
            return -1;
        }
        int len = strlen(default_str);
        len = ((unsigned int)len < (ret_str_size - 1) ? len : (ret_str_size - 1));
        strncpy(ret_str, default_str, len);
        ret_str[len] = '\0';
    }
    //SPrint("ret_str=[%s]", ret_str);
    fclose(file);
    return 0;
}