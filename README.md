# **Unix Shell**

Unix-like shell implemented in C. It provides a command-line interface for executing internal and external commands, managing foreground and background processes, handling signals, and maintaining a command history.

## **Features**
- **External Commands**:
  - Supports execution of external programs using `fork()` and `execvp`.
  - Background process execution with `&`.

- **Command History**:
  - Tracks the last 10 commands entered.
  - Displays commands in reverse order (most recent first).

- **Signal Handling**:
  - Handles `SIGINT` (`Ctrl+C`) appropriately without crashing the shell.

- **Process Management**:
  - Supports foreground and background processes.
  - Cleans up zombie processes to prevent resource leaks.

- **Built-in Commands**:
  - `help`: Display help information for internal commands.
  - `pwd`: Print the current working directory.
  - `cd`: Change the current directory, with support for:
    - `cd` (no arguments): Navigate to the home directory.
    - `cd -`: Return to the previous directory.
    - `cd ~/dir`: Expand `~` to the home directory.
  - `history`: View and re-run commands from the history.
    - `!!`: Re-run the last command.
    - `!n`: Re-run the nth command in history.
  - `exit`: Exit the shell.

## **Running the Shell**

### **Prerequisites**
To build and run this project, you need:
- A Unix-like environment (Linux, macOS, or WSL on Windows).
- `gcc` or `clang` (for compiling the code).
- `CMake` (for building the project).

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
   ```

### **Building and Running the executable**
1. Clone the project and navigate to the project root:
   ```bash
   git clone <repo-url>
   cd Unix-Shell
   ```
2. Create and navigate to the build directory:
   ```bash
   mkdir build
   cd build
   ```
3. Create the executable:
   ```bash
   cmake ..
   make
   ```
4. Run the shell executable:
   ```bash
   ./shell
   ```

## **Technical Highlights**
- **System-Level Programming**:
  - Uses system calls like `fork`, `execvp`, and `waitpid` to manage processes.
  - Implements signal handling with `sigaction` to gracefully handle interrupts (e.g., `Ctrl+C`).

- **Command Parsing**:
  - Tokenizes user input to parse commands, arguments, and background execution (`&`).

- **Dynamic Memory Management**:
  - Dynamically allocates and manages memory for command history and input parsing.

- **Portability**:
  - Designed for POSIX-compliant Unix-like systems (Linux, macOS, WSL).
 
## Limitations
- **No Advanced Shell Features**:
  - Does not support piping (`|`) or input/output redirection (`>`, `<`).

- **Limited History**:
  - Command history is limited to the last 10 commands and is not persistent across sessions.

- **No Job Control**:
  - Does not implement job control commands such as `fg`, `bg`, or `jobs`.

- **Platform Constraints**:
  - Intended for Unix-like environments, not designed to run natively on Windows without WSL.


   
