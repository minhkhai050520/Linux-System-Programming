#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "daemon.h"
#include "syslog_logger.h"

/**
 * daemonize - Transform the calling process into a background daemon.
 *
 * This implementation follows the 7-step standard (TLPI/POSIX) to ensure 
 * the process is fully detached from the environment:
 *
 * 1. Initial fork(): Moves the process to the background and ensures the 
 * child is not a process group leader.
 * 2. setsid(): Creates a new session, making the process the leader of a 
 * new process group and detaching it from the controlling terminal (TTY).
 * 3. Second fork(): Ensures the process is no longer a session leader. 
 * This prevents it from ever re-acquiring a controlling terminal.
 * 4. umask(0): Resets the file mode creation mask to 0, allowing the daemon 
 * full control over permissions of files it creates.
 * 5. chdir("/"): Changes the working directory to the root to avoid 
 * preventing unmounts of the filesystem the daemon was started in.
 * 6. Close all FDs: Closes all inherited file descriptors to release 
 * resources and avoid "leaking" open files from the parent.
 * 7. Redirect 0, 1, 2: Maps stdin, stdout, and stderr to /dev/null. This 
 * prevents library I/O functions from failing or corrupting data if 
 * new files are opened using those descriptors.
 *
 * Returns 0 on success, or -1 on error.
 */
int daemonize(int flags)
{
    pid_t pid;
    int maxfd, fd;

    /* Become background process */
    switch((pid = fork()))
    {
        case -1:
            syslog_log(LOG_ERR, "Failed to fork\n");
            return -1; /* fork failed */
        case 0:
            break; /* Child falls through */
        default:
            _exit(EXIT_SUCCESS); /* while parent terminates */
    }

    /* Child becomes session leader */
    if (setsid() < 0)
    {
        syslog_log(LOG_ERR, "Failed to create new session\n");
        return -1;
    }
    /* Ensure we are not session leader */
    switch((pid = fork()))
    {
        case -1:
            syslog_log(LOG_ERR, "Failed to fork\n");
            return -1;
        case 0:
            break;
        default:
            _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
    {
        syslog_log(LOG_INFO, "Resetting file mode creation mask to 0\n");
        umask(0);
    }
    if (!(flags & BD_NO_CHDIR))
    {
        syslog_log(LOG_INFO, "Changing working directory to the root\n");
        chdir("/");
    }

    if (!(flags & BD_NO_CLOSE_FILES))
    {
        syslog_log(LOG_INFO, "Closing all open file descriptors\n");
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)
            maxfd = BD_MAX_CLOSE;

        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS))
    {
        syslog_log(LOG_INFO, "Redirecting stdin, stdout, stderr to /dev/null\n");
        close(STDIN_FILENO);

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)
        {
            syslog_log(LOG_ERR, "Unexpected file descriptor %d for /dev/null\n", fd);
            return -1;
        }
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        {
            syslog_log(LOG_ERR, "Failed to duplicate stdin to stdout\n");
            return -1;
        }
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        {
            syslog_log(LOG_ERR, "Failed to duplicate stdin to stderr\n");
            return -1;
        }
    }
    return 0;
}