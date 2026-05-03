# Network Monitor Daemon

A Linux daemon that monitors network interface events (UP/DOWN, IP changes) using Netlink sockets, logs events via syslog, and provides CLI communication via UNIX domain sockets.

## Purpose
- Learn daemon processes, IPC, and Linux network events
- Foundation for embedded Linux services

## Key Features
- Daemonizes the process (fork, setsid)
- Monitors interface UP/DOWN and IP address changes using Netlink socket
- Logs events via syslog
- CLI interface via UNIX domain socket for commands like status and stop
- **Single-threaded event-driven architecture using select() for socket multiplexing**

## TLPI Topics Covered
- Process creation & daemonization (fork, setsid)
- File I/O (PID file, config)
- Signals (SIGTERM for stop, SIGHUP for reload config)
- Sockets (UNIX domain for CLI, Netlink for network monitoring)
- Event multiplexing (select)
- Time & logging (syslog)

## Architecture

Instead of using threads, this daemon uses **select()** for event multiplexing:
- **Single main event loop** monitors both netlink and UNIX socket file descriptors
- **Cleaner and more efficient** than thread-based approach
- **Better for embedded systems** with resource constraints
- **Standard Linux I/O multiplexing** pattern

```
┌─────────────────────────────────────┐
│   Main Daemon Process               │
├─────────────────────────────────────┤
│  Signal Handlers (SIGTERM, SIGHUP)  │
├─────────────────────────────────────┤
│  Event Loop (select)                │
│  ├─ Netlink Socket (network events) │
│  └─ UNIX Socket (CLI commands)      │
├─────────────────────────────────────┤
│  Syslog (event logging)             │
└─────────────────────────────────────┘
```

## Building
```bash
make clean && make
```

## Usage
- Start the daemon: `sudo ./network_monitor_daemon start`
- Check status: `echo "status" | socat - UNIX-CONNECT:/run/network_monitor_app/network_monitor.sock`
- Stop the daemon: `echo "stop" | socat - UNIX-CONNECT:/run/network_monitor_app/network_monitor.sock`
- Reload config: `echo "reload" | socat - UNIX-CONNECT:/run/network_monitor_app/network_monitor.sock`

> Note: The daemon executable currently accepts only `start` as a command-line argument.
> Status, stop, and reload are handled through the UNIX domain socket CLI.

## CLI Commands
Connect to the UNIX socket at `/run/network_monitor_app/network_monitor.sock` and send:
- `status` - Get daemon status
- `stop` - Stop the daemon
- `reload` - Reload configuration

Example:
```bash
echo "status" | socat - UNIX-CONNECT:/run/network_monitor_app/network_monitor.sock
```

## Modules
- `main.c`: Entry point with select() event loop
- `daemon.c`: Daemonization (fork, setsid)
- `netlink_monitor.c`: Netlink socket setup and message processing
- `syslog_logger.c`: Syslog wrapper with printf-style formatting
- `unix_socket.c`: UNIX domain socket server
- `config.c`: Configuration management
- `signal_handler.c`: Signal handling (SIGTERM, SIGHUP)
- `pid_file.c`: PID file management

## Notes
- Requires root privileges for Netlink monitoring
- PID file: `/tmp/network_monitor_daemon.pid`
- Socket file: `/tmp/network_monitor.sock`
- Single-threaded design improves reliability and simplifies debugging