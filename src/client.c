#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_TEMP 8080
#define PORT_KEY 3000

void quit();
void menu();
void *monitoring_temperature();
float get_temperature();
void print_temperature(float);
bool air_conditioning(int);

volatile float temperature = 0;
static pthread_mutex_t mutex_lock;
char *ip = "127.0.0.1";
int socket_temp_d = 0;
int socket_key_d = 0;

int main(int argc, char *argv[])
{
    signal(SIGINT, quit);

    if (argc == 2) ip = argv[1];
    else if (argc > 2) errx(1, "Invalid argument");

    if (pthread_mutex_init(&mutex_lock, NULL))
        errx(1, "Error creating mutex");

    pthread_t temperature_thread;
    if (pthread_create(&temperature_thread, NULL, monitoring_temperature, NULL))
        errx(1, "Error creating thread");

    system("clear");

    menu();
    printf("-> ");

    while (1)
    {
        int option;
        scanf("%d", &option);

        switch (option)
        {
            case 0:
                system("clear");
                menu();
                break;

            case 1:
                if (air_conditioning(option))
                    printf("Ar condicionado foi ligado.\n");
                else
                    printf("Erro ao ligar o ar condicionado.\n");
                break;
            case 2:
                if (air_conditioning(option))
                    printf("Ar condicionado foi desligado.\n");
                else
                    printf("Erro ao desligar o ar condicionado.");
                break;
            case 3:
                quit();
                break;
            default:
                printf("Opcao invalida\n");
        }

        printf("-> ");
    }

    pthread_mutex_destroy(&mutex_lock);

    return 0;
}

void quit()
{
    // close()
    printf("\nBye\n");
    if (socket_temp_d) close(socket_temp_d);
    if (socket_key_d) close(socket_key_d);
    pthread_mutex_destroy(&mutex_lock);
    exit(0);
}

void menu()
{
    printf("Controle de ar condicionado");
    printf("\n\nEscolha uma opcao:\n");
    printf("(1) Ligar\n");
    printf("(2) Desligar\n");
    printf("(3) Sair\n");
}

void *monitoring_temperature()
{
    while (1)
    {
        socket_temp_d = setup(PORT_TEMP);

        float temp = get_temperature();
        pthread_mutex_lock(&mutex_lock);
        temperature = temp;
        pthread_mutex_unlock(&mutex_lock);

        print_temperature(temperature);
        sleep(2);
    }
}

int setup(int port)
{
    int socket_id;
    struct sockaddr_in addr_client;

    // socket()
    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        errx(1, "Error creating socket");

    // connect()
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(port);
    addr_client.sin_addr.s_addr = inet_addr(ip);
    bzero(&(addr_client.sin_zero), 8);
    if (connect(socket_id, (struct sockaddr *) &addr_client, sizeof(struct sockaddr)) == -1)
    {
        close(socket_id);
        errx(1, "Error connecting server");
    }

    return socket_id;
}

float get_temperature()
{
    // write()
    char *msg = "get_temperature";
    int length = strlen(msg) + 1;
    if (write(socket_temp_d, &length, sizeof(length)) == -1)
    {
        close(socket_temp_d);
        errx(1, "Error sending message to server");
    }

    if (write(socket_temp_d, msg, length) == -1)
    {
        close(socket_temp_d);
        errx(1, "Error sending message to server");
    }

    float temp;
    if (recv(socket_temp_d, &temp, sizeof(temp), 0) == -1)
    {
        close(socket_temp_d);
        errx(1, "Error receiving message");
    }

    return temp;
}

void print_temperature(float temperature)
{
    printf("\033[%d;%dfTemperatura: %.1f C\n", 3, 40, temperature);
    printf("\033[%d;%df\n", 8, 8);
    fflush(stdout);
}

bool air_conditioning(int option)
{
    bool status = option % 2;
    socket_key_d = setup(PORT_KEY);
    char msg[4];

    status ? strcpy(msg, "on") : strcpy(msg, "off");

    int length = strlen(msg) + 1;

    if (send(socket_key_d, &length, sizeof(length), 0) == -1)
    {
        close(socket_key_d);
        errx(1, "Error sending message to server");
    }

    if (send(socket_key_d, msg, length, 0) == -1)
    {
        close(socket_key_d);
        errx(1, "Error sending message to server");
    }

    bool result;
    if (recv(socket_key_d, &result, sizeof(result), 0) == -1)
    {
        close(socket_key_d);
        errx(1, "Error receiving message");
    }

    return result;
}
