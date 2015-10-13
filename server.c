#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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

void down_server()
{
    printf("\nBye\n");
    exit(0);
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, down_server);

    const char* socket_name = argv[1];
    int socket_id;
    struct sockaddr tempaddr;
    // struct sockaddr_in statusaddr;

    // socket()
    socket_id = socket(PF_LOCAL, SOCK_STREAM, 0); // local socket

    // bind()
    tempaddr.sa_family = AF_LOCAL;
    strcpy(tempaddr.sa_data, socket_name);
    bind(socket_id, &tempaddr, sizeof(tempaddr));

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
