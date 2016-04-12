# ![](http://i.imgur.com/3pluBLY.png) BankSystem
A simple bank server and client system written in C. Uses multithreading to allow simultaneous connections.

+ Up to 20 accounts can be made
+ Uses `mutex` locks to prevent multiple access to same account
+ Client may connect to any server and port via command line arguments.
+ Server is always on port `32722` (Can be changed in `server.c`)

Note: Client simulates multiple accounts by sleeping. You may remove the `throttle()` methods to speed up program.

## ![](http://i.imgur.com/haDJTsX.png) Usage

    Compile: Type 'make' in terminal | 'make clean' to remove objects files
    
    Server: ./server (Will start and wait for incoming connections)
    Client: ./client <ip address> <port>
    
Follow instructions in the client to continue using program. Server will print out account information every 20 seconds.
