

all: server client

server: server.o md5.o
	gcc md5/md5.o -o server server.o

client: client.o md5.o
	gcc md5/md5.o -o client client.o

md5.o: md5/md5.c
	cd md5 && make

server.o: ser_eje.c 
	gcc -c ser_eje.c -o server.o

client.o: cli_eje.c
	gcc -c cli_eje.c -o client.o


clean:
	cd md5 && $(MAKE) clean
	rm  server.o client.o server client
