#include "sensor_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "platform_stdlib.h"
#include "isp_api.h"
#include "isp_ctrl_api.h"

#define SW_ALS		0
#define HW_ALS		1
#define ALS_TYPE 	HW_ALS	    /* Choose ALS type */
#define DEBUG_OSD	0           /* Enable debug message through OSD */
#define SS_DELAY	3000        /* Delay time for each als detection cycle */
#define ss_dprintf(level, ...) if(level <= ss_dbg_level) printf(__VA_ARGS__)

static int ss_dbg_level = 0;  /* Ref to ss_dbg_level */
static int en_auto_ir = 0;    /* Enable for auto-ir function*/
static int sw_lux = 0;        /* Lux information from isp*/
static int ir_brightness = 0; /* Ref value to ir pwm*/

/*
	switch function through mode changing
	case 1(ALS_MODE_RGB): switch to RGB mode
	case 2(ALS_MODE_IR_Entry): switch to IR mode
	case 3(ALS_MODE_IR_Stable): Adjust IR LED PWM during IR mode
*/
int sensor_external_set_gray_mode(int enable, int led_level)
{
	int ret = 0;
	ir_brightness = led_level;
	if (enable == ALS_MODE_RGB) {
		ss_dprintf(1, "[SensorService] Switch to RGB Mode\n");
		ir_ctrl_set_brightness_d(led_level);
		ir_cut_enable(1 - enable);
		isp_set_day_night(enable);
		isp_get_day_night(&ret);
		if (ret != enable) {
			isp_set_day_night(enable);
		}
		vTaskDelay(200);
		isp_set_gray_mode(enable);
		if (ret != enable) {
			isp_set_gray_mode(enable);
		}
	} else if (enable == ALS_MODE_IR_Entry) {
		ss_dprintf(1, "[SensorService] Switch to IR Mode\n");
		isp_set_gray_mode(enable);
		isp_get_day_night(&ret);
		if (ret != enable) {
			isp_set_gray_mode(enable);
		}
		ir_cut_enable(1 - enable);
		ir_ctrl_set_brightness_d(led_level);
		vTaskDelay(200);
		isp_set_day_night(enable);
		if (ret != enable) {
			isp_set_day_night(enable);
		}
	} else if (enable == ALS_MODE_IR_Stable) {
		ir_ctrl_set_brightness_d(led_level);
	}
	return ret;
}

void ss_cmd(int type, int index, int *value)
{
	if (type == SS_GET_CMD) {
		if (index == SS_IDX_DBG_LEVEL) {
			*value = ss_dbg_level;
		} else if (index == SS_IDX_EN_AUTO_IR) {
			*value = en_auto_ir;
		} else if (index == SS_IDX_IR_STRENGTH) {
			*value = ir_brightness;
		} else if (index == SS_IDX_SW_LUX) {
			*value = sw_lux;
		}
	} else {
		if (index == SS_IDX_DBG_LEVEL) {
			ss_dbg_level = *value;
		} else if (index == SS_IDX_EN_AUTO_IR) {
			en_auto_ir = *value;
		} else if (index == SS_IDX_IR_STRENGTH) {
			ir_brightness = *value;
		}
	}
}

#define DEF_IR_LED_IDX	2
static void autoir_set_param(auto_ir_config_t *auto_ir_config)
{
	auto_ir_config->ir_led_step[0] = IR_MIN_STRENGTH;
	auto_ir_config->ir_led_step[1] = (IR_MAX_STRENGTH + IR_MIN_STRENGTH) >> 1;
	auto_ir_config->ir_led_step[2] = IR_MAX_STRENGTH;
	auto_ir_config->def_irled_idx = DEF_IR_LED_IDX;
	auto_ir_config->thr_ir_darkder = 200;
	auto_ir_config->thr_ir_brighter = 1000;
}

