# xzpv

A high-performance, cross-platform core archiving engine written in modern C23. It seamlessly bridges Unix shell pipelines with real-time visual progress reporting, featuring automated privilege elevation checking and zero-configuration environment setup.

[![Release Toolchain Pipeline](https://github.com)](https://github.com)
[![License: MIT](https://shields.io)](https://opensource.org)
[![Language: C23](https://shields.io)](https://cppreference.com)

---

## 🚀 Features

- **Visual Pipeline Monitoring**: Utilizes `pv` stream tracking under the hood to calculate and display native throughput progress bars during heavy XZ compression loops.
- **Dynamic Context Elevation**: Automatically detects standard user vs. administrative (`root`/`sudo`) privileges to safely separate system-wide configurations (`/etc/`) from user workspace settings (`~/.config/`).
- **First-Run Autogen Layout**: Zero pre-configuration required. The tool generates its own native profile structures safely on its first functional launch.
- **Modern Architecture**: Compiled against strict C23 procedural patterns with namespaced include tracking and integrated automated unit tests.

---

## 📦 Installation

### 🍺 macOS / Unix (Homebrew)

You can install `xzpv` natively from your custom Homebrew tap setup:

```bash
brew tap yourusername/tap
brew install xzpv
```

### 🐧 Linux (Debian / Ubuntu / APT)

Download the latest `.deb` package release architecture asset directly from our GitHub Releases panel and run:

```bash
sudo apt update
sudo apt install ./xzpv.deb
```

---

## 🛠️ Usage

```text
Usage: xzpv [options] <target>

Options:
  -h, --help                 Display this usage menu interface and exit.
  -c <level>, --compression  Level of compression to apply (1-9, defaults to 6).
  -d <dest>, --destination   Explicit target path for the final compressed archive file.
  -v, --verbose              Unlock detailed internal runtime debugging logs.
  -x, --delete               Automatically delete the original source target files upon successful archival.
  --dry                      Simulate the entire process in terminal without modifying storage.
```

### Quick Examples

```bash
# Compress a project directory with high compression and active visual logging
xzpv -c 9 -v my_large_folder/

# Compress a directory to a custom path and wipe out the original source folder upon completion
xzpv -x -d /backups/data.tar.xz my_large_folder/
```

---

## 🖥️ Cross-Platform Support Matrix

| Operating System | Archiving Engine Experience | Notes / Prerequisites |
| :--- | :--- | :--- |
| **macOS** | ✨ **Premium** | Requires `pv` and `xz` (Installed automatically via Homebrew). |
| **Linux** | ✨ **Premium** | Requires `pv` and `xz-utils` (Installed automatically via APT). |
| **Windows** | ⚠️ *Degraded* | Falls back to native system `tar` Gzip compression. Visual progress monitoring (`pv`) and extreme XZ tracking metrics are currently bypassed on this environment. Full native integration is scheduled for a future release cycle. |

---

## 🧑‍💻 Local Development & Testing

This project leverages standard `CMake` build layouts alongside native `CTest` validation tracking scripts.

### Prerequisites

Ensure you have a standard C compiler supporting C23 features, alongside CMake:
```bash
# macOS
brew install cmake gcc

# Linux
sudo apt install cmake gcc build-essential
```

### Compilation & Testing Workflow

```bash
# 1. Configure the build environment
cmake -S . -B build

# 2. Compile the binaries and testing suites
cmake --build build

# 3. Execute the internal automated test suite
cd build && ctest --output-on-failure
```

---

## 📄 License

This project is licensed under the terms of the **MIT License**. See the `LICENSE` file for additional parameters.
