#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

static int dopath(int depth, char *path, size_t pathlen) {
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret = EXIT_SUCCESS, n;
    if (lstat(path, &statbuf) == -1)
        return EXIT_FAILURE;
    if (S_ISDIR(statbuf.st_mode) == 0) {
//        printf("\n");
        return EXIT_SUCCESS;
    }
//    if(depth)
//        printf("/\n");
    if (chdir(path) == -1) {
        perror("Error: chdir");
        exit(EXIT_FAILURE);
    }
    if ((dp = opendir(".")) == NULL)
        return EXIT_FAILURE;
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
//            for (int i = 0; i < depth - 1; ++i)
//                printf("|   ");
//            if(depth-1>=0)
//                printf("|---");
//            printf("%s", dirp->d_name);
            dopath(depth + 1, dirp->d_name, pathlen);
        }
    }
//    for (int i = 0; i < depth - 1; ++i)
//        printf("|   ");
//    printf("XXX\n");
    if (closedir(dp) < 0) {
        perror("Error: close");
        return EXIT_FAILURE;
    }
    if (chdir("..") == -1) {
        perror("Error: chdir");
        exit(EXIT_FAILURE);
    }
    return (ret);
}

static int dopath_long(int depth, char *path, size_t pathlen) {
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret = EXIT_SUCCESS, n;
    if (lstat(path, &statbuf) == -1)
        return EXIT_FAILURE;
    if (S_ISDIR(statbuf.st_mode) == 0) {
//        printf("\n");
        return EXIT_SUCCESS;
    }
//    if(depth)
//        printf("/\n");
//    if(chdir(path)==-1){
//        perror("Error: chdir");
//        exit(EXIT_FAILURE);
//    }
    n = strlen(path);
    if (n + NAME_MAX + 2 > pathlen) { /* увеличить размер буфера */
        pathlen *= 2;
        if ((path = realloc(path, pathlen)) == NULL)
        {
            perror("Error: realloc");
            return EXIT_FAILURE;
        }
    }
    path[n++] = '/';
    path[n] = 0;
    if ((dp = opendir(path)) == NULL)
        return EXIT_FAILURE;
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, "..")) {
//            for (int i = 0; i < depth - 1; ++i)
//                printf("|   ");
//            if(depth-1>=0)
//                printf("|---");
//            printf("%s", dirp->d_name);
            strcpy(&path[n], dirp->d_name);
            dopath_long(depth + 1, path, pathlen);
        }
    }
//    for (int i = 0; i < depth - 1; ++i)
//        printf("|   ");
//    printf("XXX\n");
    path[n-1] = 0;
    if (closedir(dp) < 0) {
        perror("Error: close");
        return EXIT_FAILURE;
    }
//    if(chdir("..")==-1){
//        perror("Error: chdir");
//        exit(EXIT_FAILURE);
//    }
    return (ret);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Error: no path in args");
        exit(EXIT_FAILURE);
    }
    struct timespec start, end;
//    clock_gettime(CLOCK_MONOTONIC, &start);
//    for(int i = 0; i < 1; ++i)
//        dopath(0, argv[1], strlen(argv[1]));
//    clock_gettime(CLOCK_MONOTONIC, &end);
//    printf("time short %lf\n", (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
    char *path = malloc(PATH_MAX + 1);
    size_t len = PATH_MAX + 1;
    strcpy(path, argv[1]);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < 1; ++i)
        dopath_long(0, path, len);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time full %lf\n", (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
}
