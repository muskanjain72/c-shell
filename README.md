
# 🐚 Custom Unix Shell in C

## 📖 Introduction

This project implements a **fully functional, Unix-like custom shell in C**, developed using **POSIX-compliant system calls**. The objective of the project is to deeply understand how modern shells operate internally by building one from scratch, covering **command parsing, process management, file descriptor manipulation, inter-process communication, and job control**.

The shell is designed in a **modular and well-structured manner**, with functionality split across multiple `.c` and `.h` files to avoid monolithic code. It strictly adheres to **POSIX standards**, using only approved C libraries and system calls such as `fork`, `exec`, `wait`, `pipe`, `dup2`, `open`, and signal-handling APIs.

At its core, the shell:
- Accepts user input through a dynamic prompt
- Parses commands using a **context-free grammar**
- Validates syntax before execution
- Supports execution of both built-in and external commands

Beyond basic execution, the shell implements advanced features including:
- Input and output redirection
- Multi-stage command pipelines
- Sequential and background execution
- Persistent command history
- Process tracking and job control
- Robust signal handling for `Ctrl-C`, `Ctrl-Z`, and `Ctrl-D`
- Maintain command history across sessions

## ✨ Features

### 🔹 Custom Shell Prompt
```text
<username@systemname:current_directory>
```
- Shows current user and system name.
- The directory where the shell is started becomes the shell's home; paths under it are shown with `~`.
- Prompt updates dynamically and is shown when the shell is ready for input.

---

### 🔹 Input Parsing (CFG-based)
- Validates user input against the required context-free grammar and ignores extraneous whitespace.
- On invalid input the shell prints `Invalid Syntax!` and returns to the prompt.
- Supports pipes (`|`), redirections (`<`, `>`, `>>`), sequential (`;`) and background (`&`) operators.

---

### 🔹 Built-in Commands (detailed implementation)
<!-- - hop: change current directory. Supported arguments: `~`, `.`, `..`, `-`, or a path. Prints `No such directory!` on failure. -->
- hop :
	- Syntax: `hop ((~ | . | .. | - | name)*)?`
	- Behavior:
		- `~` or no arguments: change to the shell's home directory (stored at shell start).
		- `.` : do nothing.
		- `..` : change to the parent directory (if any).
		- `-` : switch to the previous working directory tracked by the shell; if none exists, print `No such directory!`.
		- `name` : change to the given relative or absolute path.
    -Prints `No such directory!` on failure.

- reveal:
    - Syntax: `reveal (-(a | l)*)* (~ | . | .. | - | name)?`
	- Behavior:
		- Flags:
			- `-a`: include hidden files (names starting with `.`)
			- `-l`: list entries one-per-line
			- Combined `-al` or multiple repeated flags are supported (order-insensitive).
		- Argument handling mirrors `hop`: `~`, `.`, `..`, `-`, or a path `name`. If omitted, defaults to current directory.
		- If more than one non-flag argument is passed, prints `reveal: Invalid Syntax!`.
	-Prints `reveal: Invalid Syntax!` for invalid usage and `No such directory!` if path missing.

- log :
    - Syntax: `log (purge | execute <index>)?`
	- Behavior:
		- Maintains a persistent history file (e.g., `.shell_log` in the `shell/` directory).
		- Stores up to 15 most recent shell_cmd strings (oldest overwritten when full).
		- `log` (no args): prints stored commands oldest-to-newest.
		- `log purge`: clears the history file and in-memory buffer.
		- `log execute <index>`: executes the command at the given index (one-indexed, index relative to the most-recent-first ordering). The executed command is not stored again in history.
	- On syntax errors prints `log: Invalid Syntax!`.

---

### 🔹 External Commands, Redirection & Pipes
- Executes external programs using `fork()` + `execvp()`; prints `Command not found!` if exec fails.
- Pipelines: commands are connected by `pipe()` and `dup2()`; each stage runs in a separate process.
- Redirections:
	- `<` reads from a file (open with `O_RDONLY`); prints `No such file or directory` on failure.
	- `>` creates/truncates (`O_WRONLY|O_CREAT|O_TRUNC, 0644`); `>>` appends (`O_WRONLY|O_CREAT|O_APPEND, 0644`); prints `Unable to create file for writing` on failure.
	- Only the last input or output redirection on a command takes effect.
- Foreground pipelines receive terminal control and the shell waits for them; background pipelines run detached (stdin from `/dev/null`) and the shell prints job info immediately.
- Exit status: the shell reports the exit code of the last command in a pipeline; children that fail `execvp()` exit with `127`.

