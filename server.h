#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_MAX 8192
#define SOCKET_QUEUE_MAX 88
#define DEFAULT_PORT 6429

int init_socket(int sserver, struct sockaddr_in sserver_address, int port, int debug);
void *success_handler(void *ptr_sclient);

#endif