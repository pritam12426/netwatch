I think it's a **great systems-programming project**. It's practical, teaches IPC, networking, daemon design, and you'll actually use it yourself.

Your architecture could be:

```text
┌──────────────┐
│  netmond     │   <-- daemon
└──────┬───────┘
       │
       ├─ libpcap capture
       ├─ count rx/tx bytes
       ├─ aggregate statistics
       ├─ store history
       └─ IPC server
              ▲
              │ Unix Socket
              ▼
┌──────────────┐
│ netmon-cli   │   <-- client
└──────────────┘
```

Example:

```bash
$ netmon today
RX: 3.2 GB
TX: 180 MB

$ netmon domains
youtube.com     1.8 GB
github.com      300 MB

$ netmon month
June 2026
RX: 72 GB
TX: 4 GB
```

### IPC choice

For a daemon/client architecture, I'd choose:

```text
Unix Domain Socket
```

over:

```text
FIFO
Shared Memory
TCP localhost
D-Bus
```

because:

* works on Linux and macOS
* bidirectional
* easy request/response
* permission control via filesystem
* easy to debug
* industry standard

Example socket:

```text
/tmp/netmond.sock
```

Client:

```c
send("stats today");
```

Daemon:

```c
recv(...)
process(...)
send(...)
```

---

### Data collection

You have two possible levels.

#### Level 1 (easy)

Only total traffic.

```text
RX today
TX today
RX month
TX month
```

No packet inspection.

This can be read from network interfaces.

Linux:

```text
/proc/net/dev
```

macOS:

```c
getifaddrs()
```

Very little CPU usage.

---

#### Level 2 (interesting)

Traffic by domain.

```text
youtube.com
github.com
reddit.com
```

Requires:

```c
libpcap
```

and TLS SNI parsing.

More complex but much cooler.

---

### Storage

Don't store every packet.

Store aggregates:

```c
typedef struct {
    uint64_t rx;
    uint64_t tx;
} TrafficStat;
```

For example:

```text
2026-06-04:
    rx=125634234
    tx=5234234

2026-06:
    rx=5342342342
    tx=234234234
```

You can periodically flush to disk:

```text
~/.local/share/netmond/stats.db
```

or

```text
~/Library/Application Support/netmond/
```

on macOS.

---

### Daemon lifecycle

```bash
netmond start
netmond stop
netmond status
```

Internally:

```c
fork();
setsid();
```

Traditional daemon style.

Or simply run in foreground during development:

```bash
netmond --foreground
```

which is much easier to debug.

---

### CPU and memory concerns

Don't process packets synchronously.

A nice design:

```text
Capture Thread
      │
      ▼
Ring Buffer
      │
      ▼
Worker Thread
      │
      ▼
Statistics Table
```

This scales much better.

---

### What I'd build first

Version 1:

```text
Daemon
Unix socket IPC
Total RX/TX
Daily statistics
Monthly statistics
CLI client
```

Version 2:

```text
libpcap
Domain tracking
Top domains
Real-time monitor
```

Version 3:

```text
ncurses TUI
Traffic graphs
Export JSON
Alerts
```

For a C portfolio project, a "network usage daemon + client" is substantially more interesting than another HTTP server or file utility because it combines:

* daemon programming
* IPC
* packet capture
* data aggregation
* persistence
* cross-platform support (Linux/macOS)

all in one project.
