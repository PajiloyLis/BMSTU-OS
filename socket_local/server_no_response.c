#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCK_NAME "server.socket"
#define BUF_SIZE 128
#define WORK_TIME 10

int main() {
    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }
    struct sockaddr server_addr = {.sa_family = AF_UNIX};
    strcpy(server_addr.sa_data, SOCK_NAME);

    if (bind(sockfd, &server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(-1);
    }

    char buf[128];

    alarm(WORK_TIME);
    ssize_t bytes_read_cnt = recvfrom(sockfd, buf, BUF_SIZE, 0, NULL, NULL);
    if (bytes_read_cnt == -1) {
        perror("Error reading");
        exit(-1);
    } else {
        buf[bytes_read_cnt] = '\0';
        printf("Server received: %s\n", buf);
    }
    unlink(SOCK_NAME);
    return 0;
}