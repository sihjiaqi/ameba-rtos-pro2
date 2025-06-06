#ifndef _MODULE_VIDEO_H
#define _MODULE_VIDEO_H

#include <stdint.h>
#include <osdep_service.h>
#include "mmf2_module.h"
#include "video_api.h"

#define CMD_VIDEO_SET_PARAMS     	MM_MODULE_CMD(0x00)  // set parameter
#define CMD_VIDEO_GET_PARAMS     	MM_MODULE_CMD(0x01)  // get parameter
#define CMD_VIDEO_SET_HEIGHT		MM_MODULE_CMD(0x02)
#define CMD_VIDEO_SET_WIDTH			MM_MODULE_CMD(0x03)
#define CMD_VIDEO_BITRATE			MM_MODULE_CMD(0x04)
#define CMD_VIDEO_FPS				MM_MODULE_CMD(0x05)
#define CMD_VIDEO_GOP				MM_MODULE_CMD(0x06)
#define CMD_VIDEO_MEMORY_SIZE		MM_MODULE_CMD(0x07)
#define CMD_VIDEO_BLOCK_SIZE		MM_MODULE_CMD(0x08)
#define CMD_VIDEO_MAX_FRAME_SIZE	MM_MODULE_CMD(0x09)
#define CMD_VIDEO_RCMODE			MM_MODULE_CMD(0x0a)
#define CMD_VIDEO_SET_RCPARAM		MM_MODULE_CMD(0x0b)
#define CMD_VIDEO_GET_RCPARAM		MM_MODULE_CMD(0x0c)
#define CMD_VIDEO_INIT_MEM_POOL		MM_MODULE_CMD(0x0d)
#define CMD_VIDEO_FORCE_IFRAME		MM_MODULE_CMD(0x0e)
#define CMD_VIDEO_ISPFPS			MM_MODULE_CMD(0x0f)

#define CMD_VIDEO_STREAMID		    MM_MODULE_CMD(0x10)
#define CMD_VIDEO_FORMAT			MM_MODULE_CMD(0x11)
#define CMD_VIDEO_BPS               MM_MODULE_CMD(0x12)
#define CMD_VIDEO_SNAPSHOT          MM_MODULE_CMD(0x13)
#define CMD_VIDEO_SNAPSHOT_CB       MM_MODULE_CMD(0x14)
#define CMD_VIDEO_YUV               MM_MODULE_CMD(0x15)
#define CMD_ISP_SET_RAWFMT          MM_MODULE_CMD(0x16)
#define CMD_VIDEO_PRINT_INFO        MM_MODULE_CMD(0x17)
#define CMD_VIDEO_SET_MULTI_RCCTRL	MM_MODULE_CMD(0x18)
#define CMD_VIDEO_GET_MULTI_RCCTRL	MM_MODULE_CMD(0x19)
#define CMD_VIDEO_SET_CAP_INTVL		MM_MODULE_CMD(0x1a)  //capture every n seconds

#define CMD_VIDEO_APPLY				MM_MODULE_CMD(0x20)  // apply setting
#define CMD_VIDEO_UPDATE			MM_MODULE_CMD(0x21)  // update new setting
#define CMD_VIDEO_STREAM_STOP		MM_MODULE_CMD(0x23)  // stop stream
#define CMD_VIDEO_SET_VOE_HEAP		MM_MODULE_CMD(0x24)
#define CMD_VIDEO_SET_TIMESTAMP_OFFSET		MM_MODULE_CMD(0x25)
#define CMD_VIDEO_EN_DBG_TS_INFO	MM_MODULE_CMD(0x26)
#define CMD_VIDEO_SHOW_DBG_TS_INFO	MM_MODULE_CMD(0x27)
#define CMD_VIDEO_SET_SENSOR_ID     MM_MODULE_CMD(0x28)

#define CMD_SNAPSHOT_ENCODE_CB		MM_MODULE_CMD(0x30)

#define CMD_VIDEO_MD_SET_ROI		MM_MODULE_CMD(0x31)
#define CMD_VIDEO_MD_SET_SENSITIVITY	MM_MODULE_CMD(0x32)
#define CMD_VIDEO_MD_START			MM_MODULE_CMD(0x33)
#define CMD_VIDEO_MD_STOP			MM_MODULE_CMD(0x34)