---

### 🔹 Sequential (`;`) and Background (`&`) Execution
- Sequential ';':
    - The parser breaks the input into `cmd_group`s separated by `;` and executes them in order.
	- The shell waits for each `cmd_group` (full pipeline) to finish before starting the next one.
	- The shell prompt is shown only after all commands in the sequence have finished.
    
- Background `&` :
    - run a command in background; 
    shell prints `[job_number] pid` and immediately returns the prompt. 
    - The shell periodically (before parsing each new input) checks for completed background jobs using `waitpid(..., WNOHANG)` and reports
		- `command_name with pid process_id exited normally` or
		- `command_name with pid process_id exited abnormally`
    - The shell reports background job completion (normal/abnormal) before processing the next input.

---

### 🔹 Job Control & Signals
- `activities`: 

    - Prints currently tracked child processes (running or stopped) in the format:
		- `[pid] : command_name - State` where State is `Running` or `Stopped`.
	- The list is kept up-to-date: terminated processes are removed when noticed via `waitpid()`.

- `ping <pid> <signal>`: 
    - send `signal % 32` to a process;
    - prints `No such process found` or `Invalid syntax!` for errors.
    - On success prints `Sent signal <signal_number> to process with pid <pid>`.

- `fg [job_number]` / `bg [job_number]`: 

    - fg [job_number]: brings a background or stopped job to foreground. If the job is stopped it sends `SIGCONT` and then waits for it to finish or stop again.
    - If no job number is provided the newest job is used. If missing, prints `No such job`.
	
    - bg [job_number]: resumes a stopped background job by sending `SIGCONT`. Prints `[job_number] command_name &` on success.
    - If the job is running prints `Job already running`. If number invalid prints `No such job`.

- Ctrl-C (SIGINT): forwarded to foreground jobs; the shell itself continues running.

- Ctrl-Z (SIGTSTP): stops foreground job, moves it to job list as `Stopped`, and prints job info.

- Ctrl-D (EOF): prints `logout`, sends `SIGKILL` to child processes, and exits with status 0.

---

### 🔹 History & Persistence
- Command history persists across sessions (limited to 15 entries) and avoids storing consecutive duplicate entries.

---

## Implementation notes and policies
- The project strictly uses POSIX APIs and follows compilation flags as required by the assignment (see the Makefile for flags including `-D_POSIX_C_SOURCE=200809L` and `-std=c99`).
- Source code is modularized across `src/` and `include/` to separate concerns: parsing, execution, redirection/pipes, jobs and activity tracking, built-ins, and signal handling.
- The `Makefile` compiles all sources into `shell.out` inside the `shell/` directory (`make all`).

---

## 🗂️ Project Structure
```
shell/
├── src/
├── include/
├── Makefile
├── .shell_log
├── shell.out
└── README.md
```

---

## ⚙️ Compilation & Execution

### Prerequisites
Before building and running the custom shell, ensure the following are installed on your system:

- **Linux / Unix-based OS** (Ubuntu, Debian, Arch, macOS, WSL)
- **GCC compiler** (POSIX-compliant)
- **Make** utility
- A **POSIX-compliant environment**

> ⚠️ This shell relies strictly on POSIX system calls and is not supported on native Windows environments without WSL.

---

## 🛠️ Installation

### 1. Clone the Repository
```bash
git clone git@github.com:muskanjain72/c-shell.git
```

### 2. Navigate to the Project Directory
```bash
cd <c-shell>/shell
```

> The shell **must be compiled from inside the `shell` directory**, as required by the project structure.

---

## 🔨 Compilation

The project uses a `Makefile` to ensure correct compilation with **POSIX-compliant flags**.

```bash
make all
```

This command compiles all source files and generates the executable:

```
shell.out
```

If compilation is successful, the binary will be created **inside the `shell` directory**.

---

## ▶️ Running the Shell

After compilation, start the shell using:

```bash
./shell.out
```

You should now see the custom shell prompt:

```
<username@systemname:current_directory>
```

The shell is now ready to accept commands.

---

## 🧠 Learning Outcomes
- Understanding Unix shell internals
- Process creation and management using `fork`, `wait`, and `exec`
- File descriptor manipulation using `dup2` and `open`
- Inter-process communication using pipes
- Signal handling and job control
- Designing modular, maintainable C programs
- Modular C design

---
