#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

void *reader_thread(void *arg) {
    int fd = (int) (long) arg;
    char c;

    while (read(fd, &c, 1) > 0) {
        write(1, &c, 1);
    }

    return NULL;
}

int main() {

    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);

    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, reader_thread, (void *) (long) fd1);
    pthread_create(&thread2, NULL, reader_thread, (void *) (long) fd2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(fd1);
    close(fd2);
    return 0;
}
