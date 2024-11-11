#include "ai_glass_media.h"
#include "module_video.h"
#include "sensor.h"
#include "ftl_common_api.h"

#define MIN_VIDEO_WIDTH     80
#define MIN_VIDEO_HEIGHT    60

#define MIN_RECORD_WIDTH    MIN_VIDEO_WIDTH
#define MIN_RECORD_HEIGHT   MIN_VIDEO_HEIGHT
#define MIN_AISNAP_WIDTH    MIN_VIDEO_WIDTH
#define MIN_AISNAP_HEIGHT   MIN_VIDEO_HEIGHT
#define MIN_LIFESNAP_WIDTH  MIN_VIDEO_WIDTH
#define MIN_LIFESNAP_HEIGHT MIN_VIDEO_HEIGHT

#define IS_VALID_RECORD_TYPE(value) \
	((value) >= 0 && \
     (value) <= 7)

#define IS_VALID_RECORD_WIDTH(value) \
    ((value) >= MIN_RECORD_WIDTH && \
     (value) <= MAX_RECORD_WIDTH)

#define IS_VALID_RECORD_HEIGHT(value) \
    ((value) >= MIN_RECORD_HEIGHT && \
     (value) <= MAX_RECORD_HEIGHT)

#define IS_VALID_RECORD_BPS(value) \
	((value) >= MIN_RECORD_BPS && \
     (value) <= MAX_RECORD_BPS)

#define IS_VALID_RECORD_FPS(value) \
	((value) >= MIN_RECORD_FPS && \
     (value) <= MAX_RECORD_FPS)

#define IS_VALID_RECORD_GOP(value) \
	((value) >= MIN_RECORD_GOP && \
     (value) <= MAX_RECORD_GOP)

#define IS_VALID_RECORD_RCMODE(value) \
	 ((value) == 1 || (value) == 2 )
#define IS_VALID_RECORD_RECTIME(value) \
    ((value) >= 0 )
#define IS_VALID_SNAP_TYPE(value) \
    ((value) == 2 )
#define IS_VALID_AISNAP_WIDTH(value) \
    ((value) >= MIN_AISNAP_WIDTH && \
     (value) <= MAX_AISNAP_WIDTH)
#define IS_VALID_AISNAP_HEIGHT(value) \
    ((value) >= MIN_AISNAP_HEIGHT && \
     (value) <= MAX_AISNAP_HEIGHT)
#define IS_VALID_SNAP_QVALUE(value) \
    ((value) >= 1 && \
     (value) <= 9)
#define IS_VALID_LIFESNAP_WIDTH(value) \
    ((value) >= MIN_LIFESNAP_WIDTH && \
     (value) <= MAX_LIFESNAP_WIDTH)
#define IS_VALID_LIFESNAP_HEIGHT(value) \
    ((value) >= MIN_LIFESNAP_HEIGHT && \
     (value) <= MAX_LIFESNAP_HEIGHT)

static ai_glass_record_param_t record_params = {
	.type = DEFAULT_RECORD_TYPE,
	.width = DEFAULT_RECORD_WIDTH,
	.height = DEFAULT_RECORD_HEIGHT,
	.bps = DEFAULT_RECORD_BPS,
	.fps = DEFAULT_RECORD_FPS,
	.gop = DEFAULT_RECORD_GOP,
	.roi = {
		.xmin = 0,
		.ymin = 0,
		.xmax = 0,
		.ymax = 0,
	},
	.minQp = DEFAULT_RECORD_MINQP,
	.maxQp = DEFAULT_RECORD_MAXQP,
	.rotation = DEFAULT_RECORD_ROTATION,
	.rc_mode = DEFAULT_RECORD_RCMODE,
	.record_length = DEFAULT_RECORD_RECTIME,
};

static ai_glass_snapshot_param_t ai_snapshot_params = {
	.type = DEFAULT_AISNAP_TYPE,
	.width = DEFAULT_AISNAP_WIDTH,
	.height = DEFAULT_AISNAP_HEIGHT,
	.jpeg_qlevel = DEFAULT_AISNAP_QLEVEL,
	.roi = {
		.xmin = 0,
		.ymin = 0,
		.xmax = 0,
		.ymax = 0,
	},
	.minQp = DEFAULT_AISNAP_MINQP,
	.maxQp = DEFAULT_AISNAP_MAXQP,
	.rotation = DEFAULT_AISNAP_ROTATION,
};

