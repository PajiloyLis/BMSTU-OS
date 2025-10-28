#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

void file_info(int fd, char *path) {
    struct stat statbuff;
    if(lstat(path, &statbuff)==-1)
        exit(EXIT_FAILURE);
    stat("result", &statbuff);
    printf("inode: %ld ", statbuff.st_ino);
    printf("size: %ld ", statbuff.st_size);
    printf("pos: %ld \n", lseek(fd, 0, SEEK_CUR));
}

int main() {
    FILE *fs1 = fopen("res", "w");
    int fd1 = fileno(fs1);
    file_info(fd1, "res");
    FILE *fs2 = fopen("res", "w");
    int fd2 = fileno(fs2);
    file_info(fd2, "res");
    for (char c = 'a'; c <= 'z'; c++) {
        if (c % 2) {
            fwrite(&c, sizeof(char), 1, fs1);
        } else {
            fwrite(&c, sizeof(char), 1, fs2);
        }
    }
    fclose(fs1);
    file_info(fd1, "res");
    fclose(fs2);
    file_info(fd2, "res");
    return 0;
}
