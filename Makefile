all:	server client

server: server.o tcp.o
	cc server.o tcp.o -o server

client: client.o tcp.o
	cc client.o tcp.o -o client

server.o: server.c
	cc -c server.c

client.o: client.c
	cc -c client.c

tcp.o: tcp.c tcp.h
	cc -c tcp.c tcp.h

clean:
	rm *.o
	rm server
	rm client