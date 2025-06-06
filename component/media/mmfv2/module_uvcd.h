#ifndef _MODULE_USBD_H
#define _MODULE_USBD_H

#include <stdint.h>
#include <osdep_service.h>
#include "mmf2_module.h"
#include "uvc/inc/usbd_uvc_desc.h"

#define BAYER_TYPE_BEFORE_BLC   1
#define BAYER_TYPE_AFTER_BLC    2
#define BAYER_TYPE_AFTER_LSC    3
#define BAYER_TYPE_AFTER_DNO    4

#define CMD_UVCD_CALLBACK_SET     	MM_MODULE_CMD(0x00)  // set parameter
#define CMD_UVCD_CALLBACK_GET     	MM_MODULE_CMD(0x01)  // get parameter
#define CMD_UVCD_STOP     	        MM_MODULE_CMD(0x02)  // get parameter

struct uvc_format {
	int width;
	int height;
	int format;
	int fps;
	int state;
	int isp_format;//1:YUV420 2:YUV422 3:Bayer
	int ldc;//0:Disable 1:Enable
	int bayer_type;
	_sema uvcd_change_sema;
	int init;//It only support whether the uvc is first init 0:Not initialized 1: Initialized
};

typedef struct uvcd_ctx_s {
	void *parent;
} uvcd_ctx_t;

extern mm_module_t uvcd_module;

#endif