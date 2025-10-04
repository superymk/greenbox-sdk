# GreenBox RDesktop

GreenBox RDesktop is a lightweight, secure VNC client optimized for protected Virtual Desktop Infrastructure (VDI) environments. It offers scalable display, a range of encoding methods, and robust authentication options—all packed into an intuitive, end-user–friendly interface.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Configuration](#configuration)

   * [Server Setup](#server-setup)
   * [Client Setup](#client-setup)
4. [Features](#features)

   * [Supported Encodings](#supported-encodings)
   * [Security & Authentication](#security--authentication)
5. [Limitations & Known Issues](#limitations--known-issues)
6. [Troubleshooting](#troubleshooting)
7. [Future Work](#future-work)
8. [License](#license)

---

## Prerequisites

* A VDI server running a trusted OS with a VNC server installed (e.g., UltraVNC, TightVNC, TigerVNC).
* GreenBox and GreenBox RDesktop client installed on your local machine.
* Network access between client and server.

---

## Quick Start

1. **Start the Server**: Ensure your VDI server is powered on and VNC service is running.
2. **Configure Client**:

   * Launch **GreenBox**.
   * Open **GreenBox RDesktop** (via `RDesktop.exe`).
3. **Connect**:

   1. Enter your server’s address (`hostname:port`).
   2. Input the VNC password when prompted.
   3. Click **Connect** and enjoy your secure remote desktop.

---

## Configuration

### Server Setup

1. **Install VNC Server**

   * Choose and install a VNC server (e.g., UltraVNC, TightVNC, TigerVNC).
2. **Secure the Server**

   * Apply best commodity protection of the server; e.g., Endpoint Detection and Response (EDR) tools, OS-level hardening, apply latest updates.
   * Enable a strong password for VNC connections.
3. **Optimize Display**

   * Set server resolution to a moderate size (e.g., **1280×800**) to balance clarity and bandwidth.

### Client Setup

1. Configure **GreenBox**:

   * **Keyboard & Mouse**: Choose the correct USB devices.
   * **Screen Resolution**: Match or exceed the server’s resolution.
2. Run **GreenBox**, and then run **GreenBox RDesktop (RDesktop.exe)**. Type the network address of the server and then click "Connect".

---

## Features

### Supported Encodings

GreenBox RDesktop leverages **libvncserver**, offering all common encodings for efficient screen updates:

| Encoding | Code | Supported |
| -------- | ---- | --------- |
| Raw      | 1    | ✓         |
| CopyRect | 2    | ✓         |
| RRE      | 3    | ✓         |
| CoRRE    | 4    | ✓         |
| Hextile  | 5    | ✓         |
| Zlib     | 6    | ✓         |
| Tight    | 7    | ✓         |
| Ultra    | 9    | ✓         |
| TRLE     | 15   | ✓         |
| ZRLE     | 16   | ✓         |
| ZYWRLE   | 17   | ✓         |
| TightPNG | –260 | ✓         |

### Security & Authentication

Supports multiple VNC security types for encrypted and authenticated sessions:

| Security Type       | Code       | Supported |
| ------------------- | ---------- | --------- |
| None                | 1          | ✓         |
| VNC Password        | 2          | ✓         |
| SASL                | 20         | ✓         |
| MSLogon             | 0xFFFFFFFA | ✓         |
| Apple ARD           | 30         | ✓         |
| TLS / VeNCrypt      | 18 / 19    | ✓         |
| UltraVNC MSLogon II | 113        | ✓         |

---

## Limitations & Known Issues

* **Authentication**: Currently only password-based VNC authentication is supported. VNC servers using X.509 certificates and UltraVNC SecureVNC plugin are not yet supported.
* **Video Streaming**: Not optimized for high-frame-rate video. Expect \~20 FPS for videos at 640×400 over LAN; full-screen video may lag more.

> **Windows TLS Note**: TigerVNC on Windows supports TLS encryption but exhibit performance issues in the server. Linux-based VNC servers and QEMU may deliver better TLS performance.

**Sample Connection Log** (to TigerVNC with TLS):

```text
08/05/2025 18:51:39 Connecting to VNC server (protocol 3.8)
08/05/2025 18:51:39 Negotiating VeNCrypt (security code 19)
08/05/2025 18:51:39 TLS session initialized
08/05/2025 18:51:44 Authentication succeeded
08/05/2025 18:51:44 Session established: DESKTOP-XXXXXXX
```

---

## Troubleshooting

* **Cannot Connect**: Verify server address, port, and firewall settings.
* **Authentication Failed**: Double-check VNC password and supported security type.
* **Poor Performance**: Lower server resolution or switch to a simpler encoding (e.g., Raw or CopyRect).
* **Blank Screen**: Ensure display on the VDI host is active and not sleeping.

---

## Future Work

1. **SSH Tunneling** for encrypted traffic to any VNC server.
2. **Additional Auth Methods** such as X.509 certificate support.
3. **Performance Enhancements** for smoother video playback.

---

## License
The "secapp" folder is distributed under the **GPLv2**. See [LICENSE-secapp](./LICENSE-secapp) for details.