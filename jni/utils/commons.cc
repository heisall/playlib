// Generated by Neo

#include <time.h>
#include <math.h>
#include <sys/select.h>

#include "utils/commons.h"

hJPG g_jpg;

int g_thumb_width;
int g_thumb_quality;
int g_picture_quality;

bool g_enable_log;
bool g_has_omx_inited;
int g_hdec_instance_count;

pthread_mutex_t g_mutex;
int g_connect_indexes[MAX_WINDOW_COUNT];

struct player_suit* g_player[MAX_WINDOW_COUNT];

const int kRTMPFrameMinCount = 5;
const int kRTMPFrameBufferCount = 50;

/**
 * 毫秒级的睡觉
 *
 */
void msleep(int millis) {
	if (millis > 0) {
		struct timeval tt;
		tt.tv_sec = millis / 1000;
		tt.tv_usec = (millis % 1000) * 1000;
		select(0, NULL, NULL, NULL, &tt);
	}
}

/**
 * 获取当前时间，单位是毫秒
 *
 */
long long currentMillisSec() {
	long long result = 0l;

	struct timeval t;
	gettimeofday(&t, NULL);
	result = (long long) t.tv_sec * 1000 + t.tv_usec / 1000;

	return result;
}

ENCODING detectEncoding(char* str) {
	ENCODING enc = ASCII;

	int length = strlen(str);
	bool is_only_ascii = true;
	for (int i = 0; i < length; ++i) {
		if (0x80 < (0xFF & str[i])) {
			is_only_ascii = false;
		}

		if (i + 2 < length && ((0xFF & str[i]) >> 4) == 0x0E
				&& ((0xFF & str[i + 1]) >> 6) == 0x02
				&& ((0xFF & str[i + 2]) >> 6) == 0x02) {
			// [Neo] 0xB0~0xF7, 0xA0~0xFE
			enc = UTF8_ENC;
			break;
		} else if (i + 1 < length
				&& ((0xFF & str[i]) >= 0x81 && (0xFF & str[i]) <= 0xFE)
				&& ((0xFF & str[i + 1]) >= 0x64 && (0xFF & str[i + 1]) <= 0xFE)) {
			// [Neo] 0x81~0xF7, 0x40~0xFE
			enc = GBK_ENC;
			break;
		}
	}

	if (ASCII == enc && false == is_only_ascii) {
		enc = UNKNOW_ENC;
	}

	return enc;
}

void write2file(BYTE* buf, int size) {
	static int aaa = 0;
	static FILE * fout = NULL;
	aaa++;
	if (aaa == 1) {
		fout = fopen("/path/file", "wb");
	}
	if (fout) {
		fwrite(buf, size, 1, fout);
	}
	if (aaa == 100) {
		fclose(fout);
		fout = NULL;
	}
}

/**
 * 获取有效的数组索引
 *
 */
int getValidArrayIndex(int window) {
	int result = BAD_HAS_CONNECTED;

	pthread_mutex_lock(&g_mutex);
	for (int i = 0; i < MAX_WINDOW_COUNT; i++) {
		if (window == g_connect_indexes[i]) {
			result = i;
			break;
		}
	}

	if (result < 0) {
		result = BAD_CONN_OVERFLOW;

		for (int i = 0; i < MAX_WINDOW_COUNT; i++) {
			if (g_connect_indexes[i] < 0) {
				g_connect_indexes[i] = window;
				result = i;
				break;
			}
		}
	} else {
		result = BAD_HAS_CONNECTED;
	}
	pthread_mutex_unlock(&g_mutex);

	return result;
}

/**
 * 将窗口索引转化至数组索引
 *
 */
int window2Array(int window) {
	int result = BAD_NOT_CONNECT;

	pthread_mutex_lock(&g_mutex);
	for (int i = 0; i < MAX_WINDOW_COUNT; i++) {
		if (window == g_connect_indexes[i]) {
			result = i;
			break;
		}
	}
	pthread_mutex_unlock(&g_mutex);

	return result;
}

int array2Window(int index) {
	int result = BAD_ARRAY_OVERFLOW;

	pthread_mutex_lock(&g_mutex);
	if (index >= 0 && index < MAX_WINDOW_COUNT) {
		result = g_connect_indexes[index];
	}
	pthread_mutex_unlock(&g_mutex);

	return result;
}

void invalidArray(int index) {
	pthread_mutex_lock(&g_mutex);
	if (index >= 0 && index < MAX_WINDOW_COUNT) {
		g_player[index] = NULL;
		g_connect_indexes[index] = BAD_NOT_CONNECT;
	}
	pthread_mutex_unlock(&g_mutex);
}

/**
 * 检查 O 帧帧头类型
 *
 */
bool checkHeader(BYTE* buf, int size) {
	bool result = false;
	int header_size = sizeof(JVS_FILE_HEADER_EX);

	if (NULL != buf && size >= header_size) {
		result = (buf[34] == 'J' && buf[35] == 'F' && buf[36] == 'H');
	}

	return result;
}

/**
 * 跳过帧头
 *
 */
bool skipHeader(BYTE* buf) {
	bool result = false;

	if (NULL != buf) {
		result = (buf[0] == 'J' && buf[1] == 'V' && buf[2] == 'S');
	}

	return result;
}

/**
 * 检查设备标识，判断是否使用标准解码器
 *
 */
