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

#include "screen.c"

#define PORT_TEMP 8080
#define PORT_KEY 3000

void quit();
void *monitoring_temperature();
float get_temperature();
bool air_conditioning(bool status);

static pthread_mutex_t mutex_lock;
volatile float temperature = 0;
char *ip = "127.0.0.1";
int temp_d = 0;
int key_d = 0;

int main(int argc, char *argv[])
{
    signal(SIGINT, quit);

    if (argc == 2) ip = argv[1];
    else if (argc > 2) errx(1, "Invalid argument");

    pthread_t temp_thread;
    if (pthread_mutex_init(&mutex_lock, NULL))
        errx(1, "Error creating mutex");

    if (pthread_create(&temp_thread, NULL, monitoring_temperature, NULL))
        errx(1, "Error creating thread");

    system("clear");

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    gotoxy(w.ws_col / 2 - 12, 0);
    printf("Air conditioning: OFF");
    
    printf("(1) Turn on\n");
    printf("(2) Turn off\n");
    printf("(3) - Exit\n");
    printf("\n\nChoose an option: ");

    while(1)
    {
        int option;
        scanf("%d", &option);

        switch(option)
        {
            case 1:
                if (air_conditioning(true))
                {
                    printf("Air conditioning was turned ON. Press ENTER to continue.");
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                    save_position();
                    gotoxy(w.ws_col / 2 + 5, 0);
                    printf("ON ");
                    reset_position();
                }
                else
                    printf("Error turning air conditioning ON. Press ENTER to retry");

                getchar();
                getchar();
                break;
            case 2:
                if (air_conditioning(false))
                {
                    printf("Air conditioning was turned OFF. Press ENTER to continue.");
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                    save_position();
                    gotoxy(w.ws_col / 2 + 5, 0);
                    printf("OFF");
                    reset_position();
                }
                else
                    printf("Error turning air conditioning OFF. Press ENTER to retry");

                getchar();
                getchar();
                break;
            case 3:
                quit();
                break;
            default:
                printf("Invalid option");
                getchar();
                getchar();
        }

        gotoxy(0,8);
        printf("                                                                                       ");
        gotoxy(0,7);
        printf("Choose an option: ");
        gotoxy(3,7);
    }

    pthread_join(temp_thread, NULL);
    pthread_mutex_destroy(&mutex_lock);


    return 0;
}

void quit()
{
    // close()
    printf("\nBye\n");
    if (temp_d) close(temp_d);
    if (key_d) close(key_d);
    pthread_mutex_destroy(&mutex_lock);
    exit(0);
}

void *monitoring_temperature()
{
        while (1)
    {
        temp_d = setup(PORT_TEMP);

        float temp = get_temperature();
        pthread_mutex_lock(&mutex_lock);
        temperature = temp;
        pthread_mutex_unlock(&mutex_lock);

        printf("Temperature: %.2f\n", temperature);
        sleep(2);
    }
}

int setup(int port)
{
    int socket_id;
    struct sockaddr_in addr_client;

    // socket()
    if (socket_id = socket(AF_INET, SOCK_STREAM, 0) == -1)
        errx(1, "Error creating socket");

    // connect()
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(port);
    addr_client.sin_addr.s_addr = htonl(INADDR_ANY);
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
    if (write(temp_d, &length, sizeof(length)) == -1)
    {
        close(temp_d);
        errx(1, "Error sending message to server");
    }

    if (write(temp_d, msg, length) == -1)
    {
        close(temp_d);
        errx(1, "Error sending message to server");
    }

    float temp = 0.0;
    // if (recv(temp_d, &temp, sizeof(temp), 0) == -1)
    // {
    //     close(temp_d);
    //     errx(1, "Error receiving message");
    // }

    return temp;
}

bool air_conditioning(bool status)
{
    key_d = setup(PORT_KEY);
    char *msg;

    status ? strcpy(msg, "on") : strcpy(msg, "off");

    int length = strlen(msg) + 1;

    if (send(key_d, &length, sizeof(length), 0) == -1)
    {
        close(key_d);
        errx(1, "Error sending message to server");
    }

    if (send(key_d, msg, length, 0) == -1)
    {
        close(key_d);
        errx(1, "Error sending message to server");
    }

    bool result;
    if (recv(key_d, &result, sizeof(result), 0) == -1)
    {
        close(key_d);
        errx(1, "Error receiving message");
    }

    return result;
}