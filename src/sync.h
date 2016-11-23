#ifndef __SYNC_H__
#define __SYNC_H__

#include "server.h"

#include "sync_data_def.h"

int sync_data_to_queue(char * key , void * value , int vlength , int timeunit , int expire , char * ip , int cmd);

int sync_to_queue(struct set_command_sync_data * data);

int create_queue();
int delete_queue();

int sync_init();
int sync_destory();

#endif

