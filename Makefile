all:
	@make server
	@make client

server:
	gcc -o server.out src/server.c -Iinclude -lpthread -g

client:
	gcc -o client.out src/client.c -Iinclude -lpthread -g
