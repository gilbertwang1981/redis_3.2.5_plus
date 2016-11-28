#include "ts_plugin.h"
#include "sync_data_def.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

static char ts_plugin_path[TS_PLUGIN_PATH_LENGTH] = {0};
static void * ts_plugin_handle = 0;

static COLLECTOR_NEXT ts_callback = 0;

int init_ts_plugin(){
	char * path = getenv("TS_PLUGIN_PATH");
	if (path == 0) {
		return -1;
	}

	(void)sprintf(ts_plugin_path , "%s" , path);

	ts_plugin_handle = dlopen(ts_plugin_path , RTLD_LAZY);
	if (ts_plugin_handle == 0) {
		return -1;
	}

	return 0;
}

int ts_plugin_call_function(char * counter_name , int second , 
	int counter_value , int eof , int bof){
	if (ts_plugin_handle == 0) {
		return -1;
	}

	if (ts_callback == 0) {
		ts_callback = dlsym(ts_plugin_handle , "ts_plugin_next");
		if (ts_callback == 0) {
			return -1;
		}
	}

	return (*ts_callback)(counter_name , second , counter_value , eof , bof);
}

int destroy_ts_plugin(){
	if (ts_plugin_handle == 0) {
		return -1;
	}

	if (dlclose(ts_plugin_handle) == -1) {
		return -1;
	}
	
	return 0;
}
