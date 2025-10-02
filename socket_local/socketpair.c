#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define MSG1 "aaa"
#define MSG2 "bcbcbc"
#define MSG3 "def"
#define MSG4 "nnnn"

#define BUF_SIZE 1024

int main() {
    int sockfd[2];
    char buf[BUF_SIZE];
    pid_t pid;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1) {
        perror("Error socketpair");
        exit(-1);
    }

    if ((pid = fork()) == -1) {
        perror("Error fork");
        exit(-1);
    }

    if (pid != 0) {
        close(sockfd[1]);
        if (write(sockfd[0], MSG1, strlen(MSG1) + 1) == -1) {
            perror("Error write msg1");
            exit(-1);
        } else {
            printf("Parent sent: %s\n", MSG1);
        }

        ssize_t bytes_read_cnt = read(sockfd[0], buf, sizeof(buf));
        if (bytes_read_cnt == -1) {
            perror("Error read msg2");
            exit(-1);
        } else {
            buf[bytes_read_cnt] = '\0';
            printf("Parent received: %s\n", buf);
        }

        if (write(sockfd[0], MSG3, strlen(MSG3) + 1) == -1) {
            perror("Error write msg3");
            exit(-1);
        } else {
            printf("Parent sent: %s\n", MSG3);
        }

        bytes_read_cnt = read(sockfd[0], buf, sizeof(buf));
        if (bytes_read_cnt == -1) {
            perror("Error read msg4");
            exit(-1);
        } else {
            buf[bytes_read_cnt] = '\0';
            printf("Parent received: %s\n", buf);
        }

        close(sockfd[0]);
    } else {
        close(sockfd[0]);
        ssize_t bytes_read_cnt = read(sockfd[1], buf, sizeof(buf));
        if (bytes_read_cnt == -1) {
            perror("Error read msg1");
            exit(-1);
        } else {
            buf[bytes_read_cnt] = '\0';
            printf("Child received: %s\n", buf);
        }

        if (write(sockfd[1], MSG2, strlen(MSG2) + 1) == -1) {
            perror("Error write msg2");
            exit(-1);
        } else {
            printf("Child sent: %s\n", MSG2);
        }

        bytes_read_cnt = read(sockfd[1], buf, sizeof(buf));
        if (bytes_read_cnt == -1) {
            perror("Error read msg3");
            exit(-1);
        } else {
            buf[bytes_read_cnt] = '\0';
            printf("Child received: %s\n", buf);
        }

        if (write(sockfd[1], MSG4, strlen(MSG4) + 1) == -1) {
            perror("Error write msg4");
            exit(-1);
        } else {
            printf("Child sent: %s\n", MSG4);
        }

        close(sockfd[1]);
    }
}