bool checkStartCode(int startCode) {
	bool result;

	if (startCode != JVN_DSC_9800CARD || startCode != JVN_DSC_CARD) {
		result = true;
	}

	return result;
}

/**
 * 转化音频数据
 *
 */
BYTE* convertAudioData(jbyte* data) {
	int size = 320;
	int16_t *pszPcmBuf = (int16_t *) data;
	BYTE* pConvert = (BYTE*) malloc(sizeof(BYTE) * size);
	memset(pConvert, 0, size);

	for (int i = 0; i < size; i++) {
		pConvert[i] = pszPcmBuf[i] >> 8;
	}

	return pConvert;
}

/**
 * 判断当前是否已经断开
 *
 */
bool isDisconnected(int uchType) {
	bool result = false;
	if (uchType == JVN_CCONNECTTYPE_DISOK || uchType == JVN_CCONNECTTYPE_CONNERR
			|| uchType == JVN_CCONNECTTYPE_DISCONNE
			|| uchType == JVN_CCONNECTTYPE_SSTOP) {
		result = true;
	}

	return result;
}

/**
 * 获取真正有用的音频类型
 *
 */
int getUsefulAudioType(int type) {
	int result = AUDIO_PCM_RAW;

	switch (type) {
	case AUDIO_TYPE_PCM:
		result = AUDIO_PCM_RAW;
		break;

	case AUDIO_TYPE_AMR:
		result = JAD_CODEC_SAMR;
		break;

	case AUDIO_TYPE_ALAW:
		result = JAD_CODEC_ALAW;
		break;

	case AUDIO_TYPE_ULAW:
		result = JAD_CODEC_ULAW;
		break;

	case AUDIO_TYPE_G729:
		result = JAD_CODEC_G729;
		break;

	default:
		break;
	}

	return result;
}

VO_IN_YUV* genYUV(int y_pitch, int uv_pitch, int width, int height) {
	PVO_IN_YUV yuv = (PVO_IN_YUV) malloc(sizeof(VO_IN_YUV));

	yuv->i_chroma = 0;
	yuv->i_width = width;
	yuv->i_height = height;

	yuv->p[0].i_pitch = y_pitch;
	yuv->p[1].i_pitch = uv_pitch;
	yuv->p[2].i_pitch = uv_pitch;

	yuv->p[0].p_pixels = (unsigned char*) malloc(y_pitch * height);
	yuv->p[1].p_pixels = (unsigned char*) malloc(uv_pitch * height >> 1);
	yuv->p[2].p_pixels = (unsigned char*) malloc(uv_pitch * height >> 1);

	return yuv;
}

void dupYUV(VO_IN_YUV* dst, VO_IN_YUV* src) {
	if (NULL != src) {
		dst->i_width = src->i_width;
		dst->i_height = src->i_height;
		dst->i_chroma = src->i_chroma;

		if (NULL != src->p[0].p_pixels && NULL != src->p[1].p_pixels
				&& NULL != src->p[2].p_pixels) {
			dst->p[0].i_pitch = src->p[0].i_pitch;
			dst->p[1].i_pitch = src->p[1].i_pitch;
			dst->p[2].i_pitch = src->p[2].i_pitch;

			memcpy(dst->p[0].p_pixels, src->p[0].p_pixels,
					src->p[0].i_pitch * src->i_height);
			memcpy(dst->p[1].p_pixels, src->p[1].p_pixels,
					src->p[1].i_pitch * src->i_height >> 1);
			memcpy(dst->p[2].p_pixels, src->p[2].p_pixels,
					src->p[2].i_pitch * src->i_height >> 1);
		}
	}
}

void deleteYUV(VO_IN_YUV* yuv) {
	if (NULL != yuv) {
		if (NULL != yuv->p[0].p_pixels) {
			free(yuv->p[0].p_pixels);
		}

		if (NULL != yuv->p[1].p_pixels) {
			free(yuv->p[1].p_pixels);
		}

		if (NULL != yuv->p[2].p_pixels){
			free(yuv->p[2].p_pixels);
		}

//		free(yuv); 应在外部释放；
	}
}

/**
 * 生成播放句柄
 *
 */
