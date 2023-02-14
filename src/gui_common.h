/********************************************************************************

 ****     Copyright (C), 2022    ************************************************

 ********************************************************************************
 * File Name     : gui.h
 * Author        : ymc
 * Date          : 2022-08-11
 * Description   : gui.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2022-08-11
 *   Author      : ymc
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __GUI_COMMON_H__
#define __GUI_COMMON_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "gui_homepage.h"

#define HOME_PATH       ("/home/")
#define SYS_ICON_PATH   ("/home/.sys/icon/")
#define SYS_BG_PATH     ("/home/.sys/wallpaper/")
#define ROM_PATH        ("/home/rom/")
#define SYS_ICON_LOAD_FAILED   ("/home/.sys/icon/loadfailed.png")

typedef struct {
	lv_color_t		   *canvas_buf;
	lv_indev_drv_t	   indev_drv;
	lv_disp_drv_t	   disp_drv;
	lv_disp_draw_buf_t disp_buf;
	lv_disp_t		   *disp;
    GUI_HOMEPAGE_T     hp_data;
}GUI_DATA_T;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __GUI_COMMON_H__ */
