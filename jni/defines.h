// Generated by Neo

#ifndef _JNI_DEFINES_H
#define _JNI_DEFINES_H

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <android/log.h>
#include <android/native_window_jni.h>

#include <JADEC.h>
#include <JAENC.h>
#include <JHDEC.h>
#include <JDEC05.h>
#include <JDEC04.h>
#include <Jmp4pkg.h>
#include <JVideoOut.h>
#include <yuv_jpg.h>

#ifdef _USE_OPENAL_
#include <alu/openal_utils.h>
#else
#include <alu/audio_track.h>
#endif

#include "revision.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define DEBUG_LOG
//#define DEBUG_PLAY
//#define DEBUG_DECODER
//#define DEBUG_NETWORK_VIDEO_CALLBACK
//#define DEBUG_AUDIO
//#define DEBUG_TS

#define SHOW_STAT

// [Neo] about logs
#define TAG "JNI_PLAY"
#define LOGV(...) if(g_enable_log) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) if(g_enable_log) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) if(g_enable_log) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define LOGX(...) __android_log_print(ANDROID_LOG_DEBUG, "NEO_DEBUG", __VA_ARGS__)
#define LOGXX(...) __android_log_print(ANDROID_LOG_WARN, "NEO_DEBUG", __VA_ARGS__)

#define LOGS(...) __android_log_print(ANDROID_LOG_VERBOSE, "NEO_STAT", __VA_ARGS__)

#define LOCATE __FUNCTION__, __LINE__
#define LOCATE_PT __FUNCTION__, (void*) pthread_self()
#define LOCATE_FULL __FILE__, __FUNCTION__, __LINE__

// [Neo] env consts
#define MAX_WINDOW_COUNT	36
#define MAX_HDEC_INSTANCE	3
#define MAX_DELAY_TIMES		2
#define MIN_FPS				5
#define MAX_FPS_FACTOR		1.2f
#define ADJUST_SIZE			5

// [Neo] what callback to java
#define CALL_CONNECT_CHANGE	0xA1
#define CALL_NORMAL_DATA	0xA2
#define CALL_CHECK_RESULT	0xA3
#define CALL_CHAT_DATA		0xA4
#define CALL_TEXT_DATA		0xA5
#define CALL_DOWNLOAD		0xA6
#define CALL_PLAY_DATA		0xA7
#define CALL_LAN_SEARCH		0xA8
#define CALL_NEW_PICTURE	0xA9
#define CALL_STAT_REPORT	0xAA
#define CALL_GOT_SCREENSHOT	0xAB
#define CALL_PLAY_DOOMED	0xAC
#define CALL_PLAY_AUDIO		0xAD
#define CALL_QUERY_DEVICE	0xAE
#define CALL_HDEC_TYPE		0xAF

#define CALL_LIB_UNLOAD		0xB0
#define CALL_GEN_VOICE		0xB1
#define CALL_PLAY_BUFFER	0xB2
#define CALL_MP4_PRE_INFO   0xB3
#define CALL_MP4_TIME_INFO  0xB4
#define CALL_MP4_PLAY_FIN   0xB5
#define CALL_MP4_PLAY_ERROR 0xB6


//HLS
#define CALL_HLS_PLAY_QUIT  0xC2    //主动退出
#define CALL_HLS_PLAY_OVER  0xC3
#define CALL_HLS_DOWNLOAD_CALLBACK  0xC4


#define BUFFER_START		-1
#define BUFFER_FINISH		-2

#define DEVICE_TYPE_SCARD	0x00
#define DEVICE_TYPE_DVR		0x01
#define DEVICE_TYPE_950		0x02
#define DEVICE_TYPE_951		0x03
#define DEVICE_TYPE_IPC		0x04
#define DEVICE_TYPE_NVR		0x05

#define AUDIO_PCM_RAW		-1

#define FRAME_AMR_SIZE		42
#define FRAME_G711_SIZE		320
#define FRAME_G729_SIZE		60

#define ENC_AMR_SIZE		640
#define ENC_G711_SIZE		640
#define ENC_G729_SIZE		960

#define AUDIO_TYPE_PCM		0x00
#define AUDIO_TYPE_AMR		0x01
#define AUDIO_TYPE_ALAW		0x02
#define AUDIO_TYPE_ULAW		0x03
#define AUDIO_TYPE_G729		0x04