player_suit* genPlayer(int index) {
	size_t size = sizeof(player_suit);
	player_suit* player = (player_suit*) malloc(size);
	memset(player, 0, size);

	player->is_connecting = false;
	player->is_connected = false;
	player->is_audio_working = false;
	player->is_disconnecting = false;

	player->is_JFH = false;
	player->is_turn = false;

	player->is_play_audio = false;
	player->is_playback_mode = false;

	player->try_omx = false;
	player->try_screenshot = false;
	player->try_fast_forward = false;

	player->thumb_name = NULL;
	player->screenshot_name = NULL;

	// [Neo] normal video meta
	size = sizeof(video_meta);
	video_meta* normal = (video_meta*) malloc(size);
	normal->device_start_code = -1;
	normal->video_type = kVTypeUnknown;
	normal->video_frame_rate = -1.0f;
	normal->video_frame_period = -1;
	normal->video_max_frame_count = 0;
	normal->video_width = 0;
	normal->video_height = 0;
	normal->is_wait_by_ts = false;
	normal->audio_sample_rate = 0;
	normal->audio_channel = 0;
	normal->audio_bit = 0;
	normal->audio_type = AUDIO_PCM_RAW;
	normal->audio_enc_type = JAE_ENCODER_G729;
	normal->video_frame_min_count = kRTMPFrameMinCount;
	normal->video_frame_buffer_count = kRTMPFrameBufferCount;

	player->vm_normal = normal;

	// [Neo] playback video meta
	video_meta* playback = (video_meta*) malloc(size);
	memcpy(playback, normal, size);

	player->vm_playback = playback;

	// [Neo] stat suit
	size = sizeof(stat_suit);
	stat_suit* stat = (stat_suit*) malloc(size);
	stat->video_jump_count = 0;
	stat->audio_jump_count = 0;
	stat->video_decoder_count = 0;
	stat->video_network_count = 0;
	stat->audio_decoder_count = 0;
	stat->audio_network_count = 0;
	stat->video_frame_count = 0;
	stat->video_frame_space = 0;
	stat->video_network_bytes = 0l;
	stat->audio_network_bytes = 0l;
	stat->video_decoder_delayed = 0l;
	stat->video_render_delayed = 0l;
	stat->audio_decoder_delayed = 0l;
	stat->audio_play_delayed = 0l;
	pthread_mutex_init(&(stat->mutex), NULL);

	player->stat = stat;

	// [Neo] player core
	size = sizeof(player_core);
	player_core* core = (player_core*) malloc(size);
	memset(core, 0, size);
	core->video_queue_handle = new buffer_queue();
	core->audio_queue_handle = new buffer_queue();
	core->opengl_status = OPENGL_UNATTACHED;
	sem_init(&(core->video_queue_st), 0, 0);
	sem_init(&(core->audio_queue_st), 0, 0);
	pthread_mutex_init(&(core->video_queue_mt), NULL);
	pthread_mutex_init(&(core->audio_queue_mt), NULL);
	pthread_mutex_init(&(core->opengl_mt), NULL);
	core->legacy_handle = NULL;
	core->audio_handle = NULL;
	core->decoder_handle = NULL;
	core->hdec_handle = NULL;

	player->core = core;
	player->nplayer = NULL;

#ifdef _USE_OPENAL_
	player->alu = NULL;
#else
	player->track = NULL;
#endif

	g_player[index] = player;
	return player;
}

/**
 * 清理播放器
 *
 */
void deletePlayer(int index) {
	player_suit* player = g_player[index];

	if (NULL != player) {
		clean_all_queue(player);

		sem_destroy(&(player->core->video_queue_st));
		sem_destroy(&(player->core->audio_queue_st));

		pthread_mutex_destroy(&(player->core->video_queue_mt));
		pthread_mutex_destroy(&(player->core->audio_queue_mt));
		pthread_mutex_destroy(&(player->core->opengl_mt));

		pthread_mutex_destroy(&(player->stat->mutex));

		delete player->core->video_queue_handle;
		delete player->core->audio_queue_handle;

		deleteYUV(player->core->yuv);
		deleteYUV(player->core->yuv_thumb);

		free(player->core->yuv);
		free(player->core->yuv_thumb);

		free(player->core);
		free(player->stat);
		free(player->vm_normal);
		free(player->vm_playback);

		if (NULL != player->thumb_name) {
			free(player->thumb_name);
		}

		free(player);
	}

	invalidArray(index);
}

bool openOmx(player_suit* player, int window) {
	bool result = false;

	player_core* core = player->core;
	video_meta* meta = player->vm_normal;

	if (player->is_playback_mode) {
		meta = player->vm_playback;
	}

	if (player->try_omx) {
		pthread_mutex_lock(&(g_mutex));
		if (g_hdec_instance_count < MAX_HDEC_INSTANCE) {
			g_hdec_instance_count++;
			result = true;
		}
		pthread_mutex_unlock(&(g_mutex));

		if (result) {

			result = false;
			core->hdec_handle = JHD05_OpenDecoder(meta->video_width,
					meta->video_height);

			if (NULL == core->hdec_handle) {
				pthread_mutex_lock(&(g_mutex));
				g_hdec_instance_count--;
				pthread_mutex_unlock(&(g_mutex));
				player->try_omx = false;
			} else {
				result = true;
			}

		} else {
			player->try_omx = false;
		}
	}

	return result;
}

bool closeOmx(player_suit* player) {
	bool result = false;

	player_core* core = player->core;
	if (NULL != core->hdec_handle) {
		JHD05_CloseDecoder(core->hdec_handle);
		core->hdec_handle = NULL;

		pthread_mutex_lock(&(g_mutex));
		if (g_hdec_instance_count > 0) {
			g_hdec_instance_count--;
		}
		pthread_mutex_unlock(&(g_mutex));
		result = true;
	}

	return result;
}

bool reopenOmx(int index) {
	bool result = false;

	int window = array2Window(index);
	player_suit* player = g_player[index];

	closeOmx(player);
	result = openOmx(player, window);

	return result;
}

bool glResume(JNIEnv* env, player_suit* player, jobject surface) {
	bool result = true;

	if (NULL != env && NULL != player && NULL != surface) {
		if (NULL == player->core->opengl_handle) {
			if (OPENGL_ATTACHED == player->core->opengl_status
					|| NULL != player->core->opengl_window) {
				// [Neo] careful, must not lock here!
				glDetach(player);
			}

			pthread_mutex_lock(&(player->core->opengl_mt));
			player->core->opengl_window = ANativeWindow_fromSurface(env,
					surface);

			if (NULL != player->core->opengl_window) {
				player->core->opengl_status = OPENGL_TRY_OPEN;
			} else {
				LOGE("resume failed, from surface!!");
				result = false;
			}
			pthread_mutex_unlock(&(player->core->opengl_mt));
		}
	}

	return result;
}