void autoir_get_param(auto_ir_config_t *auto_ir_config)
{
	printf("[AutoIR] ==== Configuration ====\n");
	printf("[AutoIR] COUNT_IR_LED_STEP = %d\n", COUNT_IR_LED_STEP);
	printf("[AutoIR] def_irled_idx = %d\n", auto_ir_config->def_irled_idx);
	printf("[AutoIR] thr_ir_darkder = %d\n", auto_ir_config->thr_ir_darkder);
	printf("[AutoIR] thr_ir_brighter = %d\n", auto_ir_config->thr_ir_brighter);
	printf("[AutoIR] ==== Step ====\n");
	for (int i = 0; i < COUNT_IR_LED_STEP; i++) {
		printf("[AutoIR] Step[%d] = %d\n", i, auto_ir_config->ir_led_step[i]);
	}
}

static void autoir_flow(auto_ir_config_t auto_ir_config, int gray_mode, short *irled_idx)
{
	int pre_ir_brightness = ir_brightness;
	ss_dprintf(1, "[SensorService][Enter] EN(%d), Mode(%d), irled_idx(%d), ir_brightness(%d), sw_lux(%d)\n", en_auto_ir, gray_mode, *irled_idx, ir_brightness,
			   sw_lux);
	/*IR mode with en_auto_ir, trigger ir-led adjustment*/
	if (en_auto_ir && gray_mode) {
		if (sw_lux < auto_ir_config.thr_ir_darkder) {
			if (*irled_idx == 0) {
				ss_dprintf(1, "[SensorService][Stable] EN(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			} else {
				*irled_idx -= 1;
				if (*irled_idx <= 0) {
					*irled_idx = 0;
				}
				pre_ir_brightness = auto_ir_config.ir_led_step[*irled_idx] ;
				if (!en_auto_ir) {
					pre_ir_brightness = IR_MAX_STRENGTH;
				}
				ss_dprintf(1, "[SensorService][Adjust]EN(%d), irled_idx(%d), pre_ir_brightness(%d)\n", en_auto_ir, *irled_idx, pre_ir_brightness);
			}
		} else if (sw_lux > auto_ir_config.thr_ir_brighter) {
			if (*irled_idx == (COUNT_IR_LED_STEP - 1)) {
				ss_dprintf(1, "[SensorService][Stable] EN(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, ir_brightness);
			} else {
				*irled_idx += 1;
				if (*irled_idx >= COUNT_IR_LED_STEP) {
					*irled_idx = COUNT_IR_LED_STEP - 1;
				}
				pre_ir_brightness = auto_ir_config.ir_led_step[*irled_idx] ;
				if (!en_auto_ir) {
					pre_ir_brightness = IR_MAX_STRENGTH;
				}
				ss_dprintf(1, "[SensorService][Adjust]EN(%d), irled_idx(%d), pre_ir_brightness(%d)\n", en_auto_ir, *irled_idx, pre_ir_brightness);
			}
		}
		/*IR mode without en_auto_ir, reset ir-led to default value*/
	} else if (!en_auto_ir && gray_mode) {
		pre_ir_brightness = auto_ir_config.ir_led_step[DEF_IR_LED_IDX];
		sensor_external_set_gray_mode(ALS_MODE_IR_Stable, ir_brightness);
		ss_dprintf(1, "[SensorService][Force]EN_SmartIR(%d), irled_idx(%d), ir_brightness(%d)\n", en_auto_ir, *irled_idx, pre_ir_brightness);
	}
	/*if current pwm is not equal to pre pwm, update ir-led strength*/
	if (pre_ir_brightness != ir_brightness) {
		ir_brightness = pre_ir_brightness;
		sensor_external_set_gray_mode(ALS_MODE_IR_Stable, ir_brightness);
	}
}

#if(DEBUG_OSD)
#include "osd_api.h"
#include "osd_render.h"
#define LIVESTREAM_CHANNEL 0

void sensor_service_osd(void)
{
	char text_str[80];
	canvas_create_bitmap(LIVESTREAM_CHANNEL, 0, RTS_OSD2_BLK_FMT_1BPP);
	snprintf(text_str, sizeof(text_str), "EN:%1d IR:%3d LUX:%d", en_auto_ir, ir_brightness, sw_lux);
	canvas_set_text(LIVESTREAM_CHANNEL, 0, 10, 10, text_str, COLOR_CYAN);
	canvas_update(LIVESTREAM_CHANNEL, 0, 1);
}
#endif

#if(ALS_TYPE == HW_ALS)
#define THR_COLOR_TO_GRAY	5
#define THR_GRAY_TO_COLOR	20

void sensor_thread(void *param)
{
	int gray_mode = 0;
	float hw_lux;
	int scale = 180;
	auto_ir_config_t auto_ir_config;
	short irled_idx;

	char ifAEStable = ALS_AE_UNSTABLE;
	ambient_light_sensor_init(NULL);
	ambient_light_sensor_power(1);
	autoir_set_param(&auto_ir_config);
	autoir_get_param(&auto_ir_config);
	irled_idx = auto_ir_config.def_irled_idx;

#if(DEBUG_OSD)
	int ch_enable[3] = {1, 0, 0};
	int char_resize_w[3] = {32, 32, 0}, char_resize_h[3] = {32, 32, 0};
	int ch_width[3] = {1280, 0, 0}, ch_height[3] = {720, 0, 0};
	osd_render_dev_init(ch_enable, char_resize_w, char_resize_h);
	osd_render_task_start(ch_enable, ch_width, ch_height);
#endif

	while (1) {
		/*hw-als switch algorithm*/
		hw_lux = (ambient_light_sensor_get_lux(50) * scale) / 100;
		if (!gray_mode && (hw_lux <= THR_COLOR_TO_GRAY)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), HW-Lux(%3.3f) <= THR_COLOR_TO_GRAY(%d)\n", gray_mode, hw_lux, (int)THR_COLOR_TO_GRAY);
			gray_mode = ALS_MODE_IR_Entry;
			if (!en_auto_ir) {
				irled_idx = DEF_IR_LED_IDX;
			} else {
				irled_idx = auto_ir_config.def_irled_idx;
			}
			sensor_external_set_gray_mode(gray_mode, auto_ir_config.ir_led_step[irled_idx]);
		} else if (gray_mode && (hw_lux > THR_GRAY_TO_COLOR)) {
			ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), HW-Lux(%3.3f) >= THR_GRAY_TO_COLOR(%d)\r\n", gray_mode, hw_lux, (int)THR_GRAY_TO_COLOR);
			gray_mode = ALS_MODE_RGB;
			if (en_auto_ir) {
				auto_ir_config.def_irled_idx = irled_idx;
			}
			sensor_external_set_gray_mode(gray_mode, 0);
		} else {
			ss_dprintf(1, "[SensorService] Stay:Mode(%d), HW-Lux(%3.3f)\n", gray_mode, hw_lux);
		}

		/*auto-ir algorithm*/
		if (en_auto_ir && gray_mode) {
			ifAEStable = isp_get_ifAEstable(&sw_lux, 500);
			ss_dprintf(1, "[SensorService] ifAEStable(%d), SW-Lux(%d)\n", ifAEStable, sw_lux);
			if (ifAEStable == ALS_AE_UNSTABLE) {
				gray_mode = ALS_MODE_IR_Entry;
			} else {
				gray_mode = ALS_MODE_IR_Stable;
				autoir_flow(auto_ir_config, gray_mode, &irled_idx);
			}
		} else if (!en_auto_ir && gray_mode) {
			if (irled_idx != DEF_IR_LED_IDX) {
				irled_idx = DEF_IR_LED_IDX;
				sensor_external_set_gray_mode(gray_mode, auto_ir_config.ir_led_step[irled_idx]);
			}
		}
#if(DEBUG_OSD)
		sensor_service_osd();
#endif
		vTaskDelay(SS_DELAY);
	}
}
#else

