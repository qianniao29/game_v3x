#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl.h"
#include "fbdev.h"
#include "evdev.h"

#include "gui_common.h"

 GUI_DATA_T         gui_data;


int gui_init()
{
	uint32_t h = 0, w = 0, dpi = 0;
	int ret = 0;

	lv_init();  /* lvgl 初始化 */

    /**
     * fb 初始化 此函数在 lv_drivers/display/fbdev.c 中
     * 就是打开 fb 设备映射显存出来使用
     */
    fbdev_init();
//	fbdev_set_offset(0, 2);

    lv_disp_drv_init(&gui_data.disp_drv);
	fbdev_get_sizes(&w, &h, &dpi);
    gui_data.disp_drv.hor_res = h;
    gui_data.disp_drv.ver_res = w;
	
	printf("%s: w=%d,h=%d\r\n",__func__, w, h);

	gui_data.canvas_buf = calloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(gui_data.disp_drv.hor_res,
								gui_data.disp_drv.ver_res), 1);
	if (!gui_data.canvas_buf)
        return -1;
    // Allocate LVGL buffers
    void * lvgl_buf1 = malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(gui_data.disp_drv.hor_res, gui_data.disp_drv.ver_res));
    if (!lvgl_buf1)
        return -1;
    void *lvgl_buf2 = malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(gui_data.disp_drv.hor_res, gui_data.disp_drv.ver_res));
    if (!lvgl_buf2)
        return -1;
    lv_disp_draw_buf_init(&gui_data.disp_buf, lvgl_buf1, lvgl_buf2,
    						gui_data.disp_drv.hor_res * gui_data.disp_drv.ver_res);

    gui_data.disp_drv.draw_buf = &gui_data.disp_buf;
    gui_data.disp_drv.flush_cb = fbdev_flush;    /* drm_flush这就是输入显示驱动提供的操作函数 */
    gui_data.disp = lv_disp_drv_register(&gui_data.disp_drv);

     /* Linux input device init */
    evdev_init();
    /* Initialize and register a display input driver */
    lv_indev_drv_init(&gui_data.indev_drv);      /*Basic initialization*/
    gui_data.indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    gui_data.indev_drv.read_cb = evdev_read;
    lv_indev_t *key_indev = lv_indev_drv_register(&gui_data.indev_drv);

	gui_homepage_init(&gui_data.hp_data, key_indev);

	return ret;
}


/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

