server: logic.o server.o util.o thread.o
	gcc -o server logic.o server.o util.o thread.o -lpthread
	rm *.o
server.o: server.c
	gcc -c -g server.c
logic.o: logic.c
	gcc -c -g logic.c
util.o: util.c
	gcc -c -g util.c
thread.o: thread.c
	gcc -c -g thread.c
client: client.o util.o
	gcc -o client client.o util.o
client.o: client.c
	gcc -c client.c