static ai_glass_snapshot_param_t life_snapshot_params = {
	.type = DEFAULT_LIFESNAP_TYPE,
	.width = DEFAULT_LIFESNAP_WIDTH,
	.height = DEFAULT_LIFESNAP_HEIGHT,
	.jpeg_qlevel = DEFAULT_LIFESNAP_QLEVEL,
	.roi = {
		.xmin = 0,
		.ymin = 0,
		.xmax = 0,
		.ymax = 0,
	},
	.minQp = DEFAULT_LIFESNAP_MINQP,
	.maxQp = DEFAULT_LIFESNAP_MAXQP,
	.rotation = DEFAULT_LIFESNAP_ROTATION,
};

static int record_data_check(const ai_glass_record_param_t *params)
{
	// Todo: check the parameters
	if (params) {
		if (!IS_VALID_RECORD_TYPE(params->type)) {
			return MEDIA_INVALID_VTYPE;
		}
		if (!IS_VALID_RECORD_WIDTH(params->width)) {
			return MEDIA_INVALID_WIDTH;
		}
		if (!IS_VALID_RECORD_HEIGHT(params->height)) {
			return MEDIA_INVALID_HEIGHT;
		}
		if (!IS_VALID_RECORD_BPS(params->bps)) {
			return MEDIA_INVALID_BPS;
		}
		if (!IS_VALID_RECORD_FPS(params->fps)) {
			return MEDIA_INVALID_FPS;
		}
		if (!IS_VALID_RECORD_GOP(params->gop)) {
			return MEDIA_INVALID_GOP;
		}
		if (!IS_VALID_RECORD_RCMODE(params->rc_mode)) {
			return MEDIA_INVALID_RCMODE;
		}
		if (!IS_VALID_RECORD_RECTIME(params->record_length)) {
			return MEDIA_INVALID_RECTIME;
		}
	} else {
		return MEDIA_FAIL;
	}

	return MEDIA_OK;
}

static int ai_snapshot_data_check(const ai_glass_snapshot_param_t *params)
{
	// Todo: check the parameters
	if (params) {
		if (!IS_VALID_SNAP_TYPE(params->type)) {
			return MEDIA_INVALID_SNAP_TYPE;
		}
		if (!IS_VALID_AISNAP_WIDTH(params->width)) {
			return MEDIA_INVALID_WIDTH;
		}
		if (!IS_VALID_AISNAP_HEIGHT(params->height)) {
			return MEDIA_INVALID_HEIGHT;
		}
		if (!IS_VALID_SNAP_QVALUE(params->jpeg_qlevel)) {
			return MEDIA_INVALID_QVALUE;
		}
	} else {
		return MEDIA_FAIL;
	}

	return MEDIA_OK;
}

static int life_snapshot_data_check(const ai_glass_snapshot_param_t *params)
{
	// Todo: check the parameters
	if (params) {
		if (!IS_VALID_SNAP_TYPE(params->type)) {
			return MEDIA_INVALID_SNAP_TYPE;
		}
		if (!IS_VALID_LIFESNAP_WIDTH(params->width)) {
			return MEDIA_INVALID_WIDTH;
		}
		if (!IS_VALID_LIFESNAP_HEIGHT(params->height)) {
			return MEDIA_INVALID_HEIGHT;
		}
		if (!IS_VALID_SNAP_QVALUE(params->jpeg_qlevel)) {
			return MEDIA_INVALID_QVALUE;
		}
	} else {
		return MEDIA_FAIL;
	}

	return MEDIA_OK;
}

static int media_set_record_params(const ai_glass_record_param_t *params)
{
	int ret = record_data_check(params);
	if (ret == MEDIA_OK) {
		memcpy(&record_params, params, sizeof(ai_glass_record_param_t));
	}
	return ret;
}

static int media_set_ai_snapshot_params(const ai_glass_snapshot_param_t *params)
{
	int ret = ai_snapshot_data_check(params);
	if (ret == MEDIA_OK) {
		memcpy(&ai_snapshot_params, params, sizeof(ai_glass_snapshot_param_t));
	}
	return ret;
}

