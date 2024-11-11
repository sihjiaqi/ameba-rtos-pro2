#ifndef _AI_GLASS_MEDIA_H
#define _AI_GLASS_MEDIA_H
#include <platform_opts.h>
#include "FreeRTOS.h"
#include "task.h"
#include "sensor.h"

#define WIDTH_2K    2560
#define HEIGHT_2K   1440
#define WIDTH_5M    2592
#define HEIGHT_5M   1944
#define WIDTH_8M    3264
#define HEIGHT_8M   2448
#define WIDTH_12M   4096
#define HEIGHT_12M  3072

enum {
	MEDIA_INVALID_SNAP_TYPE = -11,  // MEDIA_INVALID_SNAPSHOT_TYPE
	MEDIA_INVALID_RECTIME   = -10,  // MEDIA_INVALID_RECTIME
	MEDIA_INVALID_RCMODE    = -9,   // MEDIA_INVALID_RCMODE
	MEDIA_INVALID_GOP       = -8,   // MEDIA_INVALID_GOP
	MEDIA_INVALID_FPS       = -7,   // MEDIA_INVALID_FPS
	MEDIA_INVALID_BPS       = -6,   // MEDIA_INVALID_BPS
	MEDIA_INVALID_VTYPE     = -5,   // MEDIA_INVALID_VIDEO_TYPE
	MEDIA_INVALID_QVALUE    = -4,   // MEDIA_INVALID_QVALUE
	MEDIA_INVALID_HEIGHT    = -3,   // MEDIA_HEIGHT_INVALID
	MEDIA_INVALID_WIDTH     = -2,   // MEDIA_WIDTH_INVALID
	MEDIA_FAIL              = -1,   // MEDIA_FAIL
	MEDIA_OK                = 0,    // MEDIA_OK
};

typedef enum {
	STATE_IDLE,          // 0
	STATE_RECORDING,     // 1
	STATE_END_RECORDING, // 2
	STATE_ERROR,         // Add more states if needed
} MP4State;

typedef struct ai_glass_record_param_s {
	uint8_t     type;
	uint16_t    width;
	uint16_t    height;
	uint32_t    bps;
	uint16_t    fps;
	uint16_t    gop;
	struct {
		uint32_t    xmin;
		uint32_t    ymin;
		uint32_t    xmax;
		uint32_t    ymax;
	} roi;
	uint16_t    minQp;
	uint16_t    maxQp;
	uint8_t     rotation;
	uint8_t     rc_mode;
	uint16_t    record_length;
} ai_glass_record_param_t;

typedef struct ai_glass_snapshot_param_s {
	uint8_t     type; // JPEG for sure
	uint32_t    width;
	uint32_t    height;
	uint8_t     jpeg_qlevel;
	struct {
		uint32_t    xmin;
		uint32_t    ymin;
		uint32_t    xmax;
		uint32_t    ymax;
	} roi;
	uint16_t    minQp;
	uint16_t    maxQp;
	uint8_t     rotation;
} ai_glass_snapshot_param_t;

#define MAX_LIFESNAP_WIDTH          WIDTH_12M
#define MAX_LIFESNAP_HEIGHT         HEIGHT_12M
#define MAX_AISNAP_WIDTH            sensor_params[USE_SENSOR].sensor_width
#define MAX_AISNAP_HEIGHT           sensor_params[USE_SENSOR].sensor_height
#define MAX_RECORD_WIDTH            WIDTH_2K
#define MAX_RECORD_HEIGHT           HEIGHT_2K
#define MAX_RECORD_BPS              (4*1024*1024)
#define MIN_RECORD_BPS              (512*1024)
#define MAX_RECORD_FPS              sensor_params[USE_SENSOR].sensor_fps
#define MIN_RECORD_FPS              6
#define MAX_RECORD_GOP              sensor_params[USE_SENSOR].sensor_fps
#define MIN_RECORD_GOP              6


#define DEFAULT_RECORD_TYPE         VIDEO_H264
#define DEFAULT_RECORD_WIDTH        WIDTH_2K
#define DEFAULT_RECORD_HEIGHT       HEIGHT_2K
#define DEFAULT_RECORD_BPS          (4*1024*1024)
#define DEFAULT_RECORD_FPS          24
#define DEFAULT_RECORD_GOP          24
#define DEFAULT_RECORD_MINQP        0
#define DEFAULT_RECORD_MAXQP        0
#define DEFAULT_RECORD_ROTATION     0
#define DEFAULT_RECORD_RCMODE       2 // 1: CBR, 2: VBR
#define DEFAULT_RECORD_RECTIME      30

#define GET_GSENSOR_PERIOD          10 // in ms

#define DEFAULT_AISNAP_TYPE         VIDEO_JPEG
#define DEFAULT_AISNAP_WIDTH        640
#define DEFAULT_AISNAP_HEIGHT       480
#define DEFAULT_AISNAP_QLEVEL       8
#define DEFAULT_AISNAP_MINQP        0
#define DEFAULT_AISNAP_MAXQP        0
#define DEFAULT_AISNAP_ROTATION     0

#define DEFAULT_LIFESNAP_TYPE       VIDEO_JPEG
#define DEFAULT_LIFESNAP_WIDTH      WIDTH_12M
#define DEFAULT_LIFESNAP_HEIGHT     HEIGHT_12M
#define DEFAULT_LIFESNAP_QLEVEL     8
#define DEFAULT_LIFESNAP_MINQP      0
#define DEFAULT_LIFESNAP_MAXQP      0
#define DEFAULT_LIFESNAP_ROTATION   0

#define FLASH_AI_SNAPSHOT_DATA  (USER_DATA_BASE + 0x0DC00) //Store the AI Glass Snapshot params
#define FLASH_RECORD_DATA       (USER_DATA_BASE + 0x0E800) //Store the AI Glass Record params
#define FLASH_SNAPSHOT_DATA     (USER_DATA_BASE + 0x0F400) //Store the AI Glass Record params

/**
* Nor Flash Snapshot/Record data
*/
#define NOR_FLASH_AI_SNAPSHOT   FLASH_AI_SNAPSHOT_DATA
#define NOR_FLASH_RECORD        FLASH_RECORD_DATA
#define NOR_FLASH_LIFE_SNAPSHOT FLASH_SNAPSHOT_DATA


// Function for media
int media_update_record_params(const ai_glass_record_param_t *params);
int media_update_ai_snapshot_params(const ai_glass_snapshot_param_t *params);
int media_update_life_snapshot_params(const ai_glass_snapshot_param_t *params);
int media_get_record_params(ai_glass_record_param_t *params);
int media_get_ai_snapshot_params(ai_glass_snapshot_param_t *params);
int media_get_life_snapshot_params(ai_glass_snapshot_param_t *params);
void print_record_data(const ai_glass_record_param_t *params);
void print_snapshot_data(const ai_glass_snapshot_param_t *params);
void initial_media_parameters(void);


// ai snapshot
int ai_snapshot_initialize(void);
int ai_snapshot_take(const char *file_name);
int ai_snapshot_deinitialize(void);

// life snapshot
int lifetime_snapshot_initialize(void);
int lifetime_snapshot_take(const char *file_name);
int lifetime_snapshot_deinitialize(void);

// life recording
extern MP4State current_state;
void lifetime_recording_initialize(void);
void lifetime_recording_deinitialize(void);
int media_update_record_time(uint16_t record_length);

#define MAIN_STREAM_ID  1

#endif