bool glPause(player_suit* player) {
	bool result = false;

//	LOGW("***********commons glPause！***********");
	if (NULL != player && NULL != player->core) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window) {
			ANativeWindow_release(player->core->opengl_window);
			player->core->opengl_window = NULL;
			player->core->opengl_status = OPENGL_TRY_CLOSE;
			result = true;
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

	return result;
}

/**
 * 分离显示窗口
 *
 */
bool glDetach(player_suit* player) {
//	LOGW("%s [%p] E", LOCATE_PT);
	bool result = false;
//	LOGW("***********commons glDetach！***********");
	if (NULL != player && NULL != player->core) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window) {
			ANativeWindow_release(player->core->opengl_window);
//			LOGW("gldetach player->core->opengl_window is null");
			player->core->opengl_window = NULL;
			player->core->opengl_status = OPENGL_UNATTACHED;
			result = true;
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

//	LOGW("%s [%p] X, %d", LOCATE_PT, result);
	return result;
}

/**
 * 附加显示窗口
 *
 */
bool glAttach(JNIEnv* env, player_suit* player, jobject surface) {
//	LOGW("%s [%p] E", LOCATE_PT);
	bool result = false;

	if (NULL != env && NULL != player && NULL != surface) {
		if (NULL == player->core->opengl_handle) {
			if (OPENGL_ATTACHED == player->core->opengl_status) {
				// [Neo] careful, must not lock here!
				glDetach(player);
			}

			pthread_mutex_lock(&(player->core->opengl_mt));
			player->core->opengl_window = ANativeWindow_fromSurface(env,
					surface);

			if (NULL != player->core->opengl_window) {
				player->core->opengl_status = OPENGL_ATTACHED;
				result = true;
			} else {
				LOGE("attach failed, from surface!!");
			}
			pthread_mutex_unlock(&(player->core->opengl_mt));

		} else {
			LOGE("attach failed, OpenGL handle has been init!!");
		}
	}

//	LOGW("%s [%p] X, %d", LOCATE_PT, result);
	return result;
}

/**
 * 打开显示句柄，需要在线程中调用
 *
 */
bool glOpen(player_suit* player) {
//	LOGW("%s [%p] E", LOCATE_PT);
	bool result = false;

	if (NULL != player) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window
				&& NULL == player->core->opengl_handle) {
			if (OPENGL_ATTACHED == player->core->opengl_status
					|| OPENGL_TRY_OPEN == player->core->opengl_status) {
//				LOGW("glopen player->core->opengl_window is not null ");
				player->core->opengl_handle = JVO_Open(
						player->core->opengl_window);
				if (NULL != player->core->opengl_handle) {
					result = true;
					player->core->opengl_status = OPENGL_OPENGED;
				}
			} else {
				LOGE("open failed, with bad status = %d!!", player->core->opengl_status);
			}
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

//	LOGW("%s [%p] X, %d", LOCATE_PT, result);
	return result;
}

/**
 * 关闭显示句柄，需要在线程中调用
 *
 */
bool glClose(player_suit* player) {
//	LOGW("%s [%p] E", LOCATE_PT);
	bool result = false;

	if (NULL != player) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_handle) {
			if (OPENGL_OPENGED == player->core->opengl_status
					|| OPENGL_TRY_CLOSE == player->core->opengl_status) {
				JVO_Close(player->core->opengl_handle);
				result = true;
				player->core->opengl_handle = NULL;
				player->core->opengl_status = OPENGL_ATTACHED;
			} else {
				LOGE(
						"close failed, with bad status = %d!!", player->core->opengl_status);
			}
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

//	LOGW("%s [%p] X, %d", LOCATE_PT, result);
	return result;
}

/**
 * 渲染当前的 YUV 数据
 *
 */
int glRender(player_suit* player, PVO_IN_YUV pVo) {
	int result = -1;

	if (NULL != player && NULL != pVo) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window
				&& NULL != player->core->opengl_handle) {
			if (OPENGL_OPENGED == player->core->opengl_status) {
#ifdef DEBUG_DECODER
				LOGE(
						"render.chroma: %d, %dx%d", pVo->i_chroma, pVo->i_width, pVo->i_height);
#endif
				result = JVO_Render(player->core->opengl_handle, pVo);
			} else {
				LOGW(
						"render failed, with bad status = %d", player->core->opengl_status);
			}
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

	return result;
}

int glColor(player_suit* player, float red, float green, float blue,
		float alpha) {
	int result = -1;

	if (NULL != player) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window
				&& NULL != player->core->opengl_handle) {
			result = JVO_ClearColor(player->core->opengl_handle, red, green,
					blue, alpha);
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

	return result;
}

/**
 * 判断当前的状态是否可用于渲染显示
 *
 */
