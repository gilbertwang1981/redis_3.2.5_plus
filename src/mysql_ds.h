#ifndef __MYSQL_REDIS_DS__
#define __MYSQL_REDIS_DS__

#include "sync_data_def.h"

#define MYSQL_REDIS_WHITE_LIST_EACH_PAGE_SIZE 5

struct white_list_data {
	char key[SET_CMD_KEY_LENGTH];
};

int mysql_ds_init();
int mysql_ds_destory();

int insert_log(struct set_command_sync_data * data);

int load_white_list_from_db();

int is_in_white_list(char * key);

int health_check();

#endif