static void als_set_param(als_config_t *als_config)
{
	char mask[ALS_MAX_COL * ALS_MAX_ROW] = {
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0,
		1, 1, 1, 1, 1,
		0, 1, 1, 1, 0,
		0, 0, 1, 0, 0
	};
	als_config->Thr_Color_to_Gray = 12000;
	als_config->Thr_Gray_to_Color = 4000;
	als_config->Thr_Color_Ratio = 192;
	als_config->Thr_Valid_Block = 6;
	for (int i = 0; i < ALS_MAX_COL * ALS_MAX_ROW; i++) {
		als_config->Mask[i] = mask[i];
	}
}

void als_get_param(als_config_t *als_config)
{
	printf("[ALS] ==== Configuration ====\n");
	printf("[ALS] THR_COLOR_TO_GRAY=%d\n", als_config->Thr_Color_to_Gray);
	printf("[ALS] THR_GRAY_TO_COLOR=%d\n", als_config->Thr_Gray_to_Color);
	printf("[ALS] THR_COLOR_RATIO  =%d\n", als_config->Thr_Color_Ratio);
	printf("[ALS] THR_VALID_BLOCK  =%d\n", als_config->Thr_Valid_Block);
	printf("[ALS] ==== MASK ====\n");
	for (int i = 0; i < ALS_MAX_ROW; i++) {
		printf("[ALS] ");
		for (int j = 0; j < ALS_MAX_ROW; j++) {
			printf("%2d ", als_config->Mask[i * 5 + j]);
		}
		printf("\n");
	}
}

