#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char const *argv[])
{
	const char* socket_name = argv[1];
	const char* msg = argv[2];

	int socket_id;
	struct sockaddr localaddr;

	// socket()
	socket_id = socket(PF_LOCAL, SOCK_STREAM, 0);

	// connect()
	localaddr.sa_family = AF_LOCAL;
	strcpy(localaddr.sa_data, socket_name);
	connect(socket_id, &localaddr, sizeof(localaddr));

	// write()
	int length = strlen(msg) + 1;
	write(socket_id, &length, sizeof(length));
	write(socket_id, msg, length);

	// close()
	close(socket_id);

	return 0;
}
