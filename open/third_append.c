#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

void file_info(int fd, char *path) {
    struct stat statbuff;
    if (lstat(path, &statbuff) == -1)
        exit(EXIT_FAILURE);
    stat("result", &statbuff);
    printf("inode: %ld ", statbuff.st_ino);
    printf("size: %ld ", statbuff.st_size);
    printf("pos: %ld \n", lseek(fd, 0, SEEK_CUR));
}

int main() {
    int fd1 = open("res", O_WRONLY | O_CREAT | O_APPEND, 0666);
    file_info(fd1, "res");
    int fd2 = open("res", O_WRONLY | O_APPEND);
    file_info(fd2, "res");
    for (char c = 'a'; c <= 'z'; c++) {
        if (c % 2) {
            write(fd1, &c, sizeof(char));
        } else {
            write(fd2, &c, sizeof(char));
        }
    }
    close(fd1);
    file_info(fd1, "res");
    close(fd2);
    file_info(fd2, "res");
    return 0;
}
