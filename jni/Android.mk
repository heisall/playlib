# Generated by Neo

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := network
LOCAL_SRC_FILES := libs/$(APP_ABI)/lib_100.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := libs/$(APP_ABI)/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := libs/$(APP_ABI)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jsoncpp
LOCAL_SRC_FILES := libs/$(APP_ABI)/libjsoncpp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := auth_base64
LOCAL_SRC_FILES := libs/$(APP_ABI)/libauth_base64.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := aenc
LOCAL_SRC_FILES := libs/$(APP_ABI)/libJAENC.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := adec
LOCAL_SRC_FILES := libs/$(APP_ABI)/libJADEC.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := legacy
LOCAL_SRC_FILES := libs/$(APP_ABI)/libJDEC04.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := dec
LOCAL_SRC_FILES := libs/$(APP_ABI)/libdec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := hdec
LOCAL_SRC_FILES := libs/$(APP_ABI)/libhdec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opengl
LOCAL_SRC_FILES := libs/$(APP_ABI)/libopengl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := screenshot
LOCAL_SRC_FILES := libs/$(APP_ABI)/libscreenshot.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mp4
LOCAL_SRC_FILES := libs/$(APP_ABI)/libmp4.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := curl
LOCAL_SRC_FILES := libs/$(APP_ABI)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cloudstore
LOCAL_SRC_FILES := libs/$(APP_ABI)/libcloudstores.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := iconv
LOCAL_SRC_FILES := libs/$(APP_ABI)/libiconv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := alu
LOCAL_SRC_FILES := libs/$(APP_ABI)/libalu.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := play
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := utils/commons.cc utils/voiceenc.c utils/char_conv.cc \
	utils/threads.cc utils/callbacks.cc utils/playmp4.cc utils/playhls.cc play.cc 
LOCAL_CFLAGS := -Wall -Wno-write-strings
LOCAL_LDLIBS := -llog -lGLESv2 -lEGL -landroid
LOCAL_SHARED_LIBRARIES := alu
LOCAL_STATIC_LIBRARIES := auth_base64 curl cloudstore jsoncpp iconv network screenshot opengl mp4 aenc adec legacy hdec dec avcodec avutil

include $(BUILD_SHARED_LIBRARY)