#define TEXT_REMOTE_CONFIG	0x01
#define TEXT_AP				0x02
#define TEXT_GET_STREAM		0x03
#define TEXT_GET_PTZ		0x04

#define FLAG_WIFI_CONFIG	0x01
#define FLAG_WIFI_AP		0x02
#define FLAG_BPS_CONFIG		0x03
#define FLAG_CONFIG_SCCUESS	0x04
#define FLAG_CONFIG_FAILED	0x05
#define FLAG_CONFIG_ING		0x06
#define FLAG_SET_PARAM		0x07

#define FLAG_CAPTURE_FLASH	0x10
#define FLAG_GET_MD_STATE	0x11
#define FLAG_SET_MD_STATE	0x12

#define FLAG_GET_PWD		0x14

#define FLAG_GET_PARAM		0x50
#define FLAG_SET_PARAM_OK	0x55

#define EX_WIFI_CONFIG		0x0A

#define RESULT_SUCCESS		0x01
#define RESULT_NO_FILENAME	0x10
#define RESULT_OPEN_FAILED	0x11
#define RESULT_NO_DATA		0x12

#define BAD_HAS_CONNECTED	-1
#define BAD_CONN_OVERFLOW	-2
#define BAD_NOT_CONNECT		-3
#define BAD_ARRAY_OVERFLOW	-4
#define BAD_CONN_UNKOWN		-5

#define RTMP_CONN_SCCUESS	(0x01 | 0xA0) //链接成功
#define RTMP_CONN_FAILED	(0x02 | 0xA0)//链接失败
#define RTMP_DISCONNECTED	(0x03 | 0xA0)//断开链接成功
#define RTMP_EDISCONNECT	(0x04 | 0xA0)//异常断开
#define RTMP_LOGNNODATA		(0x05 | 0xA0)//长时间没有数据主控断开

#define RTMP_TYPE_META		0x00
#define RTMP_TYPE_H264_I	0x01
#define RTMP_TYPE_H264_BP	0x02
#define RTMP_TYPE_ALAW		0x11
#define RTMP_TYPE_ULAW		0x12

#define BAD_STATUS_NOOP		0x00
#define BAD_STATUS_DECODE	0x01
#define BAD_STATUS_OPENGL	0x02
#define BAD_STATUS_AUDIO	0x03
#define BAD_STATUS_OMX		0x04
#define BAD_STATUS_FFMPEG	0x05
#define BAD_STATUS_LEGACY	0x06
#define PLAYBACK_DONE		0x10
#define VIDEO_SIZE_CHANGED	0x11

#define BAD_SCREENSHOT_NOOP	0x00
#define BAD_SCREENSHOT_INIT	0x01
#define BAD_SCREENSHOT_CONV	0x02
#define BAD_SCREENSHOT_OPEN	0x03

// [Neo] about dummy frame
#define DUMMY_FRAME_0_O		0x00
#define DUMMY_FRAME_0_FIN	0x01
#define DUMMY_FRAME_DIRTY	0x02
#define DUMMY_FRAME_THUMB	0x03
#define DUMMY_FRAME_COLOR	0x10
#define DUMMY_FRAME_SIZE	0x11

#define DUMMY_FRAME_HLS_END	0xFF

// [Neo] decode type
#define TYPE_FFMPEG			0x00
#define TYPE_OMX			0x01
#define TYPE_LEGACY			0x02

using namespace std;

typedef enum _ENCODEING {
	UNKNOW_ENC, ASCII, UTF8_ENC, GBK_ENC
} ENCODING;

// [Neo] video/audio frame
struct frame {
	int size;
	bool is_i_frame;
	bool is_chat_data;
	bool is_play_back;
	unsigned int ts;
	unsigned char* buf;
};

// [Neo] video/audio buffer queue handle
typedef queue<frame*> buffer_queue;

typedef enum _video_type {
	kVTypeUnknown = -1, kVType04 = 0, kVTypeH264 = 1, kVTypeH265 = 2,
} VideoType;

// [Neo] video meta info
struct video_meta {
	int device_start_code;

	VideoType video_type;
	float video_frame_rate;
	float video_frame_rate_backup;

