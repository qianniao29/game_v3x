#include <stdio.h>
#include <dirent.h>
#include "lvgl.h"
#include "gui_common.h"

int gui_file_explore_entry(lv_obj_t *parent)
{
	struct dirent *de;
	DIR *dr;
	uint32_t index = 0;

	lv_obj_t *cont = lv_obj_create(parent);
	lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(cont, 1);

    lv_obj_t *file_list = lv_list_create(cont);
    lv_obj_set_size(file_list, LV_PCT(100), LV_PCT(84));
    lv_table_set_col_width(file_list, 0, LV_PCT(100));
    lv_table_set_col_cnt(file_list, 1);
	lv_obj_set_scroll_dir(file_list, LV_DIR_TOP | LV_DIR_BOTTOM); // only scroll up and down

	dr = opendir(HOME_PATH);
	if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory!\n");
        return -1;
    }

	while ((de = readdir(dr)) != NULL)
	{
		lv_table_set_cell_value_fmt(file_list, index, 0, "  %s", de->d_name);
		index++;
	}

	closedir(dr);

	return 0;
}