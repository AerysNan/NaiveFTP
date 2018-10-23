CC = gcc -c -Wall

server: logic.o server.o util.o thread.o transport.o
	gcc -o server logic.o server.o util.o thread.o transport.o -lpthread
server.o: server.c
	$(CC) server.c
logic.o: logic.c
	$(CC) logic.c
util.o: util.c
	$(CC) util.c
thread.o: thread.c
	$(CC) thread.c
tranport.o: tranport.c
	$(CC) transport.c

clean:
	rm *.o