

all: server client

server: server.o
	gcc -o server server.o

client: client.o
	gcc -o client client.o

server.o: ser_eje.c 
	gcc -c ser_eje.c -o server.o

client.o: cli_eje.c
	gcc -c cli_eje.c -o client.o


clean:
	rm  server.o client.o server client
