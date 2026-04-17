#define _GNU_SOURCE
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>
#include "syslog_logger.h"
#include "config.h"

/* Initialize syslog with the configured ident */
void init_syslog(void)
{
    openlog(config.log_ident, LOG_PID | LOG_CONS, LOG_DAEMON);
}

/* Log a message with printf-style formatting.
 * This function formats the variable argument list into a fixed-size
 * message buffer, then sends the result to syslog with the function name
 * prefix.
 *
 * priority - syslog priority level
 * func     - caller function name used as a prefix in the log message
 * format   - printf-style format string
 * ...      - printf-style arguments matching `format`
 */
void syslog_log_internal(int priority, const char *func, const char *format, ...)
{
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    syslog(priority, "[%s] %s", func, message);
}

/* Close syslog */
void close_syslog(void)
{
    closelog();
}