#define CMD_VIDEO_META_CB		    MM_MODULE_CMD(0x35)
#define CMD_VIDEO_GET_META_DATA	    MM_MODULE_CMD(0x36)
#define CMD_VIDEO_SET_PRIVATE_MASK  MM_MODULE_CMD(0x37)

#define CMD_VIDEO_BPS_STBL_CTRL_EN				MM_MODULE_CMD(0x40)
#define CMD_VIDEO_SET_BPS_STBL_CTRL_PARAMS		MM_MODULE_CMD(0x41)
#define CMD_VIDEO_SET_BPS_STBL_CTRL_FPS_STG		MM_MODULE_CMD(0x42)
#define CMD_VIDEO_SET_BPS_STBL_CTRL_GOP_STG		MM_MODULE_CMD(0x43)
#define CMD_VIDEO_GET_CURRENT_BITRATE			MM_MODULE_CMD(0x44)

#define CMD_VIDEO_GET_REMAIN_QUEUE_LENGTH		MM_MODULE_CMD(0x45)
#define CMD_VIDEO_GET_MAX_QP					MM_MODULE_CMD(0x46)
#define CMD_VIDEO_SET_MAX_QP					MM_MODULE_CMD(0x47)

#define CMD_VIDEO_SET_SPS_PPS_INFO  MM_MODULE_CMD(0x48)
#define CMD_VIDEO_GET_SPS_PPS_INFO  MM_MODULE_CMD(0x49)


#define CMD_VIDEO_SET_EXT_INPUT     MM_MODULE_CMD(0x50)

#define CMD_VIDEO_SPS_CB            MM_MODULE_CMD(0x51)

#define CMD_VIDEO_PRE_INIT_PARM     MM_MODULE_CMD(0x52)
#define CMD_VIDEO_GET_PRE_INIT_PARM	MM_MODULE_CMD(0x53)
#define CMD_VIDEO_PRE_INIT_LOAD  	MM_MODULE_CMD(0x54)
#define CMD_VIDEO_PRE_INIT_SAVE  	MM_MODULE_CMD(0x55)

#define MMF_VIDEO_DBG_TS_MAX_CNT	5
#define MMF_VIDEO_DEFAULT_META_CB			(0xFFFFFFFF)

typedef struct dbg_ts_info {
	int timestamp_cnt;
	uint32_t timestamp[MMF_VIDEO_DBG_TS_MAX_CNT];
} dbg_ts_info_t;

typedef struct video_bps_stats_s {
	uint32_t cnt_br;
	uint32_t sum_br;
	int cur_bps;
} video_bps_stats_t;

typedef struct video_ctx_s {
	void *parent;
	int iq_addr;
	int sensor_addr;
	hal_video_adapter_t *v_adp;
	void *mem_pool;

	video_params_t params;
	int (*snapshot_cb)(uint32_t, uint32_t);
	void (*change_parm_cb)(void *);
	video_state_t state;
	uint32_t timestamp_offset;
	video_meta_t meta_data;
	void (*meta_cb)(void *);
	void (*sps_pps_cb)(void *);
	video_bps_stats_t bps_stats;
	uint64_t frame_cnt;
	int frame_drop_interval;

	dbg_ts_info_t *dbg_ts_info;
} video_ctx_t;

extern mm_module_t video_module;


int video_voe_presetting(int v1_enable, int v1_w, int v1_h, int v1_bps, int v1_shapshot,
						 int v2_enable, int v2_w, int v2_h, int v2_bps, int v2_shapshot,
						 int v3_enable, int v3_w, int v3_h, int v3_bps, int v3_shapshot,
						 int v4_enable, int v4_w, int v4_h);

int video_voe_presetting_by_params(const void *v1_params, int v1_jpg_only_shapshot, const void *v2_params, int v2_jpg_only_shapshot, const void *v3_params,
								   int v3_jpg_only_shapshot, const void *v4_params);
								   
int video_extra_voe_presetting(int originl_heapsize, int vext_enable, int vext_w, int vext_h, int vext_bps, int vext_shapshot);


void video_voe_release(void);
void video_set_sensor_id(int SensorName);
void video_setup_sensor(void *sensor_setup_cb);
void video_show_fps(int enable);
int video_get_cb_fps(int chn);
void video_set_fps_dropframe_mode(int drop_frame);
#endif