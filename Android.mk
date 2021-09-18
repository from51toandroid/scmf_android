# Copyright 2011 The Android Open Source Project

ifneq ($(BUILD_TINY_ANDROID),true)

#$(warning debug info BUILD_TINY_ANDROID = $(BUILD_TINY_ANDROID) ) 
#$(error debug info then exit)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := plug.c plugsdk.c protool.c common.c tcpserver.c register.c devsdk.c \
fwupgrade.c plugupgrade.c Md5.c cJSON.c tcpqueue.c registertcpserver.c registerqueue.c \
download.c 

LOCAL_MODULE := scmf
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
#LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
#LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_C_INCLUDES := bootable/recovery external/openssl/include 
LOCAL_STATIC_LIBRARIES := libminui libpixelflinger_static libpng
LOCAL_STATIC_LIBRARIES += libz libstdc++ libcutils liblog libm libc

include $(BUILD_EXECUTABLE)

#$(warning BUILD_EXECUTABLE = $(BUILD_EXECUTABLE) ) 
#$(error debug info then exit)

endif
