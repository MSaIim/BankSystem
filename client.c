#include "client.h"

// Prototypes
void serv_connect(CLSOCK, char *, int);
void *input_handler(void *);
void *message_handler(void *);
void new_acc(CLSOCK);
void curr_acc(CLSOCK, char *);

// Global boolean
static char istrue = ' ';

int main(int argc, char *argv[])
{
	// Get port number and initialize client struct
	int portno = atoi(argv[2]);
	CLSOCK clsock = malloc(sizeof(struct clsock));

	// Try to connect
	serv_connect(clsock, argv[1], portno);

	// Close socket and free the memory
	close(clsock->sock);
	free(clsock->message);
	free(clsock->server_reply);
	free(clsock);

	return 0;
}

// Connection method
void serv_connect(CLSOCK clsock, char *ip_addr, int portno)
{
	int dot = 3;
	// Create socket stuff
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// Create socket and check for errors
	clsock->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(clsock->sock == -1)
	{
		printf(" Could not create socket.\n");
		exit(0);
	}

	// Get ip address/domain
	server =  gethostbyname(ip_addr);
	if(server == NULL)
	{
		printf("\n Error: No such host.");
		exit(0);
	}

	// Prepare socketaddr_in structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	// Try to connect to server
	printf("\n Trying to connect...");
	while(1)
	{
		// Conect to remove server
		if(connect(clsock->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{	
			// Loading animation
			if(dot == 3)
			{
				printf("\b\b\b   \b\b\b");
				dot = 0;
			}
			printf(".");

			dot++;
			fflush(stdout);
		}
		else
		{
			printf("\n Connected to %s on port %d!\n", ip_addr, portno);
			puts("\n   Usage:\n   ------");
			puts("   open    [account name]\t- Create new account.");
			puts("   start   [account name]\t- Open existing account.");
			puts("   credit  [amount]\t\t- Add amount to account.");
			puts("   debit   [amount]\t\t- Deduct amount from account.");
			puts("   balance\t\t\t- Show current account balance.");
			puts("   finish\t\t\t- End customer session.");
			puts("   exit\t\t\t\t- Disconnect from server.\n");
			break;
		}

		// Try to connect again after 2 seconds
		throttle(3);
	}

	// Make thread to listen for messages from server
	if(pthread_create(&clsock->message_thread, NULL, message_handler, (void *) clsock) < 0)
	{
		perror(" \x1b[31m[Error]\x1b[0m Thread creation\n");
		exit(0);
	}

	// Make thread to send input to server
	//pthread_t input_thread;
	if(pthread_create(&clsock->input_thread, NULL, input_handler, (void *) clsock) < 0)
	{
		perror(" \x1b[31m[Error]\x1b[0m Thread creation\n");
		exit(0);
	}

	// Wait until no more inputs
	pthread_join(clsock->input_thread, NULL);
}

// Message handler (Receive message from server)
void *message_handler(void *client_sock)
{
	CLSOCK clsock = (CLSOCK) client_sock;
	clsock->server_reply = malloc(256);
	int read_size;

	// Actively listen for messages from server
	while((read_size = recv(clsock->sock, clsock->server_reply, 256, 0)) > 0)
	{
		if(clsock->server_reply[0] == '\n')
		{
			istrue = 'T';
			printf(" %s\n", clsock->server_reply);
		}
		else if(*(clsock->server_reply) == 'F')
		{
			istrue = 'F';
			printf("\n Goodbye! Options: [open, start, exit]\n");
		}
		else
			printf(" %s\n", clsock->server_reply);
		
		// Erase the contents
		memset(clsock->server_reply, 0, sizeof(clsock->server_reply));
	}

	// If server has shutdown
	if(read_size == 0)
	{
		pthread_cancel(clsock->input_thread);
		puts("\n\n \x1b[31m[Diconnected]\x1b[0m Server has shutdown!\n");
	}

	// Exit the thread
	pthread_exit(0);
}

// Input method (Send input to server)
void *input_handler(void *client_sock)
{
	CLSOCK clsock = (CLSOCK) client_sock;
	char *temp;

	// Communicate with server
	while(1)
	{
		int i = 0;
		// Make enough space for command input
		clsock->command = malloc(10);
		clsock->arg = malloc(100);

		// Get a command input, split it, and make it lowercase
		printf(" :");
		scanf("%s%99[^\n]", clsock->command, clsock->arg);
		lower(clsock);

		// "OPEN ACCOUNTNAME" Creates a new account
		if(strcmp(clsock->command, "open") == 0 || strcmp(clsock->command, "start") == 0)
		{
			write(clsock->sock, clsock->command, 10);
			throttle(1);

			write(clsock->sock, clsock->arg, 100);
		}
		// CUSTOMER MENU
		else if(istrue == 'T')
		{
			// "BALANCE" Check balance of account only if logged in
			if(strcmp(clsock->command, "balance") == 0)
				write(clsock->sock, clsock->command, 10);
			// "FINISH" Log out of account and go back to main menu
			else if(strcmp(clsock->command, "finish") == 0)
			{
				write(clsock->sock, clsock->command, 10);
				istrue = 'F';
			}
			// "CREDIT" Add amount to account
			else if(strcmp(clsock->command, "credit") == 0 || strcmp(clsock->command, "debit") == 0)
			{
				write(clsock->sock, clsock->command, 10);
				throttle(1);
				write(clsock->sock, clsock->arg, 100);
			}
			else
				write(clsock->sock, clsock->command, 7);
		}
		// "EXIT" Disconnect from server and close client
		else if(strcmp(clsock->command, "exit") == 0)
		{
			write(clsock->sock, clsock->command, 10);
			throttle(1);
			break;
		}
		else
			write(clsock->sock, clsock->command, 10);
	
		// Free the string
		free(clsock->command);
		free(clsock->arg);

		// Throttle the speed
		throttle(2);
	}

	// Exit the thread
	pthread_exit(0);
}