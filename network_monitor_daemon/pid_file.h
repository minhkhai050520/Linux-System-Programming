#ifndef PID_FILE_H
#define PID_FILE_H

#include <unistd.h>
#include <sys/types.h>

/* Function to ensure directory of the full path exists */
int ensure_dir(const char *path);

/* Function to write PID to file */
int write_pid_file(void);

/* Function to read PID from file */
pid_t read_pid_file(void);

/* Function to remove PID file */
void remove_pid_file(void);

#endif /* PID_FILE_H */