static int media_set_life_snapshot_params(const ai_glass_snapshot_param_t *params)
{
	int ret = life_snapshot_data_check(params);
	if (ret == MEDIA_OK) {
		memcpy(&life_snapshot_params, params, sizeof(ai_glass_snapshot_param_t));
	}
	return ret;
}

void print_record_data(const ai_glass_record_param_t *params)
{
	printf("record_params print\r\n");
	printf("type = %u\r\n", params->type);
	printf("width = %u\r\n", params->width);
	printf("height = %u\r\n", params->height);
	printf("bps = %lu\r\n", params->bps);
	printf("fps = %u\r\n", params->fps);
	printf("gop = %u\r\n", params->gop);
	printf("roi.xmin = %lu\r\n", params->roi.xmin);
	printf("roi.ymin = %lu\r\n", params->roi.ymin);
	printf("roi.xmax = %lu\r\n", params->roi.xmax);
	printf("roi.ymax = %lu\r\n", params->roi.ymax);
	printf("minQp = %u\r\n", params->minQp);
	printf("maxQp = %u\r\n", params->maxQp);
	printf("rotation = %u\r\n", params->rotation);
	printf("rc_mode = %u\r\n", params->rc_mode);
	printf("record_length = %u\r\n", params->record_length);
}

void print_snapshot_data(const ai_glass_snapshot_param_t *params)
{
	printf("snapshot_params print\r\n");
	printf("type = %u\r\n", params->type);
	printf("width = %lu\r\n", params->width);
	printf("height = %lu\r\n", params->height);
	printf("jpeg_qlevel = %u\r\n", params->jpeg_qlevel);
	printf("roi.xmin = %lu\r\n", params->roi.xmin);
	printf("roi.ymin = %lu\r\n", params->roi.ymin);
	printf("roi.xmax = %lu\r\n", params->roi.xmax);
	printf("roi.ymax = %lu\r\n", params->roi.ymax);
	printf("minQp = %u\r\n", params->minQp);
	printf("maxQp = %u\r\n", params->maxQp);
	printf("rotation = %u\r\n", params->rotation);
}

