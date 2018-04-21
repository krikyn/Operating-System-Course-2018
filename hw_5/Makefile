all: server client

server: server.c
	gcc -Wall -fsanitize=address -o server -pthread server.c

client: client.c
	gcc -Wall -fsanitize=address -o client -pthread client.c

clean:
	-rm client server
