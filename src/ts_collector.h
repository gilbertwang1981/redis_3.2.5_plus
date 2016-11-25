#ifndef __TS_COLLECTOR_H__
#define __TS_COLLECTOR_H__

#include "server.h"

#define TS_MONITOR_COLLECTOR_SECS 5
#define TS_CTR_NAME_LENGTH 128


#define DEFAULT_PUB_AND_SUB_EVENT_NAME "REDIS_INTERNAL_CUSTOMIZED_EVENT_PEGGING"
#define DEFAULT_INTERNAL_NOTIFICATION_STR_LENGTH 1024


#include <pthread.h>

struct ts_data_node {
	char counter_name[TS_CTR_NAME_LENGTH];
	int counter[TS_MONITOR_COLLECTOR_SECS];
	pthread_mutex_t counter_mutex;
};

int init_ts_collector();

int create_counter(char * counter_name);

int collect_counter(char * formatted_msg);

int peg_counter(char * counter_name , int delta);

struct ts_data_node * counter_get(char * counter_name);

#endif


