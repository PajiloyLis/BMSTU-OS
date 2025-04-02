#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHILD_COUNT 3

int main(void) {
    pid_t child_pid[CHILD_COUNT];

    for (int i = 0; i < CHILD_COUNT; i++) {
        if ((child_pid[i] = fork()) == -1) {
            perror("Can't fork\n");
            exit(1);
        } else if (child_pid[i] == 0) {
            printf("Before sleep: child_%d: pid = %d, ppid = %d, group = %d\n", \
                i + 1, getpid(), getppid(), getpgrp());
            sleep(2);
            printf("After sleep: child_%d: pid = %d, ppid = %d, group = %d\n", \
                i + 1, getpid(), getppid(), getpgrp());
            return 0;
        } else {
            printf("Parent: pid = %d, ppid = %d, group = %d\n", getpid(), getppid(), getpgrp());
        }
    }

    return 0;
}