#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SIZE 27
#define PORT 9878
#define IP_SERVER "127.0.01"
#define REQUEST "R"

int main() {
    srand(time(0));
    int clntfd;
    struct sockaddr_in serv_addr;
    char array[SIZE + 1] = {0};

    if ((clntfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error: socket");
        close(clntfd);
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(IP_SERVER);
    if (connect(clntfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("Error: connect");
        close(clntfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (send(clntfd, REQUEST, strlen(REQUEST), 0) == -1) {
            perror("Error: send");
            close(clntfd);
            exit(EXIT_FAILURE);
        }
        if (read(clntfd, array, SIZE) == -1) {
            perror("Error: read");
            close(clntfd);
            exit(EXIT_FAILURE);
        }
        array[SIZE] = 0;

        printf("Array read: %s\n", array);

        char symbols[SIZE] = {0};

        int cnt = 0;

        for (int i = 0; i < SIZE; ++i) {
            if (array[i] != '_') {
                symbols[cnt] = array[i];
                cnt++;
            }
        }

        symbols[cnt] = 0;
        cnt--;

        if (cnt <= 0) {
            printf("No free letters\n");
            close(clntfd);
            break;
        }

        char symbol = symbols[0];
        printf("Get symbol: %c\n", symbol);

        if (send(clntfd, &symbol, 1, 0) == -1) {
            perror("Error send");
            close(clntfd);
            exit(EXIT_FAILURE);
        }
        if (read(clntfd, array, SIZE) == -1) {
            perror("Error: read");
            close(clntfd);
            exit(EXIT_FAILURE);
        }
        if (strcmp(array, "occupied\0") == 0)
            printf("Symbol %c is occupied\n", symbol);
        else
            printf("Array changed: %s\n\n", array);
        sleep(rand() % 2 + 1);
    }
    close(clntfd);
}