#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define CONSUMERS_CNT 4
#define PRODUCERS_CNT 8

#define SEMAPHORE_CNT 3
//semaphore nums
#define BINARY_SEM 0
#define BUFFER_EMPTY 1
#define BUFFER_FULL 2

int flag = 1;

void signal_handler(int sig_numb) {
    flag = 0;
    printf("pid %d received signal %d\n", getpid(), sig_numb);
}

struct sembuf start_produce[2] = {{BUFFER_EMPTY, -1, 0},
                                  {BINARY_SEM,   -1, 0}};
struct sembuf stop_produce[2] = {{BINARY_SEM,  1, 0},
                                 {BUFFER_FULL, 1, 0}};
struct sembuf start_consume[2] = {{BUFFER_FULL, -1, 0},
                                  {BINARY_SEM,  -1, 0}};
struct sembuf stop_consume[2] = {{BINARY_SEM,   1, 0},
                                 {BUFFER_EMPTY, 1, 0}};

void producer(const int semid, char *addr) {
    srand(getpid());
    char **prod_ptr = (char **) addr;
    char *alpha_ptr = (char *) (prod_ptr + 2);

    while (flag) {
        if (semop(semid, start_produce, sizeof(start_produce) / sizeof(start_produce[0])) == -1) {
            perror("semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
        printf("Producer %d produce %c\n", getpid(), *alpha_ptr);
        **prod_ptr = *alpha_ptr;
        if (*alpha_ptr == 'z') {
            *alpha_ptr = 'a';
        } else {
            (*alpha_ptr)++;
        }
        (*prod_ptr)++;
        if (semop(semid, stop_produce, sizeof(stop_produce) / sizeof(stop_produce[0])) == -1) {
            perror("semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
    }
    exit(0);
}

void consumer(const int semid, char *addr) {
    srand(getpid());
    char **cons_ptr = (char **) (addr) + 1;
    while (flag) {
        if (semop(semid, start_consume, sizeof(start_consume) / sizeof(start_consume[0])) == -1) {
            perror("semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
        printf("Consumer %d consume %c\n", getpid(), **cons_ptr);

        (*cons_ptr)++;


        if (semop(semid, stop_consume, sizeof(stop_consume) / sizeof(stop_consume[0])) == -1) {
            perror("semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
    }
    exit(0);
}

int main(int argc, char **argv) {

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, 0) == -1) {
        perror("Error: change sigterm handler error");
        exit(1);
    }

    int shmid, semid, perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char **prod_ptr = NULL;
    char **cons_ptr;
    char *alpha_ptr;
    key_t shm_key = ftok(argv[0], 1);
    if (shm_key == -1) {
        perror("Error: shared memory key generate error");
        exit(1);
    }
    if ((shmid = shmget(shm_key, BUFFER_SIZE, IPC_CREAT | perms)) == -1) {
        perror("Error: create shared memory error");
        exit(1);
    }
    char *buf = shmat(shmid, NULL, 0);
    if (buf == (char *) -1) {
        perror("Error: shared memory attach error");
        exit(1);
    }
    prod_ptr = (char **) buf;
    cons_ptr = ((char **) buf) + 1;
    alpha_ptr = (char *) (cons_ptr + 1);
    *prod_ptr = alpha_ptr + 1;
    *cons_ptr = *prod_ptr;
    *alpha_ptr = 'a';


    key_t sem_key = ftok(argv[0], 1);
    if (sem_key == -1) {
        perror("Error: semaphore set key generate error");
        exit(1);
    }
    if ((semid = semget(sem_key, SEMAPHORE_CNT, IPC_CREAT | perms)) == -1) {
        perror("Error: create semaphore set error");
        exit(1);
    }

    if (semctl(semid, BINARY_SEM, SETVAL, 1) == -1) {
        perror("Error: set binary semaphore error");
        exit(1);
    }
    if (semctl(semid, BUFFER_FULL, SETVAL, 0) == -1) {
        perror("Error: set buffer full semaphore error");
        exit(1);
    }
    if (semctl(semid, BUFFER_EMPTY, SETVAL, BUFFER_SIZE) == -1) {
        perror("Error: set buffer empty semaphore error");
        exit(1);
    }

    pid_t ch_pid[CONSUMERS_CNT + PRODUCERS_CNT];
    for (int i = 0; i < PRODUCERS_CNT; ++i) {
        if ((ch_pid[i] = fork()) == -1) {
            perror("Error: fork error");
            exit(1);
        }
        if (ch_pid[i] == 0) {
            producer(semid, buf);
        }
    }
    for (int i = PRODUCERS_CNT; i < sizeof(ch_pid) / sizeof(ch_pid[0]); ++i) {
        if ((ch_pid[i] = fork()) == -1) {
            perror("Error: fork error");
            exit(1);
        }
        if (ch_pid[i] == 0) {
            consumer(semid, buf);
        }
    }

    int children_running = PRODUCERS_CNT + CONSUMERS_CNT;
    int child_status;
    while (children_running) {
        pid_t rc = waitpid(-1, &child_status,
                           WNOHANG); // ask about state change of child's processes
        if (rc == -1)
            perror("Wait failure");
        else if (rc == 0)
            continue;
        if (WIFEXITED(child_status)) {
            --children_running;
            printf("Child: pid = %d exited with code %d\n", rc, WEXITSTATUS(child_status));
        } else if (WIFSTOPPED(child_status)) {
            printf("Child: pid %d stopped by signal %d\n", rc, WSTOPSIG(child_status));
        } else if (WIFCONTINUED(child_status)) {
            printf("Child: pid %d continued\n", rc);
        }
    }
    if (shmdt((void *) buf) == -1) {
        perror("Error: shared memory detach error");
        exit(1);
    }

    if (semctl(semid, BINARY_SEM, IPC_RMID, NULL) == -1) {
        perror("Error: semaphore set delete error");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shared memory delete error\n");
        exit(1);
    }
    return 0;
}
