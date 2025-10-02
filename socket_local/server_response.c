#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "request_response.h"
#include <signal.h>

int flag = 1;

void sig_handler() {
    flag = 0;
}

int main() {
    struct sockaddr server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    request_t request;

    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("Error socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, SOCK_NAME);

    if (bind(sockfd, &server_addr, sizeof(server_addr)) == -1) {
        perror("Error bind");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    while (flag)
    {
        if (recvfrom(sockfd, &request, sizeof(request_t), 0, &client_addr, &client_addr_size)==-1)
        {
            perror("Error recvfrom");
            break;
        }
        double res;
        switch(request.operation)
        {
            case '+':
                res = request.first_operand+request.second_operand;
                break;
            case '-':
                res = request.first_operand-request.second_operand;
                break;
            case '*':
                res = request.first_operand*request.second_operand;
                break;
            case '/':
                if(fabs(request.second_operand) < 1e-5)
                    res = INFINITY;
                else
                    res = request.first_operand/request.second_operand;
        }
        if(sendto(sockfd, &res, sizeof(res), 0, &client_addr, client_addr_size)==-1)
        {
            perror("Error sendto");
            break;
        }
    }
    unlink(server_addr.sa_data);
    close(sockfd);
}