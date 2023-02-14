#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/input.h>

#include "lvgl.h"
#include "nes_main.h"
#include "gui_common.h"
#include "gui_calendar.h"


#define TOP_WIDGETS_COL_SPACE      (30)
#define MIDDLE_CONT_COL_SPACE      (320)
#define BOTTOM_CONT_COL_SPACE      (90)

#define MIDDLE_CONT_VER_RES        (20 + TOP_WIDGETS_COL_SPACE)

#define ICON_SIZE           (64)
#define ICON_ROW_COUNT      (4)
#define ICON_COLUNM_COUNT   (6)
#define ICON_PAD_TOP        (20)
#define ICON_PAD_BOTTOM     (40)
#define ICON_PAD_LEFT       (115)
#define ICON_PAD_RIGHT      (115)

#define ICON_ROW_SPACE      (20)
#define ICON_COL_SPACE      (40)//((ICON_HOR_RES - (ICON_SIZE * ICON_COLUNM_COUNT)) / (ICON_COLUNM_COUNT - 1))
#define ICON_HOR_RES        (4 + (ICON_SIZE * ICON_COLUNM_COUNT) + (ICON_COL_SPACE * (ICON_COLUNM_COUNT - 1)))//((LV_HOR_RES - ICON_PAD_LEFT - ICON_PAD_RIGHT))        // 列间距
#define ICON_VER_RES        (20)

#define BOTTOM_CONT_PAD_TOP     (2)
#define BOTTOM_CONT_PAD_COL     (20)

typedef int (*BOTTOM_MENU_START_FUNC) (lv_obj_t *, void *);

struct bottom_menu_field {
    const char *icon_file;
    BOTTOM_MENU_START_FUNC start_func;
};

static struct bottom_menu_field bottom_menu_entry[] = {
    {
        .icon_file = "calendar.png",
        .start_func = gui_app_calendar,
    },
    {
        .icon_file = "picture_explore.png",
        .start_func = NULL,
    },
    {
        .icon_file = "music_player.png",
        .start_func = NULL,
    },
    {
        .icon_file = "video_play.png",
        .start_func = NULL,
    },
    {
        .icon_file = "setting.png",
        .start_func = NULL,
    },
};

// 去掉最后的后缀名
void strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }
}


static void gui_img_set_zoom(lv_obj_t * obj_img, uint32_t obj_width, uint32_t obj_height)
{
    lv_img_t * img = (lv_img_t *)obj_img;
    if (obj_img == NULL)
    {
        printf("[%s:%d] param errror\n", __FUNCTION__, __LINE__);
        return;
    }
 
    if (obj_width == 0 || obj_height == 0)
    {
        printf("[%s:%d] param errror\n", __FUNCTION__, __LINE__);
        return;
    }
 
    uint32_t img_width = 0, img_height = 0, zoom_factor = 0;
    img_width = img->w;
    img_height = img->h;
 
    if (img_width != 0 && img_height != 0)
    {
        uint32_t y_a= obj_height * img_width;   
        uint32_t x_b= obj_width * img_height;
 
        if (x_b >= y_a)
        {
            if (img_height >= obj_height)
            {
                uint32_t x = obj_height << 8;
                zoom_factor = x / img_height;
                lv_img_set_zoom(obj_img, zoom_factor);
            }
        }
        else
        {
            if (img_width > obj_width)
            {
                uint32_t x = obj_width << 8;
                zoom_factor = x / img_width;
                lv_img_set_zoom(obj_img, zoom_factor);
            }
        }
    }
}

static void lv_timer_update_time(lv_timer_t * timer)
{
    lv_obj_t * label = timer->user_data;

    // 获取系统时间
    char buf[32];
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);

    lv_label_set_text_fmt(label, "   %02d:%02d  %02d-%02d-%02d", info->tm_hour, info->tm_min, (info->tm_year + 2000 - 100), (info->tm_mon + 1), info->tm_mday);
}

char* thread_run_game(void* arg)
{
	GUI_HOMEPAGE_T *hp_data = (GUI_HOMEPAGE_T *)arg;

	usleep(100*1000);
	nes_load(hp_data->rom_path);

	lv_scr_load(hp_data->def_scr);

	return NULL;
}

static void event_game_clicked_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
	GUI_HOMEPAGE_T *hp_data = lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED)
	{
        char * file_name = lv_label_get_text(lv_obj_get_child(obj, 0));
		sprintf(hp_data->rom_path, "%s%s", ROM_PATH, file_name);
        printf("rom_name: %s\n", hp_data->rom_path);
	
//		lv_indev_enable(lv_event_get_indev(e), false);

#if 0
		printf("creat thread\r\n");
		lv_scr_load(hp_data->game_scr);
		pthread_t tid;
		if (pthread_create(&tid, (void*)hp_data, (void*)thread_run_game, "GameEmulator") != 0)
		{
	        printf("pthread_create error.");
	        exit(EXIT_FAILURE);
	    }
#else
		nes_fb_clear(0x0);
		nes_load(hp_data->rom_path);
		lv_obj_invalidate(hp_data->def_scr);
#endif
    }
}


