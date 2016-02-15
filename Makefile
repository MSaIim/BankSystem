CC = gcc
CFLAGS = -Wall -g

all: banksystem

banksystem: helper.o session.o server.o client.o
	$(CC) $(CCFLAGS) -o server server.o session.o -lpthread
	$(CC) $(CCFLAGS) -o client client.o helper.o -lpthread

helper.o: helper.c
	$(CC) $(CCFLAGS) -c helper.c

session.o: session.c
	$(CC) $(CCFLAGS) -c session.c

server.o: server.c server.h
	$(CC) $(CCFLAGS) -c server.c

client.o: client.c client.h
	$(CC) $(CCFLAGS) -c client.c

clean:
	@rm -rf *.o