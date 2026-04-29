#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "pid_file.h"
#include "syslog_logger.h"
#include "config.h"

int ensure_dir(const char *path)
{
    char tmp[256];
    char *p;

    snprintf(tmp, sizeof(tmp), "%s", path);

    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
            {
                syslog_log(LOG_ERR, "Failed to create directory for PID file: %s", strerror(errno));
                return -1;
            }
            *p = '/';
        }
    }

    return 0;
}

/* Write the current process PID to the PID file */
int write_pid_file(void)
{
    if (ensure_dir(config.pid_file) == -1)
    {
        syslog_log(LOG_ERR, "Failed to ensure directory for PID file exists");
        return -1;
    }

    FILE *fp = fopen(config.pid_file, "w");
    if (fp == NULL)
    {
        syslog_log(LOG_ERR, "Failed to open PID file for writing: %s", strerror(errno));
        return -1;
    }
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
    return 0;
}

/* Read the PID from the PID file */
pid_t read_pid_file(void)
{
    FILE *fp = fopen(config.pid_file, "r");
    if (fp == NULL)
    {
        syslog_log(LOG_ERR, "Failed to open PID file for reading: %s", strerror(errno));
        return -1;
    }
    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1)
    {
        syslog_log(LOG_ERR, "Failed to read PID from file: %s", strerror(errno));
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return pid;
}

/* Remove the PID file */
void remove_pid_file(void)
{
    if (unlink(config.pid_file) != 0)
        if (errno != ENOENT)
            syslog_log(LOG_ERR, "Failed to remove PID file: %s", strerror(errno));
}