static void event_bottom_menu_clicked_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED)
    {
        lv_obj_t *obj = lv_event_get_target(e);
        GUI_HOMEPAGE_T *hp_data = lv_event_get_user_data(e);

        char * file_name = lv_label_get_text(lv_obj_get_child(obj, 0));
        printf("file_name: %s\n", file_name);
        for (int i = 0; i < sizeof(bottom_menu_entry)/sizeof(bottom_menu_entry[0]); ++i)
        {
            if((strcmp(bottom_menu_entry[i].icon_file, file_name) == 0) && (bottom_menu_entry[i].start_func != NULL))
            {
                bottom_menu_entry[i].start_func(obj, hp_data);
            }
        }
    }
}

static void event_key_handler(lv_event_t * e)
{
	uint32_t k = lv_event_get_key(e);
	lv_group_t *g = lv_obj_get_group(lv_event_get_target(e));
	GUI_HOMEPAGE_T *hp_data = lv_event_get_user_data(e);

	printf("key=%d\r\n", k);
	if(k == LV_KEY_RIGHT)
	{
		lv_group_focus_next(g);
	}
	else if(k == LV_KEY_LEFT)
	{
		lv_group_focus_prev(g);
	}
	else if(k == LV_KEY_UP)
	{
		printf("g=%x,bottom_grp=%x,%x,%x\r\n", g, hp_data->bottom_grp,hp_data->game_grp->obj_focus,*hp_data->game_grp->obj_focus);
		if(g == hp_data->bottom_grp)
		{
			lv_indev_set_group(hp_data->key_indev, hp_data->game_grp);
		}
	}
	else if(k == LV_KEY_DOWN)
	{
		printf("g=%x,game_grp=%x,%x,%x\r\n", g, hp_data->game_grp,hp_data->bottom_grp->obj_focus,*hp_data->bottom_grp->obj_focus);
		if(g == hp_data->game_grp)
		{
			lv_indev_set_group(hp_data->key_indev, hp_data->bottom_grp);
		}
	}
}