	int video_frame_period;
	int video_max_frame_count;
	unsigned int video_width;
	unsigned int video_height;

	int video_frame_min_count;
	int video_frame_buffer_count;

	bool is_wait_by_ts;
	uint64_t delta_ts;
	bool is_hls_player_over;

	int audio_type;
	int audio_enc_type;
	int audio_sample_rate;
	int audio_channel;
	int audio_bit;
};

// [Neo] statistics
struct stat_suit {
	int video_jump_count;
	int video_decoder_count;
	int video_network_count;

	int audio_jump_count;
	int audio_decoder_count;
	int audio_network_count;

	int video_frame_count;
	int video_frame_space;

	long video_network_bytes;

	long audio_network_bytes;

	long long video_decoder_delayed;
	long long video_render_delayed;

	long long audio_decoder_delayed;
	long long audio_play_delayed;

	pthread_mutex_t mutex;
};

// [Neo] core worker
struct player_core {
	// [Neo] about buffer queue
	buffer_queue* video_queue_handle;
	buffer_queue* audio_queue_handle;
	// [Neo] queue semaphore
	sem_t video_queue_st;
	sem_t audio_queue_st;
	// [Neo] queue mutex lock
	pthread_mutex_t video_queue_mt;
	pthread_mutex_t audio_queue_mt;

	// [Neo] about decoder
	JDEC_param_t* legacy_handle;
	JADEC_HANDLE audio_handle;
	JDEC05_HANDLE decoder_handle;
	JHD_ANDROID_HANDLE hdec_handle;

	// [Neo] about OpenGL
	int opengl_status;
	ANativeWindow* opengl_window;
	JVO_HANDLE opengl_handle;
	VO_IN_YUV* yuv;
	VO_IN_YUV* yuv_thumb;
	// [Neo] openGL mutex lock, but it makes fatel error, weird
	pthread_mutex_t opengl_mt;
};

// [Neo] player
struct player_suit {
	bool is_connecting;
	bool is_connected;
	bool is_audio_working;
	bool is_disconnecting;

	bool is_JFH;
	bool is_turn;

	bool is_play_audio;
	bool is_playback_mode;

	bool try_omx;
	bool try_screenshot;
	bool try_fast_forward;

	char* thumb_name;
	char* screenshot_name;

	video_meta* vm_normal;
	video_meta* vm_playback;

	stat_suit* stat;
	player_core* core;

#ifdef _USE_OPENAL_
	OpenALUtils* alu;
#else
	AudioTrack* track;
#endif
};

// [Neo] video & audio recoder
struct recorder_suit {
	bool need_jump;
	bool enable_audio;
	bool enable_video;

	int window;
	MP4_PKG_HANDLE handle;

	// [Neo] for audio & video
	pthread_mutex_t mutex;
};

struct color_suit {
	float red;
	float green;
	float blue;
	float alpha;
};

// [Neo] OpenGL inner status types
typedef enum _OPENGL_STATUS {
	// [Neo] init or detach -> attach
	OPENGL_UNATTACHED = 0,
	// [Neo] close or attach -> open or detach
	OPENGL_ATTACHED,
	// [Neo] open -> render or close
	OPENGL_OPENGED,
	// [Neo] (opened) -> close
	OPENGL_TRY_CLOSE,
	// [Neo] (attached) -> open
	OPENGL_TRY_OPEN,
} OPENGL_STATUS;

extern JavaVM* g_jvm;
extern jobject g_handle;
extern jmethodID g_notifyid;

extern bool is_audio_end;
extern bool is_video_end;

extern bool g_enable_log;

extern bool g_is_stat_mode;
extern bool g_is_record_mode;

extern bool g_has_omx_inited;
// [Neo] FIXME over three instances, omx will crash
extern int g_hdec_instance_count;

extern hJPG g_jpg;

extern int g_thumb_width;
extern int g_thumb_quality;
extern int g_picture_quality;

extern FILE* g_download_file;
extern char* g_download_file_name;

extern struct recorder_suit* g_recorder;

extern pthread_mutex_t g_mutex;
extern int g_connect_indexes[MAX_WINDOW_COUNT];

extern struct player_suit* g_player[MAX_WINDOW_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* _JNI_DEFINES_H */
