#include <time.h>
#include <linux/input.h>

#include "lvgl.h"
#include "gui_common.h"


static void gui_calendar_event_key_handler(lv_event_t * e)
{
    uint32_t k = lv_event_get_key(e);

    if(k == KEY_B)
    {
        lv_obj_t *obj = lv_event_get_target(e);
        GUI_HOMEPAGE_T *hp_data = lv_event_get_user_data(e);

		lv_group_remove_obj(obj);
        lv_obj_del(obj);
		lv_indev_set_group(hp_data->key_indev, hp_data->bottom_grp);
    }
}

int gui_app_calendar(lv_obj_t *parent, void *data)
{
	time_t rawtime;
	struct tm info;
	GUI_HOMEPAGE_T *hp_data = (GUI_HOMEPAGE_T *)data;
    lv_obj_t *obj = lv_calendar_create(lv_scr_act());
//    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_add_event_cb(obj, gui_calendar_event_key_handler, LV_EVENT_KEY, data);
	lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
	lv_calendar_header_dropdown_create(obj);

	time(&rawtime);
	gmtime_r(&rawtime, &info);
	lv_calendar_set_today_date(obj, info.tm_year+1900, info.tm_mon+1, info.tm_mday);
	lv_calendar_set_showed_date(obj, info.tm_year+1900, info.tm_mon+1);

	lv_group_add_obj(hp_data->app_grp, obj);
	lv_indev_set_group(hp_data->key_indev, hp_data->app_grp);
//    lv_group_focus_obj(obj);
//	lv_group_set_editing(hp_data->app_grp,false); //编辑模式
}

