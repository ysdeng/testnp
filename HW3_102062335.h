#ifndef HW3_102062335_H
#define HW3_102062335_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

#define LISTENQ 1024
#define MAXLINE 4096
#define CHATPORT 9478

typedef struct sockaddr SA;
#define max(a,b) ((a)>(b)) ? (a) : (b)
#endif
