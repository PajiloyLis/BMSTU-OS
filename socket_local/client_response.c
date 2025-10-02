#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include "request_response.h"

#define size(arr) (sizeof(arr)/sizeof(arr[0]))

int flag = 1;

void sig_handler() {
    flag = 0;
}

int main(void) {
    struct sockaddr server_addr, client_addr;
    srand(time(0));
    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("Error socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, SOCK_NAME);

    client_addr.sa_family = AF_UNIX;
    sprintf(client_addr.sa_data, "%d.sock", getpid());
    unlink(client_addr.sa_data);

    if (bind(sockfd, &client_addr, sizeof(client_addr)) == -1) {
        perror("Error bind");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
    while (flag) {
        const char operations[] = {'+', '-', '*', '/'};
        request_t request = {.first_operand = (double) (rand() % 10) / (rand() % 10 + 1),
                .second_operand = (double) (rand() % 10) / (rand() % 10 + 1),
                .operation = operations[rand() % size(operations)]};
        if (sendto(sockfd, &request, sizeof(request), 0, &server_addr, sizeof(server_addr)) == -1) {
            perror("Sendto error");
            break;
        }
        double result;
        if (recvfrom(sockfd, &result, sizeof(result), 0, NULL, NULL) == -1) {
            perror("Recvfrom error");
            break;
        }
        if (result == INFINITY)
            printf("Error: division by zero\n");
        printf("%.2f %c %.2f = %.2f\n", request.first_operand, request.operation, request.second_operand, result);
        sleep(rand() % 2);
    }
    unlink(client_addr.sa_data);
    close(sockfd);
    return 0;
}