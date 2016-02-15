/* =============================
** C server for bank system
**  - Max of 20 accounts
**  - Uses multithreading
============================= */
#ifndef SERVER_H
#define SERVER_H

// Bunch of includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

// Max accounts
#define SIZE 20

// struct to hold accounts
struct account
{
	char accName[100];
	float balance;
	int inSession;

	pthread_mutex_t lock;
};

struct servinfo
{
	int *new_sock;
	char *ip_addr;
	int portno;
};

struct account acc[SIZE];
typedef struct servinfo *SERVINFO;

pthread_mutex_t nlock;

#endif