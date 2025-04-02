#include <pthread.h>
#include <syslog.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define CONFFILE "/etc/daemon.conf"

void daemonize(const char *cmd) {
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("%s: can't get file limit\n", cmd);
        exit(1);
    }
    if ((pid = fork()) < 0) {
        printf("%s: can't fork\n", cmd);
        exit(1);
    } else if (pid != 0)
        exit(0);
    if (setsid() < 0) {
        printf("%s: can't create new session\n", cmd);
        exit(0);
    }
    sa.sa_handler = SIG_IGN;
    if (sigemptyset(&sa.sa_mask) == -1) {
        syslog(LOG_ERR, "Can't set signal mask: %s", strerror(errno));
        exit(1);
    }
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("%s: невозможно игнорировать сигнал SIGHUP", cmd);
        exit(1);
    }
    if (chdir("/") < 0) {
        printf("%s: can't change directory to /", cmd);
        exit(1);
    }
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
               fd0, fd1, fd2);
        exit(1);
    }
}

int lockfile(int fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

int already_running(void) {
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return (1);
        }
        syslog(LOG_ERR, "can't lock %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long) getpid());
    write(fd, buf, strlen(buf) + 1);
    return (0);
}

sigset_t mask;

void reread(void) {
    int fd = open(CONFFILE, O_RDONLY);
    if (fd == -1) {
        syslog(LOG_ERR, "Can't open config file");
        exit(1);
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        syslog(LOG_ERR, "Can't read config file: %s", strerror(errno));
        return;
    }
    char buf[st.st_size];
    if (read(fd, buf, st.st_size) < 0) {
        syslog(LOG_ERR, "Can't read config file: %s", strerror(errno));
        return;
    }
    syslog(LOG_INFO, "%s", buf);
}

void *thr_fn(void *arg) {
    int err, signo;

    while (1) {
        err = sigwait(&mask, &signo);
        if (err != 0) {
            syslog(LOG_ERR, "sigwait failed");
            exit(1);
        }

        switch (signo) {
            case SIGHUP:
                syslog(LOG_INFO, "recieved SIGHUP\n");
                reread();
                break;

            case SIGTERM:
                syslog(LOG_INFO, "got SIGTERM; exiting");
                exit(0);

            default:
                syslog(LOG_INFO, "unexpected signal %d\n", signo);
        }
    }
    return (0);
}

int main(int argc, char *argv[]) {
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;
    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    daemonize(cmd);
    if (already_running()) {
        syslog(LOG_ERR, "daemon already running");
        exit(1);
    }
    syslog(LOG_INFO, "daemon started\n");

    sa.sa_handler = SIG_DFL;
    if (sigemptyset(&sa.sa_mask) == -1) {
        syslog(LOG_ERR, "Can't set signal mask: %s", strerror(errno));
        exit(1);
    }
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        syslog(LOG_ERR, "%s: can't restore SIGHUP default", cmd);

    if (sigfillset(&mask) == -1) {
        syslog(LOG_ERR, "Can't set signal mask: %s", strerror(errno));
        exit(1);
    }
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
        syslog(LOG_ERR, "SIG_BLOCK error");
    }

    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
        syslog(LOG_ERR, "can't pcreate thread");
    while (1) {
        sleep(10);
        time_t cur_time = time(NULL);
        struct tm *now = localtime(&cur_time);
        syslog(LOG_INFO, "daemon running: %d.%d.%d %d:%d:%d\n", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900,
               now->tm_hour,
               now->tm_min, now->tm_sec);
    }
    exit(0);
}
