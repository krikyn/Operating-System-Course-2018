all: utils server client

server: server.c
	gcc -Wall -fsanitize=address -o server -pthread server.c

client: client.c
	gcc -Wall -fsanitize=address -o client -pthread client.c

clean:
	-rm client server

utils: 
	cp -rf server_project/server.c .
	cp -rf client_project/client.c .
