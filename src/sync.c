#include "sync.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "zmalloc.h"

#include "mysql_ds.h"

static int MSG_QUEUE_KEY = -1;
static int isRunning4Rep = 0;
static int MSG_QUEUE_ID = 0x1000ffff;

int sync_data_to_queue(char * key , void * value , int vlength , int timeunit , 
	int expire , char * ip , int type){

	struct set_command_sync_data * data = (struct set_command_sync_data *)zmalloc(sizeof(struct set_command_sync_data));
	data->expire = expire;
	data->cmd = type;

	(void)memset(data->key , 0x00 , SET_CMD_KEY_LENGTH);
	(void)memset(data->value , 0x00 , SET_CMD_VALUE_LENGTH);

	(void)strcpy(data->ipaddress , ip);

	int len = SET_CMD_KEY_LENGTH;
	if (len > (int)strlen(key)) {
		len = (int)strlen(key);
	}
	(void)memcpy(data->key , key , len);

	data->timeunit = timeunit;

	len = SET_CMD_VALUE_LENGTH;
	if (len > vlength) {
		len = vlength;
	}
	if (len > 0) {
		(void)memcpy(data->value , value , len);
	}
	
	if (-1 == sync_to_queue(data)){
		(void)zfree(data);
		
		return -1;
	}

	(void)zfree(data);
	
	return 0;
}

int sync_to_queue(struct set_command_sync_data * data){
	if (MSG_QUEUE_KEY == -1) {
		MSG_QUEUE_KEY = create_queue();
		if (MSG_QUEUE_KEY == -1) {
			return -1;
		}
	}

	struct msg_buffer * data_buf = (struct msg_buffer *)zmalloc(sizeof(struct msg_buffer));
	
	data_buf->mtype = REDIS_PLUGIN_SYNC_SET_TYPE;
	
	(void)memcpy(data_buf->mtext , (char *)data , sizeof(struct set_command_sync_data));

	while (1) {
		int ret = msgsnd(MSG_QUEUE_KEY , data_buf , sizeof(struct set_command_sync_data) , IPC_NOWAIT);
		if (ret == -1 && errno != EAGAIN) {
			serverLog(LL_WARNING , "send to queue failed , %s %d" , strerror(errno) , errno);

			(void)zfree(data_buf);
			
			return -1;
		} else if (ret == 0) {
			break;
		}
	}

	(void)zfree(data_buf);
	
	return 0;
}

int create_queue(){
	int key = msgget((key_t)MSG_QUEUE_ID , IPC_CREAT | 0660);
	if (key == -1) {
		serverLog(LL_WARNING , "create message queue failed , %s" , strerror(errno));
		
		return -1;
	}

	MSG_QUEUE_KEY = key;

	return 0;
}

int delete_queue(){
	if (MSG_QUEUE_KEY == -1) {
		serverLog(LL_WARNING , "the queue is not created. %d" , MSG_QUEUE_KEY);
		
		return -1;
	}
	
	int ret = msgctl(MSG_QUEUE_KEY , IPC_RMID , 0);
	if (ret == -1) {
		serverLog(LL_WARNING , "remove message queue failed , %s" , strerror(errno));
		
		return -1;
	}

	MSG_QUEUE_KEY = -1;

	return 0;
}

void * run_loop(void * args) {
	UNUSED(args);
	
	while (isRunning4Rep) {
		struct msg_buffer * msgbuffer = (struct msg_buffer *)zmalloc(sizeof(struct msg_buffer));
		int ret = msgrcv(MSG_QUEUE_KEY , msgbuffer , 
			sizeof(struct set_command_sync_data) , REDIS_PLUGIN_SYNC_SET_TYPE , IPC_NOWAIT);
		if (ret == -1 && (errno == EAGAIN || errno == ENOMSG)) {
			(void)usleep(DEFAULT_EXIT_INTERVAL * 4);
		} else if (ret == -1){
			serverLog(LL_WARNING , "get message from queue failed , %s:%d" , strerror(errno) , errno);

			isRunning4Rep = 0;
			delete_queue();
			(void)zfree(msgbuffer);
		
			return 0;
		} else {
			struct set_command_sync_data * data = (struct set_command_sync_data *)(msgbuffer->mtext);
			if (-1 == insert_log(data)){
				serverLog(LL_WARNING , "insert log failed.");
			}
		}
		(void)zfree(msgbuffer);
	}
	
	return 0;
}

int sync_init(){
	if (create_queue() != 0){
		return -1;
	}
		
	pthread_t tid;
	
	isRunning4Rep = 1;
	int ret = pthread_create(&tid , 0 , run_loop , 0);
	if (ret == -1) {
		serverLog(LL_WARNING , "create working thread failed , %s" , strerror(errno));

		delete_queue();

		isRunning4Rep = 0;

		return -1;
	}

	return 0;
}

int sync_destory(){
	isRunning4Rep = 0;

	(void)sleep(DEFAULT_EXIT_INTERVAL);
	
	delete_queue();
	
	return 0;
}



