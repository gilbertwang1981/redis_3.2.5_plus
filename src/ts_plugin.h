#ifndef __TS_PLUGIN_H__
#define __TS_PLUGIN_H__

int init_ts_plugin();
int destroy_ts_plugin();
int ts_plugin_call_function(char * counter_name , int second , 
	int counter_value , int eof , int bof);

#endif
