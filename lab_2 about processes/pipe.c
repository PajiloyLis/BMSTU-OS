#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define CHILD_COUNT 2

int main(void) {
    pid_t child_pid[CHILD_COUNT];

    char *strings[CHILD_COUNT] = {"aaaa\n", "bbbcbbbcbbbcbbbcbbbc\n"};
    int pipe_file_desc[2];
    if (pipe(pipe_file_desc) == -1) // creation of pipe
    {
        perror("Error: pipe error");
        return 1;
    }

    for (int i = 0; i < CHILD_COUNT; i++) {
        if ((child_pid[i] = fork()) == -1) {
            perror("Error: can't fork\n");
            exit(1);
        } else if (child_pid[i] == 0) {
            close(pipe_file_desc[0]); // close read end;
            write(pipe_file_desc[1], strings[i], strlen(strings[i])); // write string in pipe
            close(pipe_file_desc[1]); // close write end when writing ended
            printf("%d Sent message: %s", getpid(), strings[i]);
            return 0;
        }
    }
    int children_running = CHILD_COUNT;
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
            printf("%d exited with code %d\n", rc, WEXITSTATUS(child_status));
        } else if (WIFSIGNALED(child_status)) {
            --children_running;
            printf("%d terminated by signal %d\n", rc, WTERMSIG(child_status));
#ifdef WCOREDUMP
            if (WCOREDUMP(child_status))
                printf("%d made core dump\n", rc);
            else
                printf("%d didn't make core dump\n", rc);
#endif
        }
    }
    printf("Received messages:\n");
    char buf;
    close(pipe_file_desc[1]); // close write end
    int rc;
    while ((rc = read(pipe_file_desc[0], &buf, 1)) == 1) // read string char by char from pipe
        write(STDOUT_FILENO, &buf, 1);
    close(pipe_file_desc[0]); // close read end
    if (rc == -1)
        perror("Error: reading from pipe");
    else if (rc == 0) {
        write(STDOUT_FILENO, "\n", 1);
//        printf("Pipe ended\n");
    }
    return 0;
}
