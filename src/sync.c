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

static int isRunning4Rep[REDIS_CMD_NUM] = {0 , 0 , 0 , 0 , 0 , 0};

static int thread_index[REDIS_CMD_NUM] = {0 , 1 , 2 , 3 , 4 , 5};

static int MSG_QUEUE_KEY[REDIS_CMD_NUM] = {-1 , -1 , -1 , -1 , -1 , -1};
static int MSG_QUEUE_IDS[REDIS_CMD_NUM] = {0x1000FFFF , 0x2000FFFF , 0x3000FFFF , 0x4000FFFF , 0x5000FFFF , 0x6000FFFF};

int sync_data_to_queue(char * key , void * value , int vlength , int timeunit , 
	int expire , char * ip , int type){

	struct set_command_sync_data * data = (struct set_command_sync_data *)zmalloc(sizeof(struct set_command_sync_data));
	data->expire = expire;
	data->cmd = type;

	(void)memset(data->key , 0x00 , SET_CMD_KEY_LENGTH + 1);
	(void)memset(data->value , 0x00 , SET_CMD_VALUE_LENGTH + 1);
	(void)memset(data->ipaddress , 0x00 , IP_ADDRESS_LENGTH + 1);

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
	if (MSG_QUEUE_KEY[data->cmd] == -1) {
		MSG_QUEUE_KEY[data->cmd] = create_queue(MSG_QUEUE_IDS[data->cmd]);
		if (MSG_QUEUE_KEY[data->cmd] == -1) {
			return -1;
		}
	}

	struct msg_buffer * data_buf = (struct msg_buffer *)zmalloc(sizeof(struct msg_buffer));
	
	data_buf->mtype = REDIS_PLUGIN_SYNC_SET_TYPE;
	
	(void)memcpy(data_buf->mtext , (char *)data , sizeof(struct set_command_sync_data));

	int counter = 0;
	while (1) {
		int ret = msgsnd(MSG_QUEUE_KEY[data->cmd] , data_buf , sizeof(struct set_command_sync_data) , IPC_NOWAIT);
		if (ret == -1 && errno != EAGAIN) {
			serverLog(LL_WARNING , "send to queue failed , %s %d" , strerror(errno) , errno);

			(void)zfree(data_buf);
			
			return -1;
		} else if (ret == 0) {
			break;
		}

		if (++ counter % DEFAULT_SND_MSG_RETRY_TIMES == 0) { 
			serverLog(LL_WARNING , "retry to send into queue failed , %s" , strerror(errno));

			break;
		}
	}

	(void)zfree(data_buf);
	
	return 0;
}

int create_queue(int queue_id){
	int key = msgget((key_t)queue_id , IPC_CREAT | 0660);
	if (key == -1) {
		serverLog(LL_WARNING , "create message queue failed , %s" , strerror(errno));
		
		return -1;
	}

	return key;
}

int delete_queue(int key){
	if (key == -1) {
		serverLog(LL_WARNING , "the queue is not created. %d" , key);
		
		return -1;
	}
	
	int ret = msgctl(key , IPC_RMID , 0);
	if (ret == -1) {
		serverLog(LL_WARNING , "remove message queue failed , %s" , strerror(errno));
		
		return -1;
	}

	return 0;
}

void * run_loop(void * args) {
	int * pIndex = (int *)args;

	int counter = 0;
	
	while (isRunning4Rep[*pIndex]) {
		struct msg_buffer * msgbuffer = (struct msg_buffer *)zmalloc(sizeof(struct msg_buffer));
		int ret = msgrcv(MSG_QUEUE_KEY[*pIndex] , msgbuffer , 
			sizeof(struct set_command_sync_data) , REDIS_PLUGIN_SYNC_SET_TYPE , IPC_NOWAIT);
		if (ret == -1 && (errno == EAGAIN || errno == ENOMSG)) {
			(void)usleep(DEFAULT_EXIT_INTERVAL * 10);
			if (++ counter % DB_HC_FREQUENCE == 0) {
				health_check(*pIndex);
				counter = 0;
			}
		} else if (ret == -1){
			serverLog(LL_WARNING , "get message from queue failed , %s:%d" , strerror(errno) , errno);

			isRunning4Rep[*pIndex] = 0;
			delete_queue(MSG_QUEUE_KEY[*pIndex]);
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

	int i = 0;
	for (;i < REDIS_CMD_NUM; i ++) {
		int key = -1;
		if ((key = create_queue(MSG_QUEUE_IDS[i])) == -1){
			serverLog(LL_WARNING , "create queue failed. %08X" , MSG_QUEUE_IDS[i]);

			return -1;
		}

		MSG_QUEUE_KEY[i] = key;
	}

	for (i = 0;i < REDIS_CMD_NUM;i ++) {
		pthread_t tid;
		isRunning4Rep[i] = 1;
		int ret = pthread_create(&tid , 0 , run_loop , &(thread_index[i]));
		if (ret == -1) {
			serverLog(LL_WARNING , "create working thread failed , %s" , strerror(errno));

			delete_queue(MSG_QUEUE_KEY[i]);

			isRunning4Rep[i] = 0;

			return -1;
		}
	}

	return 0;
}

int sync_destory(){
	int i = 0;
	for (;i < REDIS_CMD_NUM; i ++) {
		isRunning4Rep[i] = 0;

		(void)sleep(DEFAULT_EXIT_INTERVAL);
		
		delete_queue(MSG_QUEUE_KEY[i]);
	}
	
	return 0;
}



