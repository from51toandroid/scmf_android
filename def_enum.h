//
// Created by 汪洋 on 2019-09-08.
//

#ifndef GWSDK_DEF_ENUM_H
#define GWSDK_DEF_ENUM_H
typedef enum
{
    reserved=0,
    heart =1,
    heart_ack=2,
    force_logout=3,
    subscribe=4,
    dissub=5,
    dev_found=6,
    dev_apply=7,
    dev_auth=8,
    login_conform=9,
    link_conform=10,
    ack = 11, 
    dev_divorce=12,
    notiry_dev_update=13,
    fireware_update=14,
    fireware_content=15,
    notify_dev_disconnet=16,
    management_pack_up=17,  //管理层上行数据
    management_pack_down=18,//管理层下行数据
    bissness_pack_up=19,    //业务层上行数据
    bissness_pack_down=20,  //业务层下行数据
    user_logout=21,
    management_pack_ack=25, //管理层上下行数据回复包
    bissness_pack_ack=26,   //业务层上下行数据回复包
}PACKAGE_TYPE;

typedef enum
{
    checksum_err=11,
    token_invalid=12,//token 失效，重新登录
    device_sn_invalid=13,//IP，路由器设备不合法
    device_id_err=14,//未分配到设备直连分配的id
    token_get_err=15, //未分配到token
    plug_id_err=16,   //未分配到plug分配的id
    no_access_gateway=17,//无可用的接入网关，登录时返回
}ERROR_NO;

typedef enum
{

    errid_regist=101,
    errid_login=102,//token 失效，重新登录
    errid_heart=103,
    errid_forceout=104

}ERROR_ID;

typedef enum
{
    log_init=1,
    log_login=2,
    log_link=3,
    log_update=4,
    log_forcequit=5,
    log_init_err=101,
    log_login_err=102,
    log_link_err=103,
    log_update_err=104
}ENUM_PLUG_LOG;

typedef enum
{
    datatype_plug_upgrade_ack=1,
    datatype_upgrade_cmd=3,//-fw升级相关
    datatype_upgrade_param=4,
    datatype_upgrade_process=5,
    datatype_version_report=6,
    datatype_plug_update_cmd=9,  //  plug 升级相关
    datatype_plug_update_file=10,
    datatype_plug_update_process=11
}MANAGEMENT_DATATYPE;

#endif //GWSDK_DEF_ENUM_H
