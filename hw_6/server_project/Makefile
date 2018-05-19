all: server

server: server.c
	gcc -Wall -fsanitize=address -o server -pthread server.c
	mv server ../
