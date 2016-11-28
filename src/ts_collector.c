#include "ts_collector.h"
#include "adlist.h"
#include "zmalloc.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

static list * pegging_list = 0;

int create_counter(char * counter_name){

	if (pegging_list == 0) {
		return -1;
	}

	if (counter_get(counter_name) != 0) {
		return 0;
	}

	struct ts_data_node * node = (struct ts_data_node *)zmalloc(sizeof(struct ts_data_node));
	(void)memset(node->counter , 0x00 , sizeof(int) * TS_MONITOR_COLLECTOR_SECS);

	pthread_mutex_init(&node->counter_mutex , 0);

	(void)memset(node->counter_name , 0x00 , TS_CTR_NAME_LENGTH);
	(void)strcpy(node->counter_name , counter_name);

	pegging_list = listAddNodeTail(pegging_list , node);

	return 0;
}

int collect_counter(char * formatted_msg){

	listIter * itr = listGetIterator(pegging_list , 0);
	if (itr != 0) {
		listNode * node = 0;
		int offset = 0;
		while ((node = listNext(itr)) != 0) {
			struct ts_data_node * data = (struct ts_data_node *)(node->value);
			(void)pthread_mutex_lock(&(data->counter_mutex));
			
			int i = 0;
			for (;i < TS_MONITOR_COLLECTOR_SECS; i++) {
				offset += sprintf(formatted_msg + offset , "[%s_%d:%d]" , data->counter_name , TS_MONITOR_COLLECTOR_SECS - i - 1 , 
					 data->counter[i]);
			}

			(void)memset(data->counter , 0x00 , sizeof(int) * TS_MONITOR_COLLECTOR_SECS);

			(void)pthread_mutex_unlock(&(data->counter_mutex));
		}
	}

	return 0;
}

void * run_collect_loop(void * args) {
	UNUSED(args);

	while (1) {

		robj * channel = createStringObject(DEFAULT_PUB_AND_SUB_EVENT_NAME , strlen(DEFAULT_PUB_AND_SUB_EVENT_NAME));

		char msg[DEFAULT_INTERNAL_NOTIFICATION_STR_LENGTH] = {0};
		(void)collect_counter(msg);
			
	    robj * payload = createStringObject(msg , strlen(msg));
		
	    pubsubPublishMessage(channel,payload);

		decrRefCount(channel);
	    decrRefCount(payload);		
		
		(void)sleep(TS_MONITOR_COLLECTOR_SECS);
	}
}

int peg_counter(char * counter_name , int delta){

	struct ts_data_node * counter = counter_get(counter_name);
	if (counter == 0) {
		return -1;
	}

	int ret = -1;
	
	(void)pthread_mutex_lock(&(counter->counter_mutex));

	int current_timestamp = time(0);
	int index = current_timestamp % TS_MONITOR_COLLECTOR_SECS;
	counter->counter[index] += delta;

	ret = counter->counter[index];

	(void)pthread_mutex_unlock(&(counter->counter_mutex));
	
	return ret;
}

struct ts_data_node * counter_get(char * counter_name){
	listIter * itr = listGetIterator(pegging_list , 0);
	if (itr != 0) {
		listNode * node = 0;
		while ((node = listNext(itr)) != 0) {
			struct ts_data_node * data = (struct ts_data_node *)(node->value);
			if (strcmp(data->counter_name , counter_name) == 0) {
				return data;
			}
		}
	}

	return 0;
}


int init_ts_collector(){

	if (pegging_list == 0) {
		pegging_list = listCreate();
	}

	pthread_t tid;
	if (-1 == pthread_create(&tid , 0 , run_collect_loop , 0)){
	}
	
	return 0;
}

