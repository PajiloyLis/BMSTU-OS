#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define WRITERS_CNT 5
#define READERS_CNT 4

//semaphores
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


struct sembuf stop_write[] = {{BINARY_WRITER, 1, 0},
                              {ACTIVE_WRITERS, -1, 0}};

int *shared_number;

int flag = 1;

void signal_handler(int sig_numb) {
    flag = 0;
    printf("pid %d received signal %d\n", getpid(), sig_numb);
}

void writer(int semid) {
    srand(getpid());
    while (flag) {
        sleep(rand() % 3);
        if (semop(semid, start_write, sizeof(start_write) / sizeof(start_write[0])) == -1) {
            perror("Semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
        (*shared_number)++;
        printf("Writer %d write %d\n", getpid(), *shared_number);
        if (semop(semid, stop_write, sizeof(stop_write) / sizeof(stop_write[0])) == -1) {
            perror("Semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
    }
    exit(0);
}

void reader(int semid) {
    srand(getpid());
    while (flag) {
        sleep(rand() % 2);
        if (semop(semid, start_read, sizeof(start_read) / sizeof(start_read[0])) == -1) {
            perror("Semop");
            printf("pid = %d errno = %d\n", getpid(), errno);
            exit(1);
        }
        printf("Reader %d read %d\n", getpid(), *shared_number);
        if (semop(semid, stop_read, sizeof(stop_read) / sizeof(stop_read[0])) == -1) {
            perror("Semop");
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
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    key_t sem_key = ftok(argv[0], 1);
    if (sem_key == -1) {
        perror("Error: create semaphore set key");
        exit(1);
    }
    int semid;
    if ((semid = semget(sem_key, 5, IPC_CREAT | perms)) == -1) {
        perror("Error: create semaphore set");
        exit(1);
    }
    key_t shm_key = ftok(argv[0], 1);	
    if (shm_key == -1) {
        perror("Error: create shared memory key");
        exit(1);
    }
    int shmid;
    if ((shmid = shmget(shm_key, sizeof(int), IPC_CREAT | perms)) == -1) {
        perror("Error: create shared memory");
        exit(1);
    }
    shared_number = shmat(shmid, NULL, 0);
    if (shared_number == (int *) -1) {
        perror("Error: attach shared memory");
        exit(1);
    }
    *shared_number = 0;

//  monitor initialization
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

    pid_t chpid[WRITERS_CNT + READERS_CNT];
    for (int i = 0; i < WRITERS_CNT; ++i) {
        if ((chpid[i] = fork()) == -1) {
            perror("Error: fork");
            exit(1);
        }
        if (chpid[i] == 0)
            writer(semid);
    }
    for (int i = WRITERS_CNT; i < WRITERS_CNT + READERS_CNT; ++i) {
        if ((chpid[i] = fork()) == -1) {
            perror("Error: fork");
            exit(1);
        }
        if (chpid[i] == 0)
            reader(semid);
    }
    struct shmid_ds stat;
    shmctl(shmid, IPC_STAT, &stat);
    printf("connections: %lu\n", stat.shm_nattch);
    int children_running = READERS_CNT + WRITERS_CNT;
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

    shmctl(shmid, IPC_STAT, &stat);
    printf("connections: %lu\n", stat.shm_nattch);
    if (shmdt((void *) shared_number) == -1) {
        perror("Error: shared memory detach error");
        exit(1);
    }

    shmctl(shmid, IPC_STAT, &stat);
    printf("connections: %lu\n", stat.shm_nattch);
    if (semctl(semid, ACTIVE_READERS, IPC_RMID, NULL) == -1) {
        perror("Error: semaphore set delete error");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shared memory delete error\n");
        exit(1);
    }
}
