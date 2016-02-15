/* ==========================
**	Helper functions for 
**	client..
** ========================*/
#include "client.h"

// Make a string lowercase
void lower(CLSOCK clsock)
{
	int index = 0;

	// Loop through each character and make it lowercase
	while(clsock->command[index])
	{
		clsock->command[index] = tolower(clsock->command[index]);
		index++;
	}

	index = 0;
	while(clsock->arg[index])
	{
		clsock->arg[index] = tolower(clsock->arg[index]);
		index++;
	}
}

// Throttle the command entry by sleeping for x seconds
void throttle(int sec)
{
	// If interrupted, go back to sleep
	if(sleep(sec) != 0)
		throttle(sec);
}