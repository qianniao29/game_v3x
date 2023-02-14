/********************************************************************************

 ****     Copyright (C), 2022    ************************************************

 ********************************************************************************
 * File Name     : gui_homepage.h
 * Author        : ymc
 * Date          : 2022-08-09
 * Description   : gui_homepage.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2022-08-09
 *   Author      : ymc
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __GUI_HOMEPAGE_H__
#define __GUI_HOMEPAGE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define ROM_NAME_MAX    128

typedef struct {
	lv_indev_t *key_indev;
    lv_group_t *game_grp;
    lv_group_t *bottom_grp;
    lv_group_t *app_grp;
    lv_style_t cont_style;              // 中间图标区域，容器的样式
    lv_style_t icon_style;              // 中间图标区域，容器中的图标的样式
    lv_style_t bottom_panel_style;  // 底部容器的样式
    lv_obj_t * game_scr;
    lv_obj_t * def_scr;
    char       rom_path[ROM_NAME_MAX];
} GUI_HOMEPAGE_T;

extern int gui_homepage_init(GUI_HOMEPAGE_T *hp_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __GUI_HOMEPAGE_H__ */
