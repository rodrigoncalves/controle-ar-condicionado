#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>

#include "log.c"

#define PORT_TEMP 8080
#define PORT_KEY 3000

void quit();
int setup(int);
void *recv_request_temp();
void *recv_request_key();
float get_temp_uart();
bool change_air_state(bool);

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
            // float temperature = 25.0;
            float temperature = get_temp_uart();
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

        bool key = false;
        if (strcmp("on", text) == 0)
        {
            Log("Server: request to turn ON");
            key = true;
        }
        else if (strcmp("off", text) == 0)
        {
            Log("Server: request to turn OFF");
            key = false;
        }

        // change the air state
        key = change_air_state(key);
        if (send(client_key_d, &key, sizeof(key), 0) == -1)
        {
            close(client_key_d);
            Log("Error: change air state");
            continue;
        }

        Log("Sever: success");

        free(text);
        close(client_key_d);
    }
}

float get_temp_uart()
{
    int uart_d;
    uart_d = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_d == -1)
    {
        Log("Error: failed to open file ¬dev/ttyAMA0");
        return -273.15;
    }

    // Turn off blocking for reads, use (uart_d, F_SETFL, FNDELAY) if you want that
    fcntl(uart_d, F_SETFL, 0);

    //----- TX BYTES -----
    unsigned char buffer[3];
    unsigned char *p_buffer;
    
    p_buffer = &buffer[0];
    *p_buffer++ = 0x05;

    // Write to the port
    int n = write(uart_d, &buffer[0], (p_buffer - &buffer[0]));
    if (n < 0)
    {
        close(uart_d);
        Log("Error: write to uart");
        return -273.15;
    }

    sleep(1);

    // Read up to 4 bytes from the port if they are there
    float temp;
    char buffer_temp[4];
    int m = read(uart_d, (void *) buffer_temp, 4);


    if (m < 0)
    {
        Log("Error: read temperature");
        temp = -273.15;
    }
    else if (m == 0)
    {
        Log("No data on port");
        temp = -273.15;
    }

    // Set float to temperature
    memcpy(&temp, buffer_temp, 4);

    close(uart_d);
    return temp;
}


/*
*   Receive a request to turn on/off
*/
void* receive_request_air()
{
    struct sockaddr_in addr_client;

    while(1)
    {
        int length = sizeof(struct sockaddr_in);

        // Aguada um cliente se conectar ao socket que controla o ar condicionado
        client_key_d = accept(server_key_d, (struct sockaddr *)&addr_client,&length);
        
        if (client_key_d < 0)
        {
            Log("Erro ao executar o accept");
            continue;
        }

        char* text;

        // Lê qual o tamanho da mensagem que será recebida
        if(recv(client_key_d, &length, sizeof(length), 0) == 0)
        {
            Log("Erro na leitura");
            continue;
        }
        
        text = (char*) malloc(length);
        // Recebe a mensagem
        if(recv(client_key_d, text, length, 0) == 0)
        {
            Log("Erro na leitura");
            continue;
        }

        bool result = false;
        // Verifica se a mensagem recebida é para ligar ou desligar o ar condicionado
        if(strcmp("on", text) == 0)
        {
            Log("Servidor: requisicao de ligar o ar condicionado recebida!\n");
            result = true;
        }
        else if(strcmp("off", text) == 0)
        {
            printf("Servidor: requisicao de desligar o ar condicionado recebida!\n");
            result = false;
        }

        // muda o estado do ar condicionado via uart
        result = change_air_state(result);
        if (send(client_key_d, &result, sizeof(result), 0) == -1)
        {
            close(client_key_d);
            Log("Erro ao enviar resposta");
            continue;
        }

        free(text);
        close(client_key_d);
    }
}
/*
*   Envia o novo estado para o ar condicionado
*   Return: True se a operacao foi executada com sucesso, E False, caso contrario
*/
bool change_air_state(bool state)
{
    int uart_descriptor;
    // Open the Port. We want read/write, no "controlling tty" status, and open it no matter what state DCD is in
    uart_descriptor = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_descriptor == -1)
    {
        Log("Erro ao abrir o arquivo /dev/ttyAMA0");
        return false;
    }

    // Turn off blocking for reads, use (uart_descriptor, F_SETFL, FNDELAY) if you want that
    fcntl(uart_descriptor, F_SETFL, 0);

    //----- TX BYTES -----
    unsigned char buffer[2];
    unsigned char *p_buffer;
    
    p_buffer = &buffer[0];
    *p_buffer++ = 0xA0;
    if (state)
    {
        *p_buffer++ = 0x01;
    }
    else
    {
        *p_buffer++ = 0x00; 
    }

    // Write to the port
    int n = write(uart_descriptor, &buffer[0], (p_buffer - &buffer[0]));
    if (n < 0)
    {
        close(uart_descriptor);
        Log("Erro ao escrever na uart");
        return false;
    }

    sleep(2);

    char buffer_result;

    int m = 0;
    // int m = read(uart_descriptor, (void*) &buffer_result, 1);

    bool result;
    if (m <= 0)
    {
        Log("Erro ao ler a resposta do ar condicionado");
        result = false;
    }

    if (buffer_result != 0xA1)
    {
        Log("O arduino recebeu um comando desconhecido");
        result = false;
    }
    else
    {
        result = true;
    }

    close(uart_descriptor);
    return result;
}
