//
// Created by 汪洋 on 2019-07-24.
//

#include "plugsdk.h"
#include "protool.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "devsdk.h"

int init_plug_sdk()
{
    //new_protool();
    int ret = init_protool();
    return ret;
}

int destroy_plug_sdk()
{
    destroy_protool();
    delete_protool();
    return 0;
}

void plug_sdk_test()
{

}