#include "t_tsmap.h"
#include "ts_collector.h"

#include <stdlib.h>

struct sharedObjectsStruct shared;

void peg(client * c) {

	if (c->argc != 3) {
		addReplyErrorFormat(c, "wrong number of arguments for peg command");

		return;
	}

	serverLog(LL_DEBUG , "got the pegging command. %s %d" , c->argv[1]->ptr , atoi(c->argv[2]->ptr));

	(void)create_counter(c->argv[1]->ptr);

	(void)peg_counter(c->argv[1]->ptr , atoi(c->argv[2]->ptr));
	
	addReply(c, shared.ok);
}