bool glReady(player_suit* player) {
	bool result = false;

	if (NULL != player) {
		pthread_mutex_lock(&(player->core->opengl_mt));
		if (NULL != player->core->opengl_window
				&& NULL != player->core->opengl_handle) {
			if (OPENGL_OPENGED == player->core->opengl_status) {
				result = true;
			} else {
				LOGE("Bad status: %d", player->core->opengl_status);
			}
		}
		pthread_mutex_unlock(&(player->core->opengl_mt));
	}

	return result;
}

int screenshot(int index, PVO_IN_YUV pVo, bool is_thumb) {
	int result = BAD_SCREENSHOT_NOOP;

	player_suit* player = g_player[index];
	FILE* jpg_file = NULL;
	unsigned long jpg_length = 0;
	unsigned char* jpg_out = NULL;

	int quality = g_picture_quality;
	char* name = player->screenshot_name;

	if (is_thumb) {
		quality = g_thumb_quality;
		name = player->thumb_name;
	}

	if (NULL != g_jpg && NULL != name && NULL != pVo) {
		pthread_mutex_lock(&g_mutex);
		LOGX("screenshot: %s, %p", name, pVo);
		LOGX(
				"screenshot check: %dx%d, %p(%d), %p(%d), %p(%d)", pVo->i_width, pVo->i_height, pVo->p[0].p_pixels, pVo->p[0].i_pitch, pVo->p[2].p_pixels, pVo->p[2].i_pitch, pVo->p[1].p_pixels, pVo->p[1].i_pitch);
		if (yuv_jpg_do(g_jpg, quality, pVo->p[0].p_pixels, pVo->p[2].p_pixels,
				pVo->p[1].p_pixels, pVo->p[0].i_pitch, pVo->p[2].i_pitch,
				pVo->p[1].i_pitch, pVo->i_width, pVo->i_height, 0, &jpg_out,
				&jpg_length) >= 0) {

			jpg_file = fopen(name, "wb");

			if (NULL != jpg_file) {
				fwrite(jpg_out, jpg_length, 1, jpg_file);
				fclose(jpg_file);
				jpg_file = NULL;
			} else {
				result = BAD_SCREENSHOT_OPEN;
			}

		} else {
			result = BAD_SCREENSHOT_CONV;
		}
		pthread_mutex_unlock(&g_mutex);

	} else {
		result = BAD_SCREENSHOT_INIT;
	}

	if (NULL != player->screenshot_name) {
		free(player->screenshot_name);
		player->screenshot_name = NULL;
	}

	return result;
}

/**
 * 为缓冲队列提供新的视频数据
 *
 */
void offer_video_frame(player_suit* player, BYTE* buf, int size, int type,
		bool is_play_back, unsigned int ts) {
	if (NULL == player || NULL == player->core
			|| NULL == player->core->audio_queue_handle) {
		return;
	}

	size_t s = sizeof(frame);
	frame* f = (frame*) malloc(s);
	memset(f, 0, s);

	if (size > 0) {
		f->size = size;
		f->is_i_frame = (type == JVN_DATA_I);
		f->buf = (unsigned char*) malloc(sizeof(unsigned char) * size);
		memcpy(f->buf, buf, size);

		pthread_mutex_lock(&(player->stat->mutex));
		if (type == JVN_DATA_I) {
			player->stat->video_frame_space = player->stat->video_frame_count;
			player->stat->video_frame_count = 0;
		}

		player->stat->video_frame_count++;
		player->stat->video_network_count++;
		player->stat->video_network_bytes += size;
		pthread_mutex_unlock(&(player->stat->mutex));
	} else {
		f->size = 0;
		f->buf = NULL;
		f->is_i_frame = false;
	}

	f->ts = ts;
	f->is_chat_data = false;
	f->is_play_back = is_play_back;

	pthread_mutex_lock(&(player->core->video_queue_mt));
	player->core->video_queue_handle->push(f);
	pthread_mutex_unlock(&(player->core->video_queue_mt));
	sem_post(&(player->core->video_queue_st));
}

/**
 * 从缓冲队列中拾取视频数据
 *
 */
frame* poll_video_frame(player_suit* player) {
	frame* f = NULL;

	if (NULL != player && NULL != player->core->video_queue_handle) {
		sem_wait(&(player->core->video_queue_st));
		pthread_mutex_lock(&(player->core->video_queue_mt));
		if (player->core->video_queue_handle->size() > 0) {
			f = player->core->video_queue_handle->front();
			player->core->video_queue_handle->pop();
		}
		pthread_mutex_unlock(&(player->core->video_queue_mt));
	}

	return f;
}

/**
 * 为缓冲队列提供新的音频数据
 *
 */
void offer_audio_frame(player_suit* player, BYTE* buf, int size,
		bool is_chat_data, bool is_play_back, unsigned int ts) {
	if (NULL == player || NULL == player->core
			|| NULL == player->core->audio_queue_handle) {
		return;
	}

	size_t s = sizeof(frame);
	frame* f = (frame*) malloc(s);
	memset(f, 0, s);

	if (size > 0) {
		f->size = size;
		f->buf = (unsigned char*) malloc(sizeof(unsigned char) * size);
		memcpy(f->buf, buf, size);

		pthread_mutex_lock(&(player->stat->mutex));
		player->stat->audio_network_count++;
		player->stat->audio_network_bytes += size;
		pthread_mutex_unlock(&(player->stat->mutex));
	} else {
		f->size = 0;
		f->buf = NULL;
	}

	f->ts = ts;
	f->is_chat_data = is_chat_data;
	f->is_play_back = is_play_back;

	pthread_mutex_lock(&(player->core->audio_queue_mt));
	player->core->audio_queue_handle->push(f);
	pthread_mutex_unlock(&(player->core->audio_queue_mt));
	sem_post(&(player->core->audio_queue_st));
}

