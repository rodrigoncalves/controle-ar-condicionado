all:
	gcc -o client.out client.c -lpthread
	gcc -o server.out server.c -lpthread
