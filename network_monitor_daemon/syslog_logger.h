#ifndef SYSLOG_LOGGER_H
#define SYSLOG_LOGGER_H

#include <syslog.h>
#include <stdarg.h>

/* Initialize syslog */
void init_syslog(void);

/* Log a message with printf-style formatting */
void syslog_log_internal(int priority, const char *func, const char *format, ...);

/* Close syslog */
void close_syslog(void);

/* Macro to add function name to syslog messages */
#define syslog_log(priority, format, ...) \
    syslog_log_internal(priority, __func__, format, ##__VA_ARGS__)

#endif /* SYSLOG_LOGGER_H */