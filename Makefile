all:
	gcc -o client.out client.c -lpthread -g
	gcc -o server.out server.c -lpthread -g
