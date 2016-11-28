#include "t_tsmap.h"
#include "ts_collector.h"
#include "sync_util.h"

#include <stdlib.h>

struct sharedObjectsStruct shared;

void peg(client * c) {

	if (need_switch_statstics() == 0 && c->argc != 3) {
		addReplyErrorFormat(c, "wrong number of arguments for peg command , %d." , c->argc);

		return;
	}

	(void)create_counter(c->argv[1]->ptr);

	if (need_switch_statstics() == 0) {
		(void)peg_counter(c->argv[1]->ptr , atoi(c->argv[2]->ptr));

		addReply(c, shared.ok);
	} else {
		int ret = peg_counter(c->argv[1]->ptr , 1);
		robj * rsp = createStringObjectFromLongLong(ret);
		
		addReply(c,shared.colon);
		addReply(c,rsp);
		addReply(c,shared.crlf);

		decrRefCount(rsp);
	}
}

