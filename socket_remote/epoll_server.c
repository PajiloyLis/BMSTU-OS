#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>

#define SIZE 27
#define PORT 9878
#define MAX_EVENTS 50
#define REQUEST "R"

void reader(int conn_fd, char *array) {
    if (send(conn_fd, array, SIZE, 0) == -1) {
        perror("Error: send");
        exit(EXIT_FAILURE);
    }
}

void writer(int conn_fd, char *array, char letter) {
    char *symb = strchr(array, letter);
    if (symb != NULL && *symb == letter) {
        *symb = '_';
        if (send(conn_fd, array, SIZE, 0) == -1) {
            perror("Error: send");
            exit(EXIT_FAILURE);
        }
    } else {
        if (send(conn_fd, "occupied\0", 9, 0) == -1) {
            perror("Error: send");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    int lisnfd, connfd, epollfd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    clock_t start, end;
    struct epoll_event ev, events[MAX_EVENTS];
    char array[SIZE];
    for (int i = 0; i < SIZE - 1; i++) {
        array[i] = 'a' + i;
    }
    array[SIZE - 1] = '\0';

    if ((lisnfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error:socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    if (bind(lisnfd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Error: bind");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    if (listen(lisnfd, 5) == -1) {
        perror("Error: listen");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("Error: epoll_create1");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = lisnfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lisnfd, &ev) == -1) {
        perror("Error: epoll_ctl");
        close(lisnfd);
        close(epollfd);
        exit(EXIT_FAILURE);
    }
    while (1) {
        FILE *f = fopen("epoll_server_times.txt", "a");
        if(!f)
        {
            perror("Error: fopen");
            return EXIT_FAILURE;
        }
        start = clock();
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Error: epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == lisnfd) {
                connfd = accept(lisnfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (connfd == -1) {
                    perror("Error: accept");
                    exit(EXIT_FAILURE);
                }

                int flags = fcntl(connfd, F_GETFL, 0);
                fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    perror("Error: epoll_ctl");
                    close(connfd);
                    exit(EXIT_FAILURE);
                }
            } else {
                connfd = events[n].data.fd;
                char buffer[SIZE + 1] = {0};
                ssize_t bytes_read = read(connfd, buffer, SIZE);
                if (bytes_read <= 0) {
                    close(connfd);
                } else {
                    if (strcmp(buffer, REQUEST) == 0) {
                        reader(connfd, array);
                    } else if (strlen(buffer) == 1) {
                        writer(connfd, array, buffer[0]);
                        if(buffer[0] == 'z')
                        {
                            for(int i =0; i < SIZE-1; ++i)
                            {
                                array[i]=(char)('a'+i);
                            }
                            array[SIZE-1] = 0;
                        }
                    }
                }
            }
        }
        end = clock();
//        printf("%ld\n", end - start);
        fprintf(f, "%ld\n", end - start);
        fclose(f);
    }
    close(lisnfd);
    close(epollfd);

    return 0;
}