all:
	@make server
	@make client

server:
	gcc -o server.out src/server.c -lpthread -g

client:
	gcc -o client.out src/client.c -lpthread -g
