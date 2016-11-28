#include "mysql_ds.h"
#include "server.h"

#include "mysql.h"

#include "adlist.h"
#include "zmalloc.h"

#include <pthread.h>
#include <stdlib.h>

static MYSQL mysql_handle[REDIS_CMD_NUM];

static list * white_list = 0;

int mysql_ds_init(){
	int i = 0;
	for (;i < REDIS_CMD_NUM; i ++) {
		(void)mysql_init(&mysql_handle[i]);
	}

	char * host = getenv("VIP_DB_HOST");
	if (host == 0) {
		serverLog(LL_WARNING , "Need configure the VIP_DB_HOST env variable");
		
		return -1;
	}

	char * user = getenv("VIP_DB_USER");
	if (user == 0) {
		serverLog(LL_WARNING , "Need configure the VIP_DB_USER env variable");
		
		return -1;
	}

	char * pass = getenv("VIP_DB_PASS");
	if (pass == 0) {
		serverLog(LL_WARNING , "Need configure the VIP_DB_PASS env variable");
		
		return -1;
	}

	char * dbname = getenv("VIP_DB_NAME");
	if (dbname == 0) {
		serverLog(LL_WARNING , "Need configure the VIP_DB_NAME env variable");
		
		return -1;
	}

	for (i = 0;i < REDIS_CMD_NUM; i++) {
		if(!mysql_real_connect(&mysql_handle[i] , host , user ,
	                     pass , dbname , 0 , NULL , 0)){
	        serverLog(LL_DEBUG , "Error connecting to database:%s\n" , mysql_error(&mysql_handle[i]));

			return -1;
	    }
	    else {
	         serverLog(LL_DEBUG , "Connected........");
	    }
	}
	
	return 0;
}

int mysql_ds_destory(){
	int i = 0;
	for (;i < REDIS_CMD_NUM; i++) {
		(void)mysql_close(&mysql_handle[i]);
	}
	
	return 0;
}

int insert_log(struct set_command_sync_data * data){
	char sql[DEFAULT_CACHE_BUFFER_LENGTH] = {0};
	(void)sprintf(sql , "insert into redis_log (dkey , dvalue , dexpire, dtimeunit , ip , cmd) values ("
		"'%s' , '%s' , %d , %d , '%s' , %d)" , data->key , data->value , data->expire , data->timeunit ,
		data->ipaddress , data->cmd);

	serverLog(LL_DEBUG , "execute sql:%s thread_id:%lld" , sql , (long long int)pthread_self());

	int t = mysql_query(&mysql_handle[data->cmd] , sql);
    if(t != 0) {
         serverLog(LL_WARNING , "Error making query:%s\n" , mysql_error(&mysql_handle[data->cmd]));

		 return -1;
    } else {
		return 0;
    }
}

int load_white_list_from_db(){
	white_list = listCreate();
	if (white_list == 0) {
		return -1;
	}
	
	int offset = 0;
	for (int i = 0;;i++) {
		char sql[DEFAULT_CACHE_BUFFER_LENGTH] = {0};
		(void)sprintf(sql , "select dkey from whitelist limit %d , %d" , offset , MYSQL_REDIS_WHITE_LIST_EACH_PAGE_SIZE);

		offset += MYSQL_REDIS_WHITE_LIST_EACH_PAGE_SIZE;

		serverLog(LL_DEBUG , "execute sql:%s" , sql);
		
		int ret = mysql_query(&mysql_handle[0] , sql);
		if (ret != 0) {
			serverLog(LL_WARNING , "Error making query:%s\n" , mysql_error(&mysql_handle[0]));

		 	return -1;
		} else {
			MYSQL_RES * res = mysql_store_result(&mysql_handle[0]);
			if(res != 0) {
				if (mysql_num_rows(res) == 0) {
					break;
				}
				
				MYSQL_ROW row; 
    			int fields = mysql_num_fields(res); 
	            while ((row = mysql_fetch_row(res))) {  
			        for (i = 0; i < fields; i++) {
						struct white_list_data * whiteList = (struct white_list_data *)zmalloc(sizeof(struct white_list_data));
						(void)memset(whiteList->key , 0x00 , SET_CMD_KEY_LENGTH);
						(void)strncpy(whiteList->key , row[i] , strlen(row[i]));

						white_list = listAddNodeHead(white_list , whiteList);
			        }
			    }  
	         	(void)mysql_free_result(res);
			} else {
				break;
			}
		}
	}
	
	return 0;
}

int is_in_white_list(char * key){

	listIter * itr = listGetIterator(white_list , 0);
	if (itr != 0) {
		listNode * node = 0;
		while ((node = listNext(itr)) != 0) {
			struct white_list_data * white_list_data = (struct white_list_data *)(node->value);
			if (strcmp(white_list_data->key , key) == 0) {

				listReleaseIterator(itr);
				
				return 0;
			}
		}
		
		listReleaseIterator(itr);
	}
		
	return -1;
}

int health_check(int index){
	char sql[DEFAULT_CACHE_BUFFER_LENGTH] = {0};
	(void)sprintf(sql , "SELECT 1");

	serverLog(LL_DEBUG , "execute sql:%s index:%d" , sql , index);

	int t = mysql_query(&mysql_handle[index] , sql);
	if(t != 0) {
		 serverLog(LL_WARNING , "Error making query:%s\n" , mysql_error(&mysql_handle[index]));

		 return -1;
	} else {
		MYSQL_RES * res = mysql_store_result(&mysql_handle[index]);

		(void)mysql_free_result(res);

		return 0;
	}
}