/**
 * 从缓冲队列中拾取音频数据
 *
 */
frame* poll_audio_frame(player_suit* player) {
	frame* f = NULL;

	if (NULL != player && NULL != player->core->audio_queue_handle) {
		sem_wait(&(player->core->audio_queue_st));
		pthread_mutex_lock(&(player->core->audio_queue_mt));
		if (player->core->audio_queue_handle->size() > 0) {
			f = player->core->audio_queue_handle->front();
			player->core->audio_queue_handle->pop();
		}
		pthread_mutex_unlock(&(player->core->audio_queue_mt));
	}

	return f;
}

/**
 * 获取当前缓冲队列中的视频帧数量
 *
 */
int get_video_left(player_suit* player) {
	int size = 0;

	if (NULL != player && NULL != player->core->video_queue_handle) {
		pthread_mutex_lock(&(player->core->video_queue_mt));
		size = player->core->video_queue_handle->size();
		pthread_mutex_unlock(&(player->core->video_queue_mt));
	}

	return size;
}

int get_audio_left(player_suit* player) {
	int size = 0;

	if (NULL != player && NULL != player->core->audio_queue_handle) {
		pthread_mutex_lock(&(player->core->audio_queue_mt));
		size = player->core->audio_queue_handle->size();
		pthread_mutex_unlock(&(player->core->audio_queue_mt));
	}

	return size;
}

/**
 * 清空视频和音频缓冲队列
 *
 */
bool clean_all_queue(player_suit* player) {
	bool result = false;

	if (NULL != player && NULL != player->core->video_queue_handle) {
		pthread_mutex_lock(&(player->core->video_queue_mt));
		while (player->core->video_queue_handle->size() > 0) {
			frame* f = player->core->video_queue_handle->front();
			player->core->video_queue_handle->pop();
			destroy(f);
		}
		player->try_fast_forward = true;

		deleteYUV(player->core->yuv);
		deleteYUV(player->core->yuv_thumb);

		free(player->core->yuv);
		free(player->core->yuv_thumb);

		player->core->yuv = NULL;
		player->core->yuv_thumb = NULL;

		pthread_mutex_unlock(&(player->core->video_queue_mt));
		result = true;
	}

	if (NULL != player && NULL != player->core->audio_queue_handle) {
		pthread_mutex_lock(&(player->core->audio_queue_mt));
		while (player->core->audio_queue_handle->size() > 0) {
			frame* f = player->core->audio_queue_handle->front();
			player->core->audio_queue_handle->pop();
			destroy(f);
		}
		pthread_mutex_unlock(&(player->core->audio_queue_mt));
		result = true;
	}

	return result;
}

/**
 * 销毁不需要的数据帧
 *
 */
bool destroy(frame* f) {
	bool result = false;

	if (NULL != f) {
		if (NULL != f->buf) {
			free(f->buf);
			f->buf = NULL;
		}

		free(f);
		f = NULL;
		result = true;
	}

	return result;
}

void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, FILE * fout) {
	for (int i = 0; i < ysize; i++) {
		fwrite(buf + i * wrap, 1, xsize, fout);
	}
}

int decode(int type, void* handle, H264_PACKET* in, PVO_IN_YUV out, int* arg1,
		int* arg2) {
	// [Neo] < 0 fatal, reopen or quit
	// [Neo] = 0 no picture
	// [Neo] > 0 show up
	int got = -1;
	int result = -1;

	PYUV_PACKET packet = NULL;
	PHDEC_ANDROID_YUV_PKT packet_hd = NULL;

	out->i_width = 0;
	out->i_height = 0;
	out->i_chroma = (out->i_chroma + 1) % 10;

	switch (type) {
	case TYPE_FFMPEG: {
		result = JVD05_DecodeOneFrameEx((JDEC05_HANDLE) handle,
				(PH264_PACKET) in, &packet, arg1);
		got = packet->i_got_picture;

		if (result > 0 && NULL != packet && got > 0) {
			out->p[0].i_pitch = packet->i_y_pitch;
			out->p[2].i_pitch = packet->i_u_pitch;
			out->p[1].i_pitch = packet->i_v_pitch;

			out->p[0].p_pixels = packet->pY;
			out->p[2].p_pixels = packet->pU;
			out->p[1].p_pixels = packet->pV;

			out->i_width = packet->i_frame_width;
			out->i_height = packet->i_frame_height;
		}
		break;
	}

	case TYPE_OMX: {
		result = JHD05_DecodeOneFrame((JHD_ANDROID_HANDLE) handle,
				(PHDEC_ANDROID_H264_PKT) in, &packet_hd, &got);

		if (result > 0 && NULL != packet_hd && got > 0) {
			out->p[0].i_pitch = packet_hd->p[0].i_pitch;
			out->p[2].i_pitch = packet_hd->p[1].i_pitch;
			out->p[1].i_pitch = packet_hd->p[2].i_pitch;

			out->p[0].p_pixels = packet_hd->p[0].p_pixels;
			out->p[2].p_pixels = packet_hd->p[1].p_pixels;
			out->p[1].p_pixels = packet_hd->p[2].p_pixels;

			out->i_chroma = packet_hd->i_chroma;
			out->i_width = packet_hd->i_width;
			out->i_height = packet_hd->i_height;
		}
		break;
	}

	case TYPE_LEGACY: {
		result = 1;
		JDEC_param_t* h = (JDEC_param_t*) handle;

		if (in->i_payload > 0) {
			// [Neo] TODO
//			memcpy(h->p_payload, in->p_payload, h->i_payload);
			h->p_payload = in->p_payload;
			h->i_payload = in->i_payload;

			JVD04_DecodeOneFrame(h);
			got = 1;
			out->p[0].i_pitch = h->i_y_pitch;
			out->p[2].i_pitch = h->i_uv_pitch;
			out->p[1].i_pitch = h->i_uv_pitch;

			out->p[0].p_pixels = h->pY;
			out->p[2].p_pixels = h->pU;
			out->p[1].p_pixels = h->pV;

			out->i_width = h->i_frame_width;
			out->i_height = h->i_frame_height;
		}
		break;
	}

	default:
		break;
	}

#ifdef DEBUG_DECODER
	LOGI(
			"deocde: type = %d, size = %d, got/result = %d/%d, render = %p", type, in->i_payload, got, result, out);
#endif

	if (result > 0 && 0 == got) {
		result = 0;
	}

	return result;
}

