#include "server.h"

// Protypes
void server_setup();
void *server_listen(void *);
void *client_handler(void *);
void *sig_alarm(void *);
void print_info(int);

int main(int argc, char *argv[])
{
	int i;	// Loop counter variable

	// Initialize mutex for new & current customers
	pthread_mutex_init(&nlock, NULL);
	for(i = 0; i < SIZE; i++)
		pthread_mutex_init(&acc[i].lock, NULL);

	// Listen for connections
	server_setup();

	// Destroy mutex
	for(i = 0; i < SIZE; i++)
		pthread_mutex_destroy(&acc[i].lock);

	return 0;
}

// Listen for connections
void server_setup()
{
	// Declare socket variables
	int socket_desc;
	int portno = 32722;
	struct sockaddr_in server;
	pthread_t serv_listen, info;

	// Create socket and check if able to create
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc == -1)
	{
		puts(" \x1b[31m[Error]\x1b[0m Could not create socket.");
		exit(0);
	}

	// Prepare sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(portno);

	// Bind
	if(bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		// Print error message
		perror("\n \x1b[31m[Error]\x1b[0m Bind");
		printf("\n");
		exit(0);
	}

	// Listen
	printf("\n Listening on port %d...", portno);
	fflush(stdout);
	listen(socket_desc, 3);

	// Create server acceptor thread
	if(pthread_create(&serv_listen, NULL, server_listen, (void *) &socket_desc) < 0)
	{
		perror(" \x1b[31m[Error]\x1b[0m Listen thread creation");
		exit(0);
	}

	// Create server info thread
	if(pthread_create(&info, NULL, sig_alarm, NULL) < 0)
	{
		perror(" \x1b[31m[Error]\x1b[0m Info thread creation");
		exit(0);
	}

	// Wait for acceptor thread
	pthread_join(serv_listen, NULL);
}

void *server_listen(void *socket_desc)
{	
	// Struct to hold server info
	SERVINFO serv;

	// Set up to listen
	int client_sock, sock = *((int *) socket_desc);
	struct sockaddr_in client;
	int c = sizeof(struct sockaddr_in);

	// Accept connection from an incoming cient
	while((client_sock = accept(sock, (struct sockaddr *) &client, (socklen_t *) &c)))
	{
		// Connection accepted, create struct to hold info
		serv = malloc(sizeof(struct servinfo));
		serv->ip_addr = inet_ntoa(client.sin_addr);
		serv->portno = (int) ntohs(client.sin_port);
		printf("\n \x1b[32m[Connected]\x1b[0m %s:%d\n", serv->ip_addr, serv->portno);

		// Create new thread
		pthread_t client_thread;
		serv->new_sock = malloc(1);
		*(serv->new_sock) = client_sock;

		// Check the thread
		if(pthread_create(&client_thread, NULL, client_handler, (void *) serv) < 0)
			perror(" \x1b[31m[Error]\x1b[0m Client thread creation\n");
	}

	if(client_sock < 0)
		perror(" \x1b[31m[Error]\x1b[0m Accept");
}

// Accept incoming commands
void *client_handler(void *servinfo)
{
	// Convert void struct to a server info struct
	SERVINFO serv = (SERVINFO) servinfo;

	// Prepare to receive and send
	int sock = *(serv->new_sock);
	int read_size, index;
	char *client_message = malloc(25);

	// Receive message from client
	while((read_size = recv(sock, client_message, 25, 0)) > 0)
	{
		// Print info about input from client
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d  -  %s", serv->ip_addr, serv->portno, client_message);
		fflush(stdout);

		// "OPEN" Create new account
		if(strcmp(client_message, "open") == 0)
			new_acc(serv, sock);
		// "START" Login to account
		else if(strcmp(client_message, "start") == 0)
			curr_acc(serv, sock);
		// "EXIT" Client wants to disconnect
		else if(strcmp(client_message, "exit") == 0)
		{
			write(sock, "Disconnected from server.", 50);
			break;
		}
		// INVALID INPUTS
		else
		{
			write(sock, "[Error] Invalid input!", 50);
			//printf("\n");
		}

		// Clear the client message to get new message
		memset(client_message, 0, sizeof(client_message));
	}

	// Disconnect if no message can come through from that client
	if(read_size >= 0)
	{
		printf(" \x1b[31m[Disconnected]\x1b[0m %s:%d\n", serv->ip_addr, serv->portno);
		fflush(stdout);
	}
	else if(read_size == -1)
		perror(" \x1b[31m[Error]\x1b[0m Receive");

	// Free the socket pointer
	free(servinfo);
	free(client_message);

	// Exit the thread
	pthread_exit(0);
}

// Setup alarm and signal
void *sig_alarm(void *arg)
{
	// Create signal
	signal(SIGALRM, print_info);

	// Set alarm
	alarm(20);

	// Keep going
	while(free)
		pause();

	// Exit thread
	pthread_exit(0);
}

// Print every n seconds
void print_info(int signum)
{
	int index, isfound = 0;
	printf("\n Server Info");
	printf("\n ------------");

	// Print info
	for(index = 0; index < SIZE; index++)
	{
		// Print
		if(strlen(acc[index].accName) != 0)
		{
			printf("\n \x1b[36mAccount\x1b[0m:%s\n \x1b[36m%s\x1b[0m: %.2f\n \x1b[36m%s\x1b[0m: ", 
				acc[index].accName, "Balance", acc[index].balance, "Session");

			// Check inSession
			if(acc[index].inSession == 1)
				printf("IN SERVICE\n");
			else
				printf("NOT IN SERVICE\n");

			isfound++;
		}
	}

	if(isfound == 0)
		printf("\n No accounts found");

	printf("\n");

	// Reset alarm
	alarm(20);
}