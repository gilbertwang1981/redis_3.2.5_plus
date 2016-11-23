#ifndef __SYNC_UTIL_H__
#define __SYNC_UTIL_H__

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int get_host_address(int fd , char * host);

#endif


