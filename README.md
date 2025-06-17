# TrimUI SmartPro (TG5040) Development Boilerplate

A Docker-based toolchain for building applications for the TG5040 (Trimui Smart Pro) device. This project includes a complete development environment with SDL2 support and a MinUI PAK creation system.

This is based on the [union-tg5040-toolchain](https://github.com/shauninman/union-tg5040-toolchain) project, which provides a foundation for building and running applications on the TG5040 platform.

## Table of Contents
- [Requirements](#requirements)
- [Project Structure](#project-structure)
- [Setting Up Your Development Environment](#setting-up-your-development-environment)
  - [macOS](#macos)
  - [Linux](#linux-debianubuntu)
  - [Windows](#windows)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [Development Workflow](#development-workflow)
- [Build Commands](#build-commands)
- [Creating a PAK](#creating-a-pak)
- [Hello World Example](#hello-world-example)
- [Using the Logger](#using-the-logger)
- [Troubleshooting](#troubleshooting)

## Requirements

- Docker installed and running on your host system
- Git for version control
- Make (GNU Make)
- Basic understanding of C programming with SDL2

## Project Structure

```
.
├── config.ini                 # Project configuration (name, version)
├── Dockerfile                 # Docker container definition
├── makefile                   # Root build system
├── README.md                  # This file
├── support/                   # Docker container setup scripts
│   ├── setup-env.sh
│   └── setup-toolchain.sh
└── workspace/                 # Your development workspace
    ├── Makefile               # Build rules for your application
    ├── res/                   # Resources (fonts, images, etc.)
    │   └── aller.ttf          # Default font
    └── src/                   # Source code
        ├── logger.c           # Logging system implementation
        ├── logger.h           # Logging system interface
        └── main.c             # Application entry point
```

When you build the project, two additional directories will be created (and are git-ignored):
- `workspace/.build/` - Contains intermediate build files
- `workspace/.output/` - Contains the final PAK file and distribution packages

## Setting Up Your Development Environment

Before getting started, you need to set up Docker, Git, and Make on your system.

### macOS

1. **Install Docker Desktop for Mac**:
   - Download [Docker Desktop for Mac](https://www.docker.com/products/docker-desktop)
   - Follow the installation instructions
   - Start Docker Desktop and ensure it's running (look for the whale icon in your menu bar)

2. **Install Git and Make**:
   - Install [Homebrew](https://brew.sh/) if you don't have it:
     ```zsh
     /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
     ```
   - Install Git and Make:
     ```zsh
     brew install git make
     ```

3. **Verify installations**:
   ```zsh
   docker --version
   git --version
   make --version
   ```

### Linux (Debian/Ubuntu)

1. **Install Docker**:
   ```bash
   sudo apt update
   sudo apt install -y apt-transport-https ca-certificates curl software-properties-common
   curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
   sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
   sudo apt update
   sudo apt install -y docker-ce
   sudo usermod -aG docker $USER
   ```
   Log out and log back in for the group changes to take effect.

2. **Install Git and Make**:
   ```bash
   sudo apt update
   sudo apt install -y git make
   ```

3. **Verify installations**:
   ```bash
   docker --version
   git --version
   make --version
   ```

### Windows

1. **Install Docker Desktop for Windows**:
   - Download [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop)
   - Follow the installation instructions
   - Enable WSL 2 if prompted
   - Start Docker Desktop and ensure it's running (look for the whale icon in your system tray)

2. **Install Git for Windows**:
   - Download [Git for Windows](https://gitforwindows.org/)
   - During installation, select "Use Git and optional Unix tools from the Command Prompt"
   - Choose "Checkout as-is, commit as-is" for line endings

3. **Install Make**:
   - Option 1 - Using Chocolatey:
     ```powershell
     # Install Chocolatey first if you don't have it
     Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
     
     # Install make
     choco install make
     ```
   - Option 2 - Using WSL (Windows Subsystem for Linux):
     ```powershell
     # Enable WSL
     wsl --install
     
     # Then in your WSL terminal:
     sudo apt update
     sudo apt install -y make
     ```

4. **Verify installations in PowerShell or Command Prompt**:
   ```powershell
   docker --version
   git --version
   make --version
   ```

## Getting Started

1. Clone this repository:
   ```zsh
   git clone git@github.com:marconvcm/trimui-smartpro-boilerplate.git --depth 1
   cd trimui-smartpro-boilerplate
   ```
   or fork it to your own GitHub account and clone your fork.

2. Set your project name and version in `config.ini`:
   ```ini
   [project]
   name = MyProject
   version = 1.0.0
   ```

3. Build the Docker container and enter the development shell:
   ```zsh
   make shell
   ```

4. Inside the Docker container, build your project:
   ```zsh
   cd ~/workspace
   make
   ```

## Configuration

The `config.ini` file at the root of the project controls:
- Project name (used for PAK creation)
- Version number

## Development Workflow

1. Edit source files in the `workspace/src/` directory
2. Add resources (images, fonts, etc.) to the `workspace/res/` directory
3. Build and test your application
4. Create a PAK for distribution

## Build Commands

From the project root (outside Docker):

| Command | Description |
|---------|-------------|
| `make shell` | Start a shell inside the Docker container |
| `make build` | Build the project |
| `make run` | Build and run the project inside Docker |
| `make pak` | Build and create a PAK file |
| `make pak-zip` | Create a compressed archive of the PAK |
| `make clean` | Remove build artifacts |
| `make cleanall` | Remove all build artifacts and Docker image |

Inside the Docker container:

| Command | Description |
|---------|-------------|
| `make` | Build the project |
| `make run` | Run the project |
| `make clean` | Clean build artifacts |

## Creating a PAK

PAK files are the distribution format for MinUI. To create a PAK:

```zsh
make pak
```

This will:
1. Build your application
2. Create a PAK structure in `workspace/.output/`
3. Copy resources from `workspace/res/`
4. Create launch scripts and metadata

To create a compressed archive of your PAK:

```zsh
make pak-zip
```

## Hello World Example

The toolchain includes a simple SDL2-based Hello World example in the `workspace/src/main.c` file. This example demonstrates:

- Setting up an SDL2 application for the TG5040
- Using the logger module
- Handling controller input
- Rendering text and graphics
- Proper cleanup on exit

The example features:
- A 10-second countdown timer with smooth animations
- Color transitions and dynamic scaling using linear interpolation (lerp)
- Automatic application exit when the timer reaches zero

You can examine the source code in `workspace/src/main.c` to understand how controller input is handled on the TG5040.

To build and run the example:

```zsh
make build
make run
```

## Using the Logger

The project includes a simple logging system that you can use in your code:

```c
#include "logger.h"

// Initialize with the desired log level
logger_init(LOG_LEVEL_DEBUG);  // Show all log messages

// Log messages at different levels
LOG_DEBUG("Debug message");    // For detailed debugging info
LOG_INFO("Info message");      // For general information
LOG_WARN("Warning message");   // For potential issues
LOG_ERROR("Error message");    // For recoverable errors
LOG_FATAL("Fatal message");    // For unrecoverable errors

// Clean up when done
logger_close();
```

Log messages include timestamps, log levels, and source file information.

## Troubleshooting

### Docker Issues

1. **Docker daemon not running:**
   ```
   Cannot connect to the Docker daemon...
   ```
   - **Solution:** Start Docker Desktop (Mac/Windows) or run `sudo systemctl start docker` (Linux)

2. **Permission issues on Linux:**
   ```
   Got permission denied while trying to connect to the Docker daemon socket...
   ```
   - **Solution:** Add your user to the docker group and log out/in:
     ```bash
     sudo usermod -aG docker $USER
     ```

3. **Memory/CPU limits:**
   - If Docker builds fail due to resource constraints:
     - Mac/Windows: Increase memory allocation in Docker Desktop settings
     - Linux: Modify `/etc/docker/daemon.json`

### Make Issues

1. **Make command not found:**
   - **Mac:** Run `brew install make`
   - **Linux:** Run `sudo apt install make` or equivalent for your distribution
   - **Windows:** Verify installation with Chocolatey or WSL

2. **Make errors:**
   - Check that you're running the commands from the correct directory
   - Ensure Docker is running and properly configured

### Cross-Compilation Issues

1. **ARM64 toolchain errors:**
   - Make sure you're building inside the Docker container which has the proper toolchain
   - Run `make shell` first, then build your project inside the container

2. **SDL2 linking errors:**
   - The container has SDL2 pre-installed; avoid installing it manually on your host

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.