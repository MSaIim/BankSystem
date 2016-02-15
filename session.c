/* ==========================
**	Helper functions for 
**	client..
** ========================*/
#include "server.h"

void new_acc(SERVINFO, int);
void curr_acc(SERVINFO, int);
void cus_menu(SERVINFO, int, int);
void credit(SERVINFO, int, int);
void debit(SERVINFO, int, int);

// Open new account if not full or duplicate
void new_acc(SERVINFO serv, int sock)
{
	int isfound = 0, lock_ret, i;
	char *client_message = malloc(100);

	// Get the account name
	if(recv(sock, client_message, 100, 0) > 0)
	{
		// Check if there is enough space and no duplicates
		for(i = 0; i < SIZE && isfound == 0; i++)
		{
			// Check for duplicate
			if(strcmp(acc[i].accName, client_message) == 0)
			{
				// Duplicate found
				isfound = -1;
				//puts("\n \x1b[31m[Error]\x1b[0m Account creation failed.");
				write(sock, "[Error] Duplicate account detected.", 50);
				return;
			}
			// Check for empty space
			else if(strlen(acc[i].accName) == 0)
				isfound = 1;
		}
	}

	// Try and lock the mutex then go inside
	lock_ret = pthread_mutex_trylock(&nlock);
	if(!lock_ret && isfound == 1)
	{
		// Print input
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d - %s", serv->ip_addr, serv->portno, client_message);

		// Make new account
		strcpy(acc[i-1].accName, client_message);
		acc[i].balance = 0.0;
		acc[i].inSession = 0;

		//puts("\n \x1b[32m[Success]\x1b[0m Account created.");
		write(sock, "[Success] Account created.", 50);

		// Unlock mutex
		lock_ret = pthread_mutex_unlock(&nlock);
	}
	else
	{
		//puts("\n \x1b[31m[Error]\x1b[0m Account creation failed.");
		write(sock, "[Error] Cannot open account at this time...", 50);
	}

	// Free memory
	free(client_message);
}

// Login to existing account and make transactions
void curr_acc(SERVINFO serv, int sock)
{
	int isfound = 0, lock_ret, i;
	char *client_message = malloc(256);

	// Wait for the account name
	if((recv(sock, client_message, 256, 0)) > 0)
	{
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d - %s", serv->ip_addr, serv->portno, client_message);
		fflush(stdout);
	}

	// See if account name exists
	for(i = 0; i < SIZE && isfound != 1; i++)
	{
		if((strcmp(acc[i].accName, client_message)) == 0)
			isfound = 1;
	}

	// Try and lock the thread then go inside
	lock_ret = pthread_mutex_trylock(&acc[i-1].lock);
	if(!lock_ret && isfound == 1)
	{
		// Free memory and tell client
		free(client_message);
		
		// Show customer menu
		write(sock, "\n Welcome! Options: [credit, debit, balance, finish]", 60);
		cus_menu(serv, sock, i-1);

		// Unlock the thread
		lock_ret = pthread_mutex_unlock(&acc[i-1].lock);
	}
	else
	{
		// Account not found
		//printf("\n \x1b[31m[Error]\x1b[0m Cannot open account: %s\n", client_message);
		write(sock, "[Error] Cannot login. Try again or wait...", 50);

		// Flush and free memory
		fflush(stdout);
		free(client_message);
	}
}

// Customer menu after successful login
void cus_menu(SERVINFO serv, int sock, int index)
{
	char *client_message = malloc(10);

	// Account is in session
	acc[index].inSession = 1;

	// Wait for commands for the customer menu
	while(recv(sock, client_message, 10, 0) > 0)
	{
		// Print info about input from client
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d  -  %s", serv->ip_addr, serv->portno, client_message);
		//fflush(stdout);

		if(strcmp(client_message, "credit") == 0)
			credit(serv, sock, index);
		else if(strcmp(client_message, "debit") == 0)
			debit(serv, sock, index);
		else if(strcmp(client_message, "balance") == 0)
		{
			// Convert float to char pointer then write char * to client
			char *buffer = malloc(100);
			snprintf(buffer, sizeof(buffer), "%f", acc[index].balance);
			write(sock, buffer, sizeof(buffer));
			free(buffer);
		}
		else if(strcmp(client_message, "finish") == 0)
		{
			write(sock, "F", 1);
			//printf("\n");
			break;
		}
		else
			write(sock, "[Error] Invalid input!", 50);

		// Clear the client message to get new message
		memset(client_message, 0, sizeof(client_message));
	}

	// Free memory
	free(client_message);
	acc[index].inSession = 0;
}

// Add amount to account
void credit(SERVINFO serv, int sock, int index)
{
	char *client_message = malloc(256);

	if((recv(sock, client_message, 256, 0)) > 0)
	{
		// Print info about input from client
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d - %s", serv->ip_addr, serv->portno, client_message);
		//fflush(stdout);

		// Convert to float and add to total
		if(strtof(client_message, 0) > 0)
		{
			acc[index].balance += strtof(client_message, 0);
			write(sock, "[Success] Amount added to account.", 50);
		}
		else
			write(sock, "[Error] Amount must be greater than zero.", 60);
	}

	free(client_message);
}

// Withdraw amount from account
void debit(SERVINFO serv, int sock, int index)
{
	char *client_message = malloc(256);

	if((recv(sock, client_message, 256, 0)) > 0)
	{
		// Print info about input from client
		//printf("\n \x1b[36m[Input]\x1b[0m %s:%d - %s", serv->ip_addr, serv->portno, client_message);
		//fflush(stdout);

		// Convert to float and add to total
		if(acc[index].balance - strtof(client_message, 0) >= 0)
		{
			acc[index].balance -= strtof(client_message, 0);
			write(sock, "[Success] Amount withdrawn from account.", 50);
		}
		else
			write(sock, "[Error] Amount must not exceed current balance.", 60);
	}

	free(client_message);
}