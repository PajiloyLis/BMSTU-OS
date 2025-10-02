#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/syscall.h>

#define SIZE 27
#define PORT 9878
#define REQUEST "R"
char array[SIZE];

#define ACTIVE_READERS 0
#define ACTIVE_WRITERS 1
#define WRITE_QUEUE 2
#define READ_QUEUE 3
#define BINARY_WRITER 4

struct sembuf start_read[] = {
        {READ_QUEUE,     1,  0},
        {ACTIVE_WRITERS, 0,  0},
        {WRITE_QUEUE,    0,  0},
        {ACTIVE_READERS, 1,  0},
        {READ_QUEUE,     -1, 0}
};

struct sembuf stop_read[] = {{ACTIVE_READERS, -1, 0}};

struct sembuf start_write[] = {
        {WRITE_QUEUE,    1,  0},
        {ACTIVE_READERS, 0,  0},
        {BINARY_WRITER,  -1, 0},
        {ACTIVE_WRITERS, 0,  0},
        {ACTIVE_WRITERS, 1,  0},
        {WRITE_QUEUE,    -1, 0}
};

struct sembuf stop_write[] = {{BINARY_WRITER,  1,  0},
                              {ACTIVE_WRITERS, -1, 0}};

void writer(int connfd, int semid, char *array, char letter) {
    if (semop(semid, start_write, sizeof(start_write) / sizeof(start_write[0])) == -1) {
        perror("Error: semop");
        exit(EXIT_FAILURE);
    }
    char *symbol = strchr(array, letter);
    if (symbol != NULL && *symbol == letter) {
        *symbol = '_';

        if (send(connfd, array, SIZE, 0) == -1) {
            perror("Error: send");
            exit(EXIT_FAILURE);
        }
    } else {
        if (send(connfd, "occupied\0", 9, 0) == -1) {
            perror("Error: send");
            exit(EXIT_FAILURE);
        }
    }

    if (semop(semid, stop_write, sizeof(stop_write) / sizeof(stop_write[0])) == -1) {
        perror("Error: semop");
        exit(EXIT_FAILURE);
    }
}

void reader(int connfd, int semid, char *array) {
    if (semop(semid, start_read, sizeof(start_read) / sizeof(start_read[0])) == -1) {
        perror("Error: semop");
        exit(EXIT_FAILURE);
    }
    if (send(connfd, array, SIZE, 0) == -1) {
        perror("Error: send");
        exit(EXIT_FAILURE);
    }
    if (semop(semid, stop_read, sizeof(stop_read) / sizeof(stop_read[0])) == -1) {
        perror("Error: semop");
        exit(EXIT_FAILURE);
    }
}

struct serve_args {
    int sockfd, semid;
    clock_t start;
};

void *serve_client(void *args) {
    char filename[64];
    sprintf(filename, "serv_thread_%zu.txt", syscall(SYS_gettid));
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Error: fopen");
        exit(EXIT_FAILURE);
    }
//    printf("Thread %zu, running on %d core with shed_getcpu(), kernel thread %ld\n", (size_t) pthread_self(),
//           sched_getcpu(), syscall(SYS_gettid));
//    unsigned cpu_num;
//    syscall(SYS_getcpu, &cpu_num, NULL, NULL);
//    printf("Thread %zu, running on %d core with syscall\n", (size_t)pthread_self(), cpu_num);

    struct serve_args *params = (struct serve_args *) args;
    int sockfd = params->sockfd, semid = params->semid;
    while (1) {
        char buf[SIZE + 1] = {0};
        ssize_t bytes_read = read(sockfd, buf, SIZE);
        if (bytes_read <= 0) {
            break;
        }

        if (strcmp(buf, REQUEST) == 0) {
            reader(sockfd, semid, array);
        } else if (strlen(buf) == 1) {
//            printf("Received: %c\n", buf[0]);
            writer(sockfd, semid, array, buf[0]);
            if(buf[0] == 'z')
            {
                for(int i =0; i < SIZE-1; ++i)
                {
                    array[i]=(char)('a'+i);
                }
                array[SIZE-1] = 0;
            }
        }
    }
//    close(sockfd);
    clock_t end = clock();
    fprintf(f, "%ld\n", end - params->start);
    fclose(f);
    pthread_exit(NULL);
}


int main() {
    int lisnfd, connfd;
    struct sockaddr_in address;
    int address_size = sizeof(address);
    for (int i = 0; i < SIZE - 1; ++i) {
        array[i] = (char) ('a' + i);
    }
    array[SIZE - 1] = 0;
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int semid;
    if ((semid = semget(IPC_PRIVATE, 5, IPC_CREAT | perms)) == -1) {
        perror("Error: semget");
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, ACTIVE_READERS, SETVAL, 0) == -1) {
        perror("Error: set value for active reader semaphore");
        exit(1);
    }
    if (semctl(semid, ACTIVE_WRITERS, SETVAL, 0) == -1) {
        perror("Error: set value for active writers semaphore");
        exit(1);
    }
    if (semctl(semid, READ_QUEUE, SETVAL, 0) == -1) {
        perror("Error: set value for read queue semaphore");
        exit(1);
    }
    if (semctl(semid, WRITE_QUEUE, SETVAL, 0) == -1) {
        perror("Error: set value for write queue semaphore");
        exit(1);
    }
    if (semctl(semid, BINARY_WRITER, SETVAL, 1) == -1) {
        perror("Error: set value for writer binary semaphore");
        exit(1);
    }

    if ((lisnfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error: socket");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(lisnfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("Error: bind");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    if (listen(lisnfd, 5) == -1) {
        perror("Error: listen");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        if ((connfd = accept(lisnfd, (struct sockaddr *) &address, (socklen_t *) &address_size)) == -1) {
            perror("Error: accept");
            close(lisnfd);
            exit(EXIT_FAILURE);
        }
        pthread_t thread;
        clock_t start = clock();
        struct serve_args serve_args = {connfd, semid, start};
        if (pthread_create(&thread, &attr, serve_client, &serve_args) != 0) {
            perror("Error: thread create");
            close(connfd);
        }
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Error: semctl");
        exit(EXIT_FAILURE);
    }
    return 0;
}