static void gui_top_widgets(lv_obj_t * parent)
{
    static lv_style_t obj_layout_style;   // 容器的样式

    lv_style_init(&obj_layout_style);
    lv_style_set_pad_all(&obj_layout_style, 0);
    lv_style_set_bg_opa(&obj_layout_style, LV_OPA_0);
    lv_style_set_text_font(&obj_layout_style, &lv_font_montserrat_16);
    lv_style_set_border_opa(&obj_layout_style, LV_OPA_0);
    lv_style_set_radius(&obj_layout_style, 0);
    lv_style_set_text_color(&obj_layout_style, lv_color_white());

    /* Layout Init */
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel,  LV_PCT(100), TOP_WIDGETS_COL_SPACE);
    lv_obj_add_style(panel, &obj_layout_style, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 5);

    /* 右上角小图标 */
    lv_obj_t * panel_icon = lv_obj_create(panel);
    lv_obj_set_size(panel_icon,  200, 25);
    lv_obj_set_layout(panel_icon, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(panel_icon, LV_BASE_DIR_RTL, 0);
    lv_obj_set_flex_flow(panel_icon, LV_FLEX_FLOW_ROW);
    lv_obj_align(panel_icon, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(panel_icon, &obj_layout_style, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_t * label = lv_label_create(panel_icon);
    lv_label_set_text(label,  " ");

    lv_obj_t * label_bat = lv_label_create(panel_icon);
    lv_label_set_text(label_bat,  LV_SYMBOL_BATTERY_EMPTY);

    lv_obj_t * label_batchar = lv_label_create(label_bat);
    lv_obj_set_style_text_font(label_batchar, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_batchar,  LV_SYMBOL_CHARGE);
    lv_obj_center(label_batchar);


    lv_obj_t * label_wifi = lv_label_create(panel_icon);
    lv_label_set_text(label_wifi, LV_SYMBOL_WIFI);

    lv_obj_t * label_time = lv_label_create(panel);
    lv_label_set_text(label_time, "  ");
    lv_obj_align(label_time, LV_ALIGN_LEFT_MID, 0, 0);

    lv_timer_t * timer = lv_timer_create(lv_timer_update_time, 1000,  label_time);
}


int gui_homepage_init(GUI_HOMEPAGE_T *hp_data)
{
	int ret = 0;
	lv_obj_t * img_icon;
	lv_obj_t *btn_icon;
	lv_obj_t *label;
	char file_name[64] = {0};
	struct dirent *de;
	DIR *dr;
	uint32_t i = 0;

	hp_data->game_grp = lv_group_create();
    hp_data->bottom_grp = lv_group_create();
    hp_data->app_grp = lv_group_create();
	lv_indev_set_group(hp_data->key_indev, hp_data->game_grp);
//    lv_indev_set_group(key_indev, hp_data->bottom_grp);

	hp_data->def_scr = lv_scr_act();
	lv_obj_set_scrollbar_mode(hp_data->def_scr, LV_SCROLLBAR_MODE_OFF);
//	hp_data->game_scr = lv_obj_create(NULL);

	/* 设置容器的样式 */
	lv_style_init(&hp_data->cont_style);
    lv_style_set_bg_opa(&hp_data->cont_style, LV_OPA_0);
    lv_style_set_border_opa(&hp_data->cont_style, LV_OPA_0);
    lv_style_set_pad_column(&hp_data->cont_style, ICON_COL_SPACE);
    lv_style_set_pad_row(&hp_data->cont_style, ICON_ROW_SPACE);
	lv_style_set_pad_top(&hp_data->cont_style, ICON_PAD_TOP);
//    lv_style_set_pad_all(&hp_data->cont_style, 0);


	/* 容器中的图标的样式 */
	lv_style_init(&hp_data->icon_style);
    lv_style_set_bg_opa(&hp_data->icon_style, LV_OPA_100);
	lv_style_set_bg_color(&hp_data->icon_style, lv_palette_lighten(LV_PALETTE_GREY,1));
    lv_style_set_text_opa(&hp_data->icon_style, LV_OPA_0);
    lv_style_set_text_font(&hp_data->icon_style,  &lv_font_montserrat_8);
    lv_style_set_text_color(&hp_data->icon_style, lv_color_white()); //设置字体颜色
//    lv_style_set_border_opa(&hp_data->icon_style, LV_OPA_0);
//	lv_style_set_border_color(&hp_data->icon_style, lv_color_white()); // 设置边框颜色
//    lv_style_set_border_opa(&hp_data->icon_style, LV_OPA_50); // 设置边框透明度
//    lv_style_set_border_width(&hp_data->icon_style, 3); // 设置边框宽度
	lv_style_set_radius(&hp_data->icon_style, 5);

	/* 底部面板样式 */
    lv_style_init(&hp_data->bottom_panel_style);
    lv_style_set_pad_all(&hp_data->bottom_panel_style, 0);
    lv_style_set_bg_opa(&hp_data->bottom_panel_style, LV_OPA_50);
	lv_style_set_pad_column(&hp_data->bottom_panel_style, BOTTOM_CONT_PAD_COL);
	lv_style_set_pad_top(&hp_data->bottom_panel_style, BOTTOM_CONT_PAD_TOP);
    lv_style_set_border_opa(&hp_data->bottom_panel_style, LV_OPA_0);
    lv_style_set_radius(&hp_data->bottom_panel_style, 22);


    /* 屏幕顶部状态栏区域 */
    gui_top_widgets(hp_data->def_scr);

	/* 中间图标区域 */
    lv_obj_t * icon_cont = lv_obj_create(hp_data->def_scr);
    lv_obj_set_size(icon_cont, LV_PCT(100), MIDDLE_CONT_COL_SPACE);
    lv_obj_set_layout(icon_cont, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(icon_cont, LV_BASE_DIR_LTR, 0);
    lv_obj_set_flex_flow(icon_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_y(icon_cont, MIDDLE_CONT_VER_RES);
    lv_obj_add_style(icon_cont, &hp_data->cont_style, LV_PART_MAIN|LV_STATE_DEFAULT);

    /* 底部面板区域 */
    /* Layout Init */
    lv_obj_t * bottom_panel = lv_obj_create(hp_data->def_scr);
    lv_obj_set_size(bottom_panel,  LV_PCT(70), BOTTOM_CONT_COL_SPACE);
    lv_obj_add_style(bottom_panel, &hp_data->bottom_panel_style, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(bottom_panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_layout(bottom_panel, LV_LAYOUT_FLEX);
    //lv_obj_set_style_base_dir(bottom_panel, LV_BASE_DIR_RTL, 0);
    lv_obj_set_flex_flow(bottom_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_align(bottom_panel, LV_ALIGN_BOTTOM_MID, 0, -15);

	lv_obj_t * img_bg;
	img_bg = lv_img_create(hp_data->def_scr);
	lv_img_set_src(img_bg, "/home/.sys/wallpaper/bg1.png");
	lv_obj_move_background(img_bg);  // 将背景移动到后台

	dr = opendir(ROM_PATH);
	if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory!\n");
        return 0;
    }

	while ((de = readdir(dr)) != NULL)
	{
		if((strcmp(de->d_name,".") == 0)  ||\
           (strcmp(de->d_name,"..") == 0) ||\
           ((strcmp((de->d_name + (strlen(de->d_name) - 4)) , ".nes") != 0)&&\
           (strcmp((de->d_name + (strlen(de->d_name) - 4)) , ".NES") != 0)))
		{
			continue;
		}

		btn_icon = lv_btn_create(icon_cont);
		lv_obj_set_size(btn_icon, 256, 240);
		lv_obj_set_style_outline_width(btn_icon, 4, LV_STATE_FOCUS_KEY);
	    lv_obj_add_event_cb(btn_icon, event_game_clicked_handler, LV_EVENT_CLICKED, hp_data);
	    lv_obj_add_event_cb(btn_icon, event_key_handler, LV_EVENT_KEY, hp_data);
	    lv_obj_add_style(btn_icon, &hp_data->icon_style, 0);
		lv_obj_add_flag(btn_icon, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
		lv_group_add_obj(hp_data->game_grp, btn_icon);
		
		label = lv_label_create(btn_icon); //创建名称
		lv_label_set_text(label, de->d_name);
		lv_obj_align_to(label, btn_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//		lv_obj_move_foreground(label);
		
		strip_ext(de->d_name);
		sprintf(file_name, "%s%s.png", ROM_PATH, de->d_name);
		if(access(file_name, F_OK) == -1)
			sprintf(file_name, "%s", SYS_ICON_LOAD_FAILED);
		img_icon = lv_img_create(btn_icon);
		lv_img_set_src(img_icon, file_name);
		gui_img_set_zoom(img_icon, 256, 240);
		lv_obj_center(img_icon);

	}
	closedir(dr);		

	lv_group_set_editing(hp_data->game_grp,false);//导航模式
	lv_group_set_wrap(hp_data->game_grp, true);

	for (i = 0; i < sizeof(bottom_menu_entry)/sizeof(bottom_menu_entry[0]); ++i)
	{
		#if 1
        btn_icon = lv_btn_create(bottom_panel);
		
		lv_obj_remove_style(btn_icon, NULL, 0);
        lv_obj_set_size(btn_icon, 80, 80);
        lv_obj_add_event_cb(btn_icon, event_bottom_menu_clicked_handler, LV_EVENT_CLICKED, hp_data);
        lv_obj_add_event_cb(btn_icon, event_key_handler, LV_EVENT_KEY, hp_data);
		lv_obj_set_style_bg_opa(btn_icon, LV_OPA_0, 0);
		lv_obj_set_style_border_opa(btn_icon, LV_OPA_0, 0);
		lv_obj_set_style_outline_width(btn_icon, 2, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_radius(btn_icon, 20, 0);
		lv_obj_set_style_pad_top(btn_icon, 0, 0);
        lv_obj_add_flag(btn_icon, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

        label = lv_label_create(btn_icon); //创建名称
        lv_obj_set_style_text_opa(label, LV_OPA_0, 0);
        lv_label_set_text(label, bottom_menu_entry[i].icon_file);
		
		img_icon = lv_img_create(btn_icon);
		sprintf(file_name, "%s%s", SYS_ICON_PATH, bottom_menu_entry[i].icon_file);
		lv_img_set_src(img_icon, file_name);
		gui_img_set_zoom(img_icon, 80, 80);
		lv_obj_center(img_icon);
		
		lv_group_add_obj(hp_data->bottom_grp, btn_icon);
		#else
		img_icon = lv_img_create(bottom_panel);
		sprintf(file_name, "%s%s", SYS_ICON_PATH, bottom_menu_entry[i].icon_file);
		lv_img_set_src(img_icon, file_name);
		gui_img_set_zoom(img_icon, 80, 80);
		lv_obj_center(img_icon);
		
		lv_obj_set_style_outline_opa(btn_icon, LV_OPA_100, LV_STATE_FOCUS_KEY);
		lv_obj_set_style_outline_width(btn_icon, 2, LV_STATE_FOCUS_KEY);
        lv_obj_add_flag(img_icon, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_add_event_cb(img_icon, event_bottom_menu_clicked_handler, LV_EVENT_CLICKED, hp_data);
        lv_obj_add_event_cb(img_icon, event_key_handler, LV_EVENT_KEY, hp_data);
		lv_group_add_obj(hp_data->bottom_grp, img_icon);

		#endif
	}
	lv_group_set_editing(hp_data->bottom_grp,false);//导航模式
	lv_group_set_wrap(hp_data->bottom_grp, true);
	
	return ret;
}

