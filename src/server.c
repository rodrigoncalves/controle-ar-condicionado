#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include "log.c"

#define PORT_TEMP 8080
#define PORT_KEY 3000

void quit();
int setup(int);
void *recv_request_temp();
void *recv_request_key();

char *ip = "127.0.0.1";
int server_temp_d = 0;
int server_key_d = 0;
int client_temp_d = 0;
int client_key_d = 0;

int main(int argc, char *argv[])
{
    LogCreate();
    signal(SIGINT, quit);

    if (argc == 2) ip = argv[1];
    else if (argc > 2) LogErr("Invalid argument");

    char str[30];
    sprintf(str, "Running at %s", ip);
    Log(str);

    pthread_t temp_thread;
    pthread_t key_thread;

    server_temp_d = setup(PORT_TEMP);
    server_key_d = setup(PORT_KEY);

    if (pthread_create(&temp_thread, NULL, recv_request_temp, NULL))
        LogErr("Error creating thread");
    if (pthread_create(&key_thread, NULL, recv_request_key, NULL))
        LogErr("Error creating thread");

    pthread_join(temp_thread, NULL);
    pthread_join(key_thread, NULL);

    unlink(ip);
    close(server_temp_d);
    close(server_key_d);

    return 0;
}

void quit()
{
    printf("\nBye\n");

    if (server_temp_d) close(server_temp_d);
    if (server_key_d) close(server_key_d);
    if (client_temp_d) close(client_temp_d);
    if (client_key_d) close(client_key_d);
    exit(0);
}

int setup(int port)
{
    int socket_id;
    struct sockaddr_in addr_server;

    // socket()
    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        LogErr("Error creating socket");

    // bind()
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    addr_server.sin_addr.s_addr = inet_addr(ip);
    bzero(&(addr_server.sin_zero), 8);

    if (bind(socket_id, (struct sockaddr *) &addr_server, sizeof(struct sockaddr)) == -1)
    {
        close(socket_id);
        LogErr("Error executing bind()");
    }

    // listen()
    if (listen(socket_id, 10) < 0)
    {
        close(socket_id);
        LogErr("Error executing listen()");
    }

    return socket_id;
}

void *recv_request_temp()
{
    // accept()
    while (1)
    {
        struct sockaddr addr_client;
        socklen_t client_len;

        client_temp_d = accept(server_temp_d, (struct sockaddr *) &addr_client, &client_len);
        if (client_temp_d < 0)
        {
            Log("Error executing accept()");
            continue;
        }

        int length;
        char *text;

        if (recv(client_temp_d, &length, sizeof(length), 0) == 0)
        {
            Log("Nothing received");
            continue;
        }

        text = (char *) malloc(length);
        if (recv(client_temp_d, text, length, 0) == 0)
        {
            Log("Nothing received");
            continue;
        }

        if (strcmp("get_temperature", text) == 0)
        {
            Log("Server: temperature request received");

            // float temperature = get_temp_uart();
            float temperature = 25.0;
            if (send(client_temp_d, &temperature, sizeof(temperature), 0) == -1)
            {
                close(client_temp_d);
                Log("Error sending temperature");
                continue;
            }
        }

        free(text);
        close(client_temp_d);
    }

}

void *recv_request_key()
{
    // accept()
    while (1)
    {
        struct sockaddr_in addr_client;
        socklen_t client_len;

        client_key_d = accept(server_key_d, (struct sockaddr *) &addr_client, &client_len);
        if (client_key_d < 0)
        {
            Log("Error executing accept()");
            continue;
        }

        int length;
        char *text;

        if (recv(client_key_d, &length, sizeof(length), 0) == 0)
        {
            Log("Nothing received");
            continue;
        }

        text = (char *) malloc(length);
        if (recv(client_key_d, text, length, 0) == 0)
        {
            Log("Nothing received");
            continue;
        }

        if (strcmp("on", text) == 0)
        {
            Log("Server: air conditioning turned ON.");

            bool key = true;
            if (send(client_key_d, &key, sizeof(key), 0) == -1)
            {
                close(client_key_d);
                Log("Error sending temperature");
                continue;
            }
        }
        else if (strcmp("off", text) == 0)
        {
            Log("Server: air conditioning turned OFF.");

            bool result = false;
            if (send(client_key_d, &result, sizeof(result), 0) == -1)
            {
                close(client_key_d);
                Log("Error sending key air");
                continue;
            }
        }

        free(text);
        close(client_key_d);
    }
}
