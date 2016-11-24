#ifndef __SYNC_DATA_DEF__
#define __SYNC_DATA_DEF__

#define SET_CMD_KEY_LENGTH 128
#define SET_CMD_VALUE_LENGTH 256
#define REDIS_PLUGIN_SYNC_SET_TYPE 1
#define DEFAULT_EXIT_INTERVAL 5
#define IP_ADDRESS_LENGTH 64
#define DB_HC_FREQUENCE 50000

#define DEFAULT_CACHE_BUFFER_LENGTH 1024 * 100

#define REDIS_CMD_TYPE_SET 0
#define REDIS_CMD_TYPE_DEL 1
#define REDIS_CMD_TYPE_INCR 2
#define REDIS_CMD_TYPE_DECR 3

#define REDIS_CMD_NUM 6

struct set_command_sync_data{
	char key[SET_CMD_KEY_LENGTH];
	char value[SET_CMD_VALUE_LENGTH];
	int timeunit;
	int expire;
	int cmd;
	char ipaddress[IP_ADDRESS_LENGTH];
};

struct msg_buffer {
	long mtype;
	char mtext[SET_CMD_KEY_LENGTH + SET_CMD_VALUE_LENGTH + IP_ADDRESS_LENGTH + 12];
};

#endif

