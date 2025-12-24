# **Unix-Shell**

## **Description**
Unix-like shell supporting process creation, foreground/background execution, signal handling, and command parsing using POSIX system calls.


This project is a custom Unix-like shell implemented in C. It provides a command-line interface for executing both internal and external commands, managing processes, and maintaining a history of commands. The shell is designed to mimic the behavior of a typical Unix shell, making it a great demonstration of system-level programming concepts.

---

## **Features**
- **Internal Commands**:
  - `exit`: Exit the shell.
  - `pwd`: Print the current working directory.
  - `cd`: Change the current directory, with support for:
    - `cd` (no arguments): Navigate to the home directory.
    - `cd -`: Return to the previous directory.
    - `cd ~/dir`: Expand `~` to the home directory.
  - `help`: Display help information for internal commands.
  - `history`: View and re-run commands from the history.
    - `!!`: Re-run the last command.
    - `!n`: Re-run the nth command in history.

- **External Commands**:
  - Supports execution of external programs using `execvp`.
  - Background process execution with `&`.

- **Command History**:
  - Tracks the last 10 commands entered.
  - Displays commands in reverse order (most recent first).

- **Signal Handling**:
  - Handles `Ctrl+C` (`SIGINT`) gracefully without crashing the shell.

- **Process Management**:
  - Supports foreground and background processes.
  - Reaps zombie processes to prevent resource leaks.

---

## **Getting Started**

### **Prerequisites**
To build and run this project, you need:
- A Unix-like environment (Linux, macOS, or WSL on Windows).
- `gcc` or `g++` (for compiling the code).
- `CMake` (for building the project).

---

### **Setting Up the Environment**

#### **Using WSL (Windows Subsystem for Linux)**
1. Install WSL:
   - Open PowerShell as Administrator and run:
     ```powershell
     wsl --install
     ```
   - Restart your computer if prompted.
2. Install a Linux distribution (e.g., Ubuntu) from the Microsoft Store.
3. Open the WSL terminal and install the required tools:
   ```bash
   sudo apt update
   sudo apt install build-essential cmake