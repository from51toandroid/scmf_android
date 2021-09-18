//
// Created by 汪洋 on 2019-07-25.
//

#ifndef GWSDK_COMMON_H
#define GWSDK_COMMON_H

#include <unistd.h>
#include "def_define.h"
#include "def_enum.h"
#include "def_struct.h"




U16 swapInt16(U16 value);

U32 swapInt32(U32 value);

U64 swapInt64(U64 value);


void new_program_profile(const char *file_path);

void delete_program_profile();

int get_end_type();

int get_key_value_str(const char *app, const char *key, const char *default_str, char *ret_str, unsigned int ret_str_size);

#endif //GWSDK_COMMON_H
