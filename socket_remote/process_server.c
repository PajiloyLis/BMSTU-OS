#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#define SIZE 27
#define PORT 9878
#define REQUEST "R"


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


int main() {
    int lisnfd, connfd;
    struct sockaddr_in address;
    int address_size = sizeof(address);
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | perms);
    if (shmid == -1) {
        perror("Error: shmget");
        exit(EXIT_SUCCESS);
    }
    char *array = (char *) shmat(shmid, NULL, 0);
    if (array == (char *) -1) {
        perror("Error: shmat");
        exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < SIZE - 1; ++i) {
        array[i] = (char)('a' + i);
    }
    array[SIZE - 1] = 0;
    int semid = semget(IPC_PRIVATE, 5, IPC_CREAT | perms);
    if (semid == -1) {
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

    if ((lisnfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error: socket");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    address.sin_family=AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port= htons(PORT);

    if(bind(lisnfd, (struct sockaddr *)&address, sizeof(address))==-1)
    {
        perror("Error: bind");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    if(listen(lisnfd, 5) == -1)
    {
        perror("Error: listen");
        close(lisnfd);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if((connfd = accept(lisnfd, (struct sockaddr*)&address, (socklen_t *)&address_size))== -1)
        {
            perror("Error: accept");
            close(lisnfd);
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("Error: fork");
            close(lisnfd);
            continue;
        }
        else if(pid == 0)
        {
            close(lisnfd);

            while(1)
            {
                char buf[SIZE+1] = {0};
                ssize_t bytes_read = read(connfd, buf, SIZE);
                if (bytes_read <= 0)
                {
                    printf("Error: read");
                    break;
                }

                if(strcmp(buf, REQUEST) == 0)
                {
                    reader(connfd, semid, array);
                }
                else if(strlen(buf) == 1)
                {
                    printf("Received: %c\n", buf[0]);
                    writer(connfd, semid, array, buf[0]);
                }
                sleep(rand()%2+1);
            }
            close(connfd);
            break;
        }
        else
        {
            close(connfd);
        }
    }
    if (shmdt((void *) array) == -1) {
        perror("Error: shmdt");
        exit(EXIT_FAILURE);
    }
    if(shmctl(shmid, IPC_RMID, NULL)==-1)
    {
        perror("Error: shmctl");
        exit(EXIT_FAILURE);
    }
    if(semctl(semid, 0, IPC_RMID)==-1)
    {
        perror("Error: semctl");
        exit(EXIT_FAILURE);
    }
    return 0;
}
