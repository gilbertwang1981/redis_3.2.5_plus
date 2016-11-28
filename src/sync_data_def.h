#ifndef __SYNC_DATA_DEF__
#define __SYNC_DATA_DEF__

#define SET_CMD_KEY_LENGTH 256
#define SET_CMD_VALUE_LENGTH 1024 * 5
#define REDIS_PLUGIN_SYNC_SET_TYPE 1
#define DEFAULT_EXIT_INTERVAL 5
#define IP_ADDRESS_LENGTH 64
#define DB_HC_FREQUENCE 100
#define TS_PLUGIN_PATH_LENGTH 1024

#define DEFAULT_CACHE_BUFFER_LENGTH 1024 * 10

#define DEFAULT_SND_MSG_RETRY_TIMES 100

#define REDIS_CMD_TYPE_SET 0
#define REDIS_CMD_TYPE_DEL 1
#define REDIS_CMD_TYPE_INCR 2
#define REDIS_CMD_TYPE_DECR 3
#define REDIS_CMD_TYPE_MSET 4

#define REDIS_CMD_NUM 6

// parameter:
// 1. counter name
// 2. second
// 3. counter value
// 4. is eof
// 5. is bof
typedef int (* COLLECTOR_NEXT)(char * , int , int , int , int);

struct set_command_sync_data{
	char key[SET_CMD_KEY_LENGTH + 1];
	char value[SET_CMD_VALUE_LENGTH + 1];
	int timeunit;
	int expire;
	int cmd;
	char ipaddress[IP_ADDRESS_LENGTH + 1];
};

struct msg_buffer {
	long mtype;
	char mtext[SET_CMD_KEY_LENGTH + SET_CMD_VALUE_LENGTH + IP_ADDRESS_LENGTH + 15];
};

#endif

