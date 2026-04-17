CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = 

# Source files
SRCS = main.c daemon.c netlink_monitor.c syslog_logger.c unix_socket.c config.c signal_handler.c pid_file.c
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = network_monitor_daemon

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean