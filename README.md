What you're describing is a very common Unix architecture:

```text
user
  |
  v
myctl          <-- normal user command
  |
  v
IPC / socket / pipe
  |
  v
myd            <-- privileged daemon (root)
```

Examples:

```text
systemctl  ---> systemd
nmcli      ---> NetworkManager
docker     ---> dockerd
virsh      ---> libvirtd
bluetoothctl ---> bluetoothd
```

The user-facing command is usually **not** a wrapper executable anymore. It's more commonly called a:

* **client/server architecture**
* **daemon + control utility**
* **frontend/backend architecture**
* **command/daemon architecture**

---

For example, Docker:

```text
docker        (client)
    |
    | Unix socket
    v
dockerd       (root daemon)
```

When you run:

```bash
docker ps
```

the `docker` binary doesn't do the privileged work itself.

Instead it:

1. Connects to a Unix socket.
2. Sends a request.
3. `dockerd` performs the privileged operation.
4. Returns the result.

---

For your project, it could look like:

```text
ffpaneld      <-- root daemon
ffpanelctl    <-- user client
```

or

```text
ffpaneld      <-- service
ffpanel       <-- client
```

The daemon:

```text
- monitors files
- manages mounts
- touches protected directories
- runs as root
```

The client:

```text
- shows status
- starts jobs
- requests actions
- displays logs
```

Communication methods:

```text
Unix domain socket   (most common)
Named pipe (FIFO)
D-Bus
Shared memory
TCP localhost socket
```

For a modern Linux/macOS CLI application, I'd recommend:

```text
ffpaneld  <-- root daemon
ffpanel   <-- user client
```

using a Unix domain socket:

```text
/tmp/ffpanel.sock
```

or better:

```text
/run/ffpanel.sock
```

on Linux.

The architectural names you'll hear are:

* **Daemon + Client**
* **Client–Server**
* **Privileged Service + Control Utility**
* **Command/Daemon Pattern**

If the privileged binary stays running in the background and the user binary merely sends commands to it, "daemon + client" is the term most Unix developers would immediately recognize.













For a personal project, I'd rank them like this:

| Method             | Easy  | Safe  | Scalable | Recommendation      |
| ------------------ | ----- | ----- | -------- | ------------------- |
| FIFO               | ⭐⭐⭐⭐⭐ | ⭐⭐⭐   | ⭐        | Good for learning   |
| Unix Domain Socket | ⭐⭐⭐⭐  | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐    | Best choice         |
| TCP localhost      | ⭐⭐⭐   | ⭐⭐⭐   | ⭐⭐⭐⭐     | Usually unnecessary |
| Shared Memory      | ⭐     | ⭐⭐    | ⭐⭐⭐⭐⭐    | Complex             |
| D-Bus              | ⭐     | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐    | Heavyweight         |

### My recommendation

For a real project:

```text
ffpanel      (client)
      |
      v
/run/ffpanel.sock
      |
      v
ffpaneld     (daemon)
```

Use a **Unix Domain Socket**.

Why?

* Local machine only.
* Fast.
* Simple C APIs (`socket()`, `bind()`, `connect()`).
* Easy request/response.
* File permissions work naturally:

  ```bash
  srw-rw---- root ffpanel /run/ffpanel.sock
  ```
* Supports multiple clients.
* Used by many production daemons.

Examples:

```text
docker      <-> /var/run/docker.sock
podman      <-> podman.sock
cups        <-> cups.sock
```

---

### FIFO

Great for learning:

```text
client --> daemon
```

But eventually you'll want:

```text
client --> daemon --> reply
```

Then FIFOs become awkward:

```text
/tmp/in.fifo
/tmp/out.fifo
```

or one FIFO per client.

---

### Shared Memory

Only use when performance is critical:

```text
video
audio
large telemetry
game engines
```

Not for command/control messages.

---

### D-Bus

If you're writing a desktop Linux integration:

```text
NetworkManager
Bluetooth
GNOME services
```

Otherwise it's overkill.

---

### TCP localhost

```text
127.0.0.1:8080
```

Works everywhere, but you're exposing a network service even if it's only intended for local IPC.

Then you start thinking about:

* ports
* firewalls
* authentication
* accidental exposure

which Unix sockets avoid.

---

If I were building `ffpaneld` today in C, I'd do:

```text
ffpanel       -> CLI/TUI client
ffpaneld      -> root daemon
/run/ffpanel.sock
JSON messages
```

That's probably the sweet spot between **easy**, **safe**, and **maintainable**.
