#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "pid_file.h"

#define PID_FILE "/tmp/network_monitor_daemon.pid"

// Write the current process PID to the PID file
int write_pid_file(void) {
    FILE *fp = fopen(PID_FILE, "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
    return 0;
}

// Read the PID from the PID file
pid_t read_pid_file(void) {
    FILE *fp = fopen(PID_FILE, "r");
    if (fp == NULL) {
        return -1;
    }
    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return pid;
}

// Remove the PID file
void remove_pid_file(void) {
    unlink(PID_FILE);
}