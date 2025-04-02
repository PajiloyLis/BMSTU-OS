#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD_COUNT 2

int main(void) {
    pid_t child_pid[CHILD_COUNT];
    char *programs[] = {"./program_1/prog_1", "./program_2/prog_2"};
    char *args_1[] = {"./program_1/prog_1", 0};
    char *args_2[] = {"./program_2/prog_2", 0};
    char **args[] = {args_1, args_2};
    for (int i = 0; i < CHILD_COUNT; i++) {
        if ((child_pid[i] = fork()) == -1) {
            perror("Error: can't fork\n");
            exit(1);
        } else if (child_pid[i] == 0) {
            if (execv(programs[i], args[i]) == -1) {
                perror("Error: can't exec");
                return 1;
            }
            return 0;
        }
    }
    int children_running = CHILD_COUNT;
    int child_status;
    while (children_running) {
        pid_t rc = waitpid(-1, &child_status,
                           WNOHANG); // ask about state change of i child process
        if (rc == -1)
            perror("Wait failure");
        else if (rc == 0)
            continue;
        if (WIFEXITED(child_status)) {
            --children_running;
            printf("Child: pid = %d exited with code %d\n", rc, WEXITSTATUS(child_status));
        } else if (WIFSIGNALED(child_status)) {
            --children_running;
            printf("Child: pid %d terminated by signal %d\n", rc, WTERMSIG(child_status));
#ifdef WCOREDUMP
            if (WCOREDUMP(child_status))
                printf("Child: pid = %d made core dump\n", rc);
            else
                printf("Child: pid = %d didn't make core dump\n", rc);
#endif
        }
    }
    return 0;
}
