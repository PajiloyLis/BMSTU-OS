#include <fcntl.h>
#include <unistd.h>

int main()
{
    char c;
    int fd1 = open("alphabet.txt",O_RDONLY);
    int fd2 = open("alphabet.txt",O_RDONLY);
    int flag1 = 1, flag2 = 1;
    while(flag1 != 0 || flag2 != 0)
    {
        flag1 = read(fd1,&c,1);
        if (flag1 > 0)
        {
            write(1,&c,1);
        }
        flag2 = read(fd2,&c,1);
        if (flag2 > 0)
        {
            write(1,&c,1);
        }
    }
    close(fd1);
    close(fd2);
    return 0;
}