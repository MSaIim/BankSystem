/* =============================
** C client for bank system
**  - Max of 20 accounts
**  - Uses multithreading
============================= */
#ifndef CLIENT_H
#define CLIENT_H

// Bunch of includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>

// struct to hold client info
struct clsock
{
	int sock;
	char *message,
	     *command,
	     *arg;
	char *server_reply;

	// Input and message threads
	pthread_t input_thread;
	pthread_t message_thread;
};

typedef struct clsock *CLSOCK;

#endif
