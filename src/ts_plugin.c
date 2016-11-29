#include "ts_plugin.h"
#include "sync_data_def.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

static void * ts_plugin_handle[DEFAULT_MAX_PLUGIN_NUM] = {0};
static int plugin_num = -1;

static COLLECTOR_NEXT ts_callback = 0;

int load_plugin(char * path) {

	DIR * pdir = opendir(path);
	if (pdir == 0) {
		return -1;
	}

	struct dirent * ent = 0;
	while (0 != (ent = readdir(pdir))) {
		if (plugin_num >= (DEFAULT_MAX_PLUGIN_NUM - 1)) {
			break;
		}

		
		char dirname[DEFAULT_DER_LENGTH] = {0};
		(void)sprintf(dirname , "%s/%s" , path , ent->d_name);
		if (ent->d_type == 8 && strstr(dirname , "lib") != 0 && strstr(dirname , ".so") != 0) {
			plugin_num ++;
			ts_plugin_handle[plugin_num] = dlopen(dirname , RTLD_LAZY);
			if (ts_plugin_handle[plugin_num] == 0) {
				plugin_num --;
				return -1;
			}

			serverLog(LL_DEBUG , "loading plugin %s index:%d" , dirname , plugin_num);
		} else if (ent->d_type == 4 && strstr(dirname , ".") == 0 && strstr(dirname , "..") == 0) {
			load_plugin(dirname);
		} else {
			continue;
		}
	}

	(void)closedir(pdir);

	return 0;
}

int init_ts_plugin(){
	char * path = getenv("TS_PLUGIN_PATH");
	if (path == 0) {
		return -1;
	}

	return load_plugin(path);
}

int ts_plugin_call_function(char * counter_name , int second , 
	int counter_value , int eof , int bof){

	int i = 0 ;
	for (;i < plugin_num + 1;i ++) {
		ts_callback = (COLLECTOR_NEXT)dlsym(ts_plugin_handle[i] , "ts_plugin_next");
		if (ts_callback == 0) {
			return -1;
		}

		if (-1 == (*ts_callback)(counter_name , second , counter_value , eof , bof)){
			return -1;
		}
	}

	return 0;
}

int destroy_ts_plugin(){
	int i = 0 ;
	for (;i < plugin_num;i ++) {
		if (dlclose(ts_plugin_handle[i]) == -1) {
			return -1;
		}
		ts_plugin_handle[i] = 0;
	}
	
	return 0;
}