int media_get_record_params(ai_glass_record_param_t *params)
{
	if (params) {
		memcpy(params, &record_params, sizeof(ai_glass_record_param_t));
		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

int media_get_ai_snapshot_params(ai_glass_snapshot_param_t *params)
{
	if (params) {
		memcpy(params, &ai_snapshot_params, sizeof(ai_glass_snapshot_param_t));
		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

int media_get_life_snapshot_params(ai_glass_snapshot_param_t *params)
{
	if (params) {
		memcpy(params, &life_snapshot_params, sizeof(ai_glass_snapshot_param_t));
		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

int media_update_record_params(const ai_glass_record_param_t *params)
{
	if (media_set_record_params(params) == MEDIA_OK) {
		// update data to flash
		unsigned char *record_buf = malloc(2048); //Allocate a 2KB buffer
		unsigned int flash_addr = 0;

		if (sys_get_boot_sel() == 0) {
			flash_addr = NOR_FLASH_RECORD;
		} else {
			// Placeholder for NAND FLASH ADDR in future
		}
		if (record_buf == NULL) {
			printf("It can't get the record buffer\r\n");
			return MEDIA_FAIL;
		}

		memset(record_buf, 0x00, 2048);
		record_buf[0] = 'R';  // Add tag for identification (Record params)
		record_buf[1] = 'E';
		record_buf[2] = 'C';
		record_buf[3] = 'D';

		memcpy(record_buf + 4, params, sizeof(ai_glass_record_param_t));

		ftl_common_write(flash_addr, record_buf, 2048);
		memset(record_buf, 0xff, 2048);
		ftl_common_read(flash_addr, record_buf, 2048);
		ai_glass_record_param_t *read_data = (ai_glass_record_param_t *)(record_buf + 4);
		printf("[FLASH]type: %u, width: %u, height:%u, bps: %u, fps: %u, gop: %u, roi_xmin: %u, roi_ymin: %u, roi_xmax: %u, roi_ymax: %u, minQp: %u, maxQp: %u, rotation: %u, rc_mode: %u, record_length:%u\r\n",
			   read_data->type, read_data->width, read_data->height, read_data->bps, read_data->fps, read_data->gop, read_data->roi.xmin, read_data->roi.ymin,
			   read_data->roi.xmax, read_data->roi.ymax, read_data->minQp, read_data->maxQp, read_data->rotation, read_data->rc_mode, read_data->record_length);

		if (record_buf) {
			free(record_buf);
		}

		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

int media_update_record_time(uint16_t record_length)
{
	ai_glass_record_param_t temp_record_params = {0};
	media_get_record_params(&temp_record_params);
	temp_record_params.record_length = record_length;
	if (media_update_record_params(&temp_record_params) == MEDIA_OK) {
		// update data to flash has been done by media_update_record_params.
		return MEDIA_OK;
	}

	return MEDIA_FAIL;
}

int media_update_ai_snapshot_params(const ai_glass_snapshot_param_t *params)
{
	if (media_set_ai_snapshot_params(params) == MEDIA_OK) {
		// update data to flash
		unsigned char *ai_snap_buf = malloc(2048); //Allocate a 2KB buffer
		unsigned int flash_addr = 0;
		if (sys_get_boot_sel() == 0) {
			flash_addr = NOR_FLASH_AI_SNAPSHOT;
		} else {
			// Placeholder for NAND FLASH ADDR in future
		}
		if (ai_snap_buf == NULL) {
			printf("It can't get the ai snapshot buffer\r\n");
			return MEDIA_FAIL;
		}

		memset(ai_snap_buf, 0x00, 2048);
		ai_snap_buf[0] = 'A';  // Add tag for identification (AI snapshot params)
		ai_snap_buf[1] = 'I';
		ai_snap_buf[2] = 'S';
		ai_snap_buf[3] = 'N';
		ai_snap_buf[4] = 'A';
		ai_snap_buf[5] = 'P';

		memcpy(ai_snap_buf + 6, params, sizeof(ai_glass_snapshot_param_t));
		ftl_common_write(flash_addr, ai_snap_buf, 2048);
		memset(ai_snap_buf, 0xff, 2048);
		ftl_common_read(flash_addr, ai_snap_buf, 2048);
		ai_glass_snapshot_param_t *read_data = (ai_glass_record_param_t *)(ai_snap_buf + 6);
		printf("[FLASH_AISNAP]type: %u, width: %u, height:%u, jpeg_qlevel: %u, roi_xmin: %u, roi_ymin: %u, roi_xmax: %u, roi_ymax: %u, minQp: %u, maxQp: %u, rotation: %u\r\n",
			   read_data->type, read_data->width, read_data->height, read_data->jpeg_qlevel, read_data->roi.xmin, read_data->roi.ymin, read_data->roi.xmax,
			   read_data->roi.ymax, read_data->minQp, read_data->maxQp, read_data->rotation);
		if (ai_snap_buf) {
			free(ai_snap_buf);
		}
		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

int media_update_life_snapshot_params(const ai_glass_snapshot_param_t *params)
{
	if (media_set_life_snapshot_params(params) == MEDIA_OK) {
		// update data to flash
		unsigned char *lifetime_snap_buf = malloc(2048); //Allocate a 2KB buffer
		unsigned int flash_addr = 0;
		if (sys_get_boot_sel() == 0) {
			flash_addr = NOR_FLASH_LIFE_SNAPSHOT;
		} else {
			// Placeholder for NAND FLASH ADDR in future
		}
		if (lifetime_snap_buf == NULL) {
			printf("It can't get the lifetime snapshot buffer\r\n");
			return MEDIA_FAIL;
		}

		memset(lifetime_snap_buf, 0x00, 2048);
		lifetime_snap_buf[0] = 'L';  // Add tag for identification (lifetime snapshot params)
		lifetime_snap_buf[1] = 'I';
		lifetime_snap_buf[2] = 'F';
		lifetime_snap_buf[3] = 'E';
		lifetime_snap_buf[4] = 'S';
		lifetime_snap_buf[5] = 'N';
		lifetime_snap_buf[6] = 'A';
		lifetime_snap_buf[7] = 'P';

		memcpy(lifetime_snap_buf + 8, params, sizeof(ai_glass_snapshot_param_t));
		ftl_common_write(flash_addr, lifetime_snap_buf, 2048);
		memset(lifetime_snap_buf, 0xff, 2048);
		ftl_common_read(flash_addr, lifetime_snap_buf, 2048);
		ai_glass_snapshot_param_t *read_data = (ai_glass_record_param_t *)(lifetime_snap_buf + 8);
		printf("[FLASH_LIFESNAP]type: %u, width: %u, height:%u, jpeg_qlevel: %u, roi_xmin: %u, roi_ymin: %u, roi_xmax: %u, roi_ymax: %u, minQp: %u, maxQp: %u, rotation: %u\r\n",
			   read_data->type, read_data->width, read_data->height, read_data->jpeg_qlevel, read_data->roi.xmin, read_data->roi.ymin, read_data->roi.xmax,
			   read_data->roi.ymax, read_data->minQp, read_data->maxQp, read_data->rotation);
		if (lifetime_snap_buf) {
			free(lifetime_snap_buf);
		}
		return MEDIA_OK;
	}
	return MEDIA_FAIL;
}

#include "video_boot.h"
#define OPEN_CHANNEL 0
#define OPEN_STREAM STREAM_V1
static mm_context_t *video_fake_ctx = NULL;
static video_params_t video_fake_params = {
	.stream_id = OPEN_CHANNEL,
	.type = VIDEO_H264,
	.width = 176,
	.height = 144,
	.bps = 1024 * 1024,
	.fps = 6,
	.gop = 6,
	.rc_mode = 2,
	.use_static_addr = 1,
	.direct_output = 1
};

void initial_media_parameters(void)
{
	video_boot_stream_t *isp_fcs_info;
	video_get_fcs_info(&isp_fcs_info);//Get the fcs info
	int voe_heap_size = 0;

	if (isp_fcs_info->fcs_status) {
		printf("==================fcs on==============\r\n");
		voe_heap_size = video_voe_presetting(1, isp_fcs_info->video_params[STREAM_V1].width, isp_fcs_info->video_params[STREAM_V1].height,
											 isp_fcs_info->video_params[STREAM_V1].bps, 1, 1, isp_fcs_info->video_params[STREAM_V2].width, isp_fcs_info->video_params[STREAM_V2].height,
											 isp_fcs_info->video_params[STREAM_V2].bps, 1, 0, isp_fcs_info->video_params[STREAM_V3].width, isp_fcs_info->video_params[STREAM_V3].height,
											 isp_fcs_info->video_params[STREAM_V3].bps, 1, 0, 0, 0);
		video_fake_params.bps = isp_fcs_info->video_params[OPEN_STREAM].bps;
		video_fake_params.width = isp_fcs_info->video_params[OPEN_STREAM].width;
		video_fake_params.height = isp_fcs_info->video_params[OPEN_STREAM].height;
		video_fake_params.fps = isp_fcs_info->video_params[OPEN_STREAM].fps;
		video_fake_params.gop = isp_fcs_info->video_params[OPEN_STREAM].gop;
		video_fake_ctx = mm_module_open(&video_module);
		if (video_fake_ctx) {
			mm_module_ctrl(video_fake_ctx, CMD_VIDEO_SET_PARAMS, (int)&video_fake_params);
			mm_module_ctrl(video_fake_ctx, MM_CMD_SET_QUEUE_LEN, 60);
			mm_module_ctrl(video_fake_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
			mm_module_ctrl(video_fake_ctx, CMD_VIDEO_APPLY, OPEN_CHANNEL);
		} else {
			printf("video open fail\n\r");
		}
	} else {
		printf("==================fcs off==============\r\n");
		voe_heap_size = video_voe_presetting(0, 0, 0, 0, 0, 1, sensor_params[USE_SENSOR].sensor_width, sensor_params[USE_SENSOR].sensor_height, MAX_RECORD_BPS, 1, 0,
											 sensor_params[USE_SENSOR].sensor_width, sensor_params[USE_SENSOR].sensor_height, 0, 1, 0, 0, 0);
	}
	printf("\r\n voe heap size = %d\r\n", voe_heap_size);
#if 0
	// For testing we do not use the temp value
	// Todo: get data from the flash first and store in temp data
	ai_glass_record_param_t temp_record_parames = {0};
	ai_glass_snapshot_param_t temp_ai_snap_parames = {0};
	ai_glass_snapshot_param_t temp_life_snap_parames = {0};

	media_set_record_params();
#endif
}