void yuv_convert(VO_IN_YUV* dst, VO_IN_YUV* src) {
	unsigned int i, j, x, y, k;
	unsigned int iHalfDstWidth = dst->i_width >> 1;
	unsigned int iHalfDstHeight = dst->i_height >> 1;

	float xScale = (float) (src->i_width) / dst->i_width;
	float yScale = (float) (src->i_height) / dst->i_height;

	k = 0;
	for (i = 0; i < dst->i_height; i++) {
		y = (int) (i * yScale) * src->p[0].i_pitch;
		for (j = 0; j < dst->i_width; j++) {
			x = (int) (j * xScale);
			dst->p[0].p_pixels[k + j] = src->p[0].p_pixels[y + x];
		}

		k += dst->p[0].i_pitch;
	}

	k = 0;
	for (i = 0; i < iHalfDstHeight; i++) {
		y = (int) (i * yScale) * src->p[1].i_pitch;
		for (j = 0; j < iHalfDstWidth; j++) {
			x = (int) (j * xScale);
			dst->p[1].p_pixels[k + j] = src->p[1].p_pixels[y + x];
			dst->p[2].p_pixels[k + j] = src->p[2].p_pixels[y + x];
		}

		k += dst->p[1].i_pitch;
	}
}

void yuv_translate(unsigned char * pDstY, int iDstYPitch, unsigned char * pDstU,
		unsigned char * pDstV, int iDstUVPitch, int iDstWidth, int iDstHeight,
		unsigned char * pSrcY, int iSrcYPitch, unsigned char * pSrcU,
		unsigned char * pSrcV, int iSrcUVPitch, int iSrcWidth, int iSrcHeight) {
	int i, j, x, y, k;
	int iHalfDstWidth = iDstWidth >> 1;
	int iHalfDstHeight = iDstHeight >> 1;

	float xScale = (float) iSrcWidth / iDstWidth;
	float yScale = (float) iSrcHeight / iDstHeight;

	k = 0;
	for (i = 0; i < iDstHeight; i++) {
		y = (int) (i * yScale) * iSrcYPitch;
		for (j = 0; j < iDstWidth; j++) {
			x = (int) (j * xScale);
			pDstY[k + j] = pSrcY[y + x];
		}

		k += iDstYPitch;
	}

	k = 0;
	for (i = 0; i < iHalfDstHeight; i++) {
		y = (int) (i * yScale) * iSrcUVPitch;
		for (j = 0; j < iHalfDstWidth; j++) {
			x = (int) (j * xScale);
			pDstU[k + j] = pSrcU[y + x];
			pDstV[k + j] = pSrcV[y + x];
		}

		k += iDstUVPitch;
	}
}

/**
 * jbyte* -> char*
 *
 * 可读转换，调试用
 *
 */
char* byte2echo(BYTE* buf, int offset, int length) {
	char* result = NULL;
	int linebreak = 8;

	if (NULL != buf) {
		int size = 3 + length * 4 + length / linebreak;
		result = (char*) malloc(sizeof(char) * (size + 1));

		result[0] = '{';
		result[1] = ' ';
		result[size - 1] = '}';
		result[size] = '\0';

		int currentIndex = 2;
		char* dummy = (char*) malloc(sizeof(char) * 2);

		for (int i = 0; i < length; i++) {
			sprintf(dummy, "%02X", buf[i + offset] & 0xFF);
			result[currentIndex] = dummy[0];
			result[currentIndex + 1] = dummy[1];
			result[currentIndex + 2] = ',';
			result[currentIndex + 3] = ' ';
			currentIndex += 4;

			if ((i + 1) % linebreak == 0) {
				if (length - 1 == i) {
					result[currentIndex] = '$';
				} else {
					result[currentIndex] = '\n';
					currentIndex++;
				}
			}
		}

		free(dummy);

	} else {
		LOGD("%s [%d]: jbyte = NULL", LOCATE);
	}

	return result;
}

