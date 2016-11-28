#include "sync_util.h"
#include <stdio.h>
#include <stdlib.h>

static int is_need_persistence = -1;
static int is_need_switch_statstics = -1;

int need_persistence(){
	if (is_need_persistence != -1) {
		return is_need_persistence;
	} 
	
	char * is_need_persist = getenv("REDIS_PERSISTENCE_FLAG");
	if (is_need_persist == 0) {
		is_need_persistence = 0;
	} else {
		is_need_persistence = atoi(is_need_persist);
	}

	return is_need_persistence;
}

int need_switch_statstics(){
	if (is_need_switch_statstics != -1) {
		return is_need_switch_statstics;
	}

	char * is_need_switch_stat = getenv("REDIS_SWITCH_STAT_FLAG");
	if (is_need_switch_stat == 0) {
		is_need_switch_statstics = 0;
	} else {
		is_need_switch_statstics = atoi(is_need_switch_stat);
	}

	return is_need_switch_statstics;
}

int get_host_address(int fd , char * host) {
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;
	
	len = sizeof addr;
	getpeername(fd , (struct sockaddr*)&addr, &len);
	
	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in * s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}
	
	(void)sprintf(host , "%s:%d", ipstr, port);
		
	return 0;
}