void sensor_thread(void *param)
{
	als_data_t als_data;
	als_config_t als_config;
	auto_ir_config_t auto_ir_config;
	short irled_idx;
	char ifAEStable = ALS_AE_UNSTABLE;
	int gray_mode = ALS_MODE_IR_Stable;
	als_set_param(&als_config);
	als_get_param(&als_config);
	autoir_set_param(&auto_ir_config);
	autoir_get_param(&auto_ir_config);
	irled_idx = auto_ir_config.def_irled_idx;

#if(DEBUG_OSD)
	int ch_enable[3] = {1, 0, 0};
	int char_resize_w[3] = {32, 32, 0}, char_resize_h[3] = {32, 32, 0};
	int ch_width[3] = {1280, 1280, 0}, ch_height[3] = {720, 720, 0};
	osd_render_dev_init(ch_enable, char_resize_w, char_resize_h);
	osd_render_task_start(ch_enable, ch_width, ch_height);
#endif

	while (1) {
		/*sw-als switch algorithm*/
		ifAEStable = isp_get_ifAEstable(&sw_lux, 500);
		ss_dprintf(1, "[SensorService] ifAEStable(%d), Lux(%d)\n", ifAEStable, sw_lux);
		if (ifAEStable == ALS_AE_UNSTABLE && gray_mode == ALS_MODE_IR_Stable) {
			gray_mode = ALS_MODE_IR_Entry;
			ss_dprintf(1, "[SensorService] unstable\r\n");
		} else if (!gray_mode && (sw_lux >= als_config.Thr_Color_to_Gray)) {
			ss_dprintf(1, "[SensorService] RGB2IR:Mode(%d), Lux(%d) >= Thr_Color_to_Gray(%d)\n", gray_mode, sw_lux, als_config.Thr_Color_to_Gray);
			gray_mode = ALS_MODE_IR_Entry;
			if (!en_auto_iR) {
				sensor_external_set_gray_mode(gray_mode, IR_MAX_STRENGTH);
			} else {
				sensor_external_set_gray_mode(gray_mode, auto_ir_config.IR_LED_STEP[auto_ir_config.def_IRLED_idx]);
			}
		} else if (gray_mode && (sw_lux <= als_config.Thr_Gray_to_Color)) {
			als_get_statist(&als_data);
			if (als_ifSwitch(&als_config, &als_data)) {
				gray_mode = ALS_MODE_RGB;
				ss_dprintf(1, "[SensorService] IR2RGB:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(1)\n", gray_mode, sw_lux, als_config.Thr_Gray_to_Color);
				if (en_auto_iR) {
					auto_ir_config.def_IRLED_idx = irled_idx;
				}
				sensor_external_set_gray_mode(gray_mode, 0);
			} else {
				ss_dprintf(1, "[SensorService] IRStable:Mode(%d), Lux(%d) <= Thr_Gray_to_Color(%d), als_ifSwitch(0)\n", gray_mode, sw_lux, als_config.Thr_Gray_to_Color);
			}
		}
		/*auto-ir algorithm*/
		autoir_flow(auto_ir_config, gray_mode, &irled_idx);
#if(DEBUG_OSD)
		sensor_service_osd();
#endif
		vTaskDelay(SS_DELAY);
	}
}
#endif

void init_sensor_service(void)
{
	if (xTaskCreate(sensor_thread, ((const char *)"sensor_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\r\n sensor_thread: Create Task Error\n");
	}
}
