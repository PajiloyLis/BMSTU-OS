#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCK_NAME "server.socket"
#define BUF_SIZE 128

int main() {
    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("Error socket");
        exit(-1);
    }

    struct sockaddr server_addr = {.sa_family=AF_UNIX};
    strcpy(server_addr.sa_data, SOCK_NAME);

    char buf[BUF_SIZE];
    sprintf(buf, "%d", getpid());
    if (sendto(sockfd, buf, strlen(buf) + 1, 0, &server_addr, sizeof(server_addr)) == -1) {
        perror("Error sendto");
        exit(-1);
    }
    else
    {
        printf("Client sent: %s\n", buf);
    }
    close(sockfd);
}