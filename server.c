#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int server(int client_socket)
{
	// read()
	while (1)
	{
		int length;
		char *text;
		if (read(client_socket, &length, sizeof(length)) == 0)
			return 0;
		text = (char *) malloc(length);
		read(client_socket, text, length);
		fprintf(stderr, "%s\n", text);
		if (!strcmp(text, "quit"))
		{
			free(text);
			return 1;
		}

		free(text);
	}
}

int main(int argc, char const *argv[])
{
	const char* socket_name = argv[1];
	int socket_id;
	struct sockaddr localaddr;

	// socket()
	socket_id = socket(PF_LOCAL, SOCK_STREAM, 0); // local socket

	// bind()
	localaddr.sa_family = AF_LOCAL;
	strcpy(localaddr.sa_data, socket_name);
	bind(socket_id, &localaddr, sizeof(localaddr));

	// listen()
	listen(socket_id, 10);

	int client_request_termination = 0;
	// accept()
	while (!client_request_termination)
	{
		struct sockaddr client;
		int client_socket_id;
		socklen_t client_len;

		client_socket_id = accept(socket_id, &client, &client_len);

		client_request_termination = server(client_socket_id);
		close(client_socket_id);
	}

	close(socket_id);
	unlink(socket_name);

	return 0;
}
