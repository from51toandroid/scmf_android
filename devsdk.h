//
// Created by 汪洋 on 2019-09-06.
//

#ifndef GWSDK_DEVSDK_H
#define GWSDK_DEVSDK_H
#include "common.h"

/// 获取路由器MAC地址
//   返回值：0 :OK 其他失败
U16 get_dev_mac(U8 *mac/*16进制*/);
U16 get_dev_sn(char *sn);
/// 获取固件版本
U16 get_dev_version(char * version);

int dev_version_report();


/// -------------设备---------------
// *设备数据下行*

// data：数据内容json字符串
// data_len：数据长度
///-- 备注：在不需要应答的情况下，response只需要回复下行数据中的sequence内容即可 --

// 举例：
//     下行数据data内容为：{"sequence":65535,"SongControlS":2}
//     应答数据response内容为：{"sequence":65535,"xxx1":"",......}，其中应答内容出了sequence外，根据具体情景可以包含多个参数
//     ---其中应答数据中的sequence为下行数据中的sequence的值
int dev_data_down(const char *data, int data_len);

#endif //GWSDK_DEVSDK_H
