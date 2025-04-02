/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "prod_cons.h"
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "time.h"
#include <pthread.h>

char letter = 'a';
char letters[BUFFER_SIZE];
char *prod_ptr = letters, *cons_ptr = letters;
int sem_fd;


struct sembuf start_produce[2] = {{BUFFER_EMPTY, -1, 0},
                                  {BINARY_SEM,   -1, 0}};
struct sembuf stop_produce[2] = {{BINARY_SEM,  1, 0},
                                 {BUFFER_FULL, 1, 0}};
struct sembuf start_consume[2] = {{BUFFER_FULL, -1, 0},
                                  {BINARY_SEM,  -1, 0}};
struct sembuf stop_consume[2] = {{BINARY_SEM,   1, 0},
                                 {BUFFER_EMPTY, 1, 0}};

char producer(char *letter, char *letters, char **prod_ptr) {
    char res;
    if (semop(sem_fd, start_produce, sizeof(start_produce) / sizeof(start_produce[0])) == -1) {
        perror("semop");
        pthread_exit(1);
    }
    res = *letter;
    printf("Producer produce %c\n", *letter);
    **prod_ptr = *letter;
    if (*letter == 'z') {
        *letter = 'a';
    } else {
        ++(*letter);
    }
    ++(*prod_ptr);
    if (semop(sem_fd, stop_produce, sizeof(stop_produce) / sizeof(stop_produce[0])) == -1) {
        perror("semop");
        pthread_exit(1);
    }
    return res;
}

char consumer(char *letters, char **cons_ptr) {
    char res;
    if (semop(sem_fd, start_consume, sizeof(start_consume) / sizeof(start_consume[0])) == -1) {
        perror("semop");
        pthread_exit(1);
    }
    res = **cons_ptr;
    printf("Consumer consume %c\n", res);
    ++(*cons_ptr);
    if (semop(sem_fd, stop_consume, sizeof(stop_consume) / sizeof(stop_consume[0])) == -1) {
        perror("semop");
        pthread_exit(1);
    }
    return res;
}

void construct_semaphore_set(char *file_name) {
    key_t sem_key = ftok(file_name, 1);
    if (sem_key == -1) {
        perror("Error: semaphore set key generate error");
        exit(1);
    }
    if ((sem_fd = semget(sem_key, SEMAPHORE_CNT, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
        perror("Error: create semaphore set error");
        exit(1);
    }

    if (semctl(sem_fd, BINARY_SEM, SETVAL, 1) == -1) {
        perror("Error: set binary semaphore error");
        exit(1);
    }
    if (semctl(sem_fd, BUFFER_FULL, SETVAL, 0) == -1) {
        perror("Error: set buffer full semaphore error");
        exit(1);
    }
    if (semctl(sem_fd, BUFFER_EMPTY, SETVAL, BUFFER_SIZE) == -1) {
        perror("Error: set buffer empty semaphore error");
        exit(1);
    }
}

void destruct_semaphore_set() {
    if (semctl(sem_fd, BINARY_SEM, IPC_RMID, NULL) == -1) {
        perror("Error: semaphore set delete error");
        exit(1);
    }
}

bool_t
service_1_svc(int *argp, char *result, struct svc_req *rqstp) {
    bool_t retval;
    if (*argp == PROD)
        *result = producer(&letter, letters, &prod_ptr);
    else
        *result = consumer(letters, &cons_ptr);
    return retval;
}

int
producer_consumer_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result) {
    xdr_free(xdr_result, result);
    return 1;
}
