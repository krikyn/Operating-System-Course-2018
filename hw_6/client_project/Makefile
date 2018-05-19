all: client

client: client.c
	gcc -Wall -fsanitize=address -o client -pthread client.c
	mv client ../
