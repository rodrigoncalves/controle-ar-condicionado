#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PORT_TEMP 8080
#define PORT_KEY 3000

void quit();
int server(int);

char *ip = "192.168.0.109";
int temp_d = 0;
int key_d = 0;

int main(int argc, char *argv[])
{
    signal(SIGINT, quit);

    if (argc == 2) ip = argv[1];
    else if (argc > 2) errx(1, "Invalid argument");

    pthread_t temp_thread;
    pthread_t key_thread;

    temp_d = setup(PORT_TEMP);
    key_d = setup(PORT_KEY);

    if (pthread_create(&temp_thread, NULL, request_temp, NULL))
        errx(1, "Error creating thread");
    if (pthread_create(&key_thread, NULL, request_key, NULL))
        errx(1, "Error creating thread");

    pthread_join(&temp_thread, NULL);
    pthread_join(&key_thread, NULL);

    unlink(ip);
    close(temp_d);
    close(key_d);

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

    return 0;
}

void quit()
{
    printf("\nBye\n");

    if (temp_d) close(temp_d);
    if (key_d) close(key_d);
    exit(0);
}

int setup(int port)
{
    int socket_id;
    struct sockaddr_in addr_server;

    // socket()
    if (socket_id = socket(AF_INET, SOCK_STREAM, 0) == -1)
        errx(1, "Error creating socket");

    // bind()
    addr_server.sa_family = AF_INET;
    addr_server.sin_port = htons(port);
    addr_server.sin_addr.s_addr = inet_addr(ip);
    bzero(&(addr_server.sin_zero), 8);
    if (bind(socket_id, (struct sockadrr *) &addr_server, sizeof(struct sockaddr)) == -1)
    {
        close(socket_id);
        errx(1, "Error executing bind()");
    }

    // listen()
    if (listen(socket_id, 10) < 0)
    {
        close(socket_id);
        errx(1, "Error executing listen()");
    }

    return socket_id;
}

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