/**
 * 复制一份，确保结尾是 '\0'
 *
 */
char* byte2char(BYTE* buf, int offset, int length) {
	char* result = NULL;
	if (NULL != buf) {
		result = (char*) malloc(length + 1);
		memset(result, 0, length + 1);
		memcpy(result, buf + offset, length);
	}

	return result;
}

/**
 * 生成已附加的环境指针
 *
 */
JNIEnv* genAttachedEnv(JavaVM* jvm, jint version, jboolean* needDetach) {
	JNIEnv* env = NULL;
	*needDetach = JNI_FALSE;

	if (NULL != jvm) {
		bool isAttached = false;
		jint result = jvm->GetEnv((void**) &env, version);

		switch (result) {
		case JNI_OK:
			isAttached = true;
			break;

		case JNI_EDETACHED:
			// [Neo] new attached
			if (JNI_OK == jvm->AttachCurrentThread(&env, NULL)) {
				isAttached = true;
				*needDetach = JNI_TRUE;
			}
			break;

			// [Neo] generic error
		case JNI_ERR:
			// [Neo] bad version
		case JNI_EVERSION:
		default:
			LOGE("%s [%p]: GetEnv = %d!!", LOCATE_PT, result);
			break;
		}

		if (false == isAttached) {
			env = NULL;
		}
	} else {
		LOGE("%s [%p]: jvm = NULL!!", LOCATE_PT);
	}

	return (JNIEnv*) env;
}

/**
 * jbyteArray -> jbyte*
 *
 */
jbyte* getNativeByte(JNIEnv* env, jbyteArray jarray) {
	jbyte* result = NULL;
	if (NULL != env && NULL != jarray) {
		jsize size = env->GetArrayLength(jarray);

		if (size >= 0) {
			result = getNativeByteByLength(env, jarray, 0, size);
		} else {
			size_t size = sizeof(jbyte) * 8;
			result = (jbyte*) malloc(size);
			memset(result, 0, size);
		}
	} else {
		LOGD("%s [%d]: env = NULL or jarray = NULL", LOCATE);
	}

	return result;
}

/**
 * jbyteArray, offset, length -> jbyte*
 *
 */
jbyte* getNativeByteByLength(JNIEnv* env, jbyteArray jarray, jint offset,
		jint length) {
	jbyte* result = NULL;

	if (NULL != env && NULL != jarray) {
		jsize size = env->GetArrayLength(jarray);

		if (length >= 0 && offset >= 0 && offset + length <= size) {
			jboolean isCopy = JNI_FALSE;
			jbyte* bytes = env->GetByteArrayElements(jarray, &isCopy);

			if (JNI_TRUE == isCopy) {
//				LOGV("copy bytes indeed");
			}

			result = (jbyte*) malloc(sizeof(jbyte) * (length + 1));
			memcpy(result, bytes + offset, length);
			result[length] = '\0';
			env->ReleaseByteArrayElements(jarray, bytes, JNI_ABORT);
		} else {
			size_t s = sizeof(jbyte) * 8;
			result = (jbyte*) malloc(s);
			memset(result, 0, s);
			LOGD(
					"%s [%d]: bad length: (%d+%d) !? (%d)", LOCATE, offset, length, size);
		}
	} else {
		LOGD("%s [%d]: env = NULL or jarray = NULL", LOCATE);
	}

	return result;
}

/**
 * jbyte*, offset, length -> jbyteArray
 *
 */
jbyteArray genByteArray(JNIEnv* env, jbyte* buf, jint offset, jint length) {
	jbyteArray array = env->NewByteArray(length);
	env->SetByteArrayRegion(array, offset, length, buf);
	return array;
}

/**
 * jstring -> char*
 *
 */
char* getNativeChar(JNIEnv* env, jstring jstr) {
	char* result = NULL;
	if (NULL != env && NULL != jstr) {
		jboolean isCopy = JNI_FALSE;
		char* chars = (char*) env->GetStringUTFChars(jstr, &isCopy);
		jint length = env->GetStringUTFLength(jstr);
		result = (char*) malloc(sizeof(char) * (length + 1));
		memcpy(result, chars, length);
		result[length] = '\0';
		env->ReleaseStringUTFChars(jstr, chars);
	} else {
		LOGD("%s [%d]: env = NULL or jstr = NULL", LOCATE);
	}

	return result;
}

/**
 * char*, offset, length -> jstring
 *
 */
jstring genString(JNIEnv* env, const char* buf, int offset, int length) {
	char* dummy = (char*) malloc(sizeof(char) * length + 1);
	memcpy(dummy, buf + offset, length);
	dummy[length] = '\0';

	jstring jstr = env->NewStringUTF(dummy);
	free(dummy);
	return jstr;
}

/**
 * 生成指定大小的 jobjectArray
 *
 */
jobjectArray genJObjectArray(JNIEnv* env, jsize size) {
	jobjectArray jarray = NULL;
	if (NULL != env) {
		jclass clz = env->FindClass("java/lang/Object");
		jarray = env->NewObjectArray(size, clz, NULL);
//	env->SetObjectArrayElement(jarray, i, v);
		env->DeleteLocalRef(clz);
	} else {
		LOGE("%s [%d]: env = NULL!!", LOCATE);
	}

	return jarray;

}
