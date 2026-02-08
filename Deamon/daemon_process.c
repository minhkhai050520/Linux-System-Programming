#include <sys/stat.h>
#include <fcntl.h>
#include "become_daemon.h"
#include "tlpi_hdr.h"

int becomeDaemon(int flags)
{
    int maxfd, fd;

    switch (fork()) {
        case -1: return -1;
        case 0: break;                          /*Child falls through*/
        default: _exit(EXIT_SUCCESS);           /*Parent terminates*/
    }

    if (setsid() == -1)                         /*Create a new session and become leader of new session*/
        return -1;

    switch(fork())
    {
        case -1: return -1;
        case 0: break;
        default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0);

    if (!(flags & BD_NO_CHDIR))
        chdir("/");

    if (!(flags & BD_NO_CLOSE_FILES))
    {
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)
            maxfd = BD_MAX_CLOSE;

        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS))
    {
        close(STDIN_FILENO);

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }
    return 0;
}

int main()
{
    if (becomeDaemon(0) == -1)
        exit(EXIT_FAILURE);

    sleep(60);

    exit(EXIT_SUCCESS);
}