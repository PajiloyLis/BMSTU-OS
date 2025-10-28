#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

void* reader_thread(void* arg) {
    FILE* fs = (FILE*)arg;
    int flag = 1;
    char c;
    while (flag > 0) {
        flag = fscanf(fs, "%c", &c);
        if (flag > 0)
        {
            fprintf(stdout,"%c",c);
        }
    }
    return NULL;
}
int main()
{
    int fd = open("alphabet.txt",O_RDONLY);
    FILE *fs1 = fdopen(fd,"r");
    char buff1[20];
    setvbuf(fs1,buff1,_IOFBF,20);
    FILE *fs2 = fdopen(fd,"r");
    char buff2[20];
    setvbuf(fs2,buff2,_IOFBF,20);
    pthread_t thread;
    if (pthread_create(&thread, NULL, reader_thread, (void*)fs2) != 0) {
        perror("Failed to create thread");
        fclose(fs1);
        fclose(fs2);
        close(fd);
        return 1;
    }
    int flag = 1;
    char c;
    while (flag > 0) {
        flag = fscanf(fs1, "%c", &c);
        if (flag > 0) {
            fprintf(stdout,"%c",c);
        }
    }
    pthread_join(thread, NULL);
    fclose(fs1);
    fclose(fs2);
    close(fd);
    return 0;
}