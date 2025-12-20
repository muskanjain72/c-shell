
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
```
<username@systemname:current_directory>
```
- Shows the current user and system name using `getlogin()` / `getpwuid(getuid())` and `gethostname()`.
- The current directory is printed relative to the shell's home directory; when the home directory is an ancestor it is replaced by `~`.
- Prompt is displayed only when the shell is not running a foreground job.

---

### 🔹 Input Parsing (CFG-based)
- The shell validates user input using the supplied context-free grammar for `shell_cmd`, `cmd_group`, `atomic`, `input`, and `output` tokens.
- The parser tokenizes the input while ignoring whitespace (space, tab, CR, LF). It performs a left-to-right recursive/iterative parse to verify grammar conformance and construct the internal command representation used by the executor.
- On invalid input the shell prints `Invalid Syntax!` and returns to the prompt without executing any commands.
- The parser supports separators `;` (sequential) and `&` (background) and recognizes pipelines `|`, and redirections `<`, `>`, `>>`.

---

### 🔹 Built-in Commands (detailed implementation)

The shell implements several built-ins that modify the shell state or require special handling. All built-ins are defined across `src/` files and declared in `include/*.h`. Below are implementation notes and behavior guarantees for each built-in.

- hop (implemented in `src/hop.c`)
	- Syntax: `hop ((~ | . | .. | - | name)*)?`
	- Behavior:
		- `~` or no arguments: change to the shell's home directory (stored at shell start).
		- `.` : do nothing.
		- `..` : change to the parent directory (if any).
		- `-` : switch to the previous working directory tracked by the shell; if none exists, print `No such directory!`.
		- `name` : change to the given relative or absolute path.
	- Implementation details:
		- Uses `chdir()` to change directories and `getcwd()` to update the current working directory string.
		- Maintains a `prev_cwd` string for the `-` operation.
		- On chdir failure prints `No such directory!` and does not change `prev_cwd`.

- reveal (implemented in `src/reveal.c`)
	- Syntax: `reveal (-(a | l)*)* (~ | . | .. | - | name)?`
	- Behavior:
		- Flags:
			- `-a`: include hidden files (names starting with `.`)
			- `-l`: list entries one-per-line
			- Combined `-al` or multiple repeated flags are supported (order-insensitive).
		- Argument handling mirrors `hop`: `~`, `.`, `..`, `-`, or a path `name`. If omitted, defaults to current directory.
		- If more than one non-flag argument is passed, prints `reveal: Invalid Syntax!`.
		- If the directory does not exist, prints `No such directory!`.
	- Implementation details:
		- Uses `opendir()` / `readdir()` to read directory entries and `closedir()` afterwards.
		- Filters entries according to `-a` and sorts the resulting names using `qsort()` and `strcmp()` (ASCII lexicographic order).
		- For the `-l` flag each entry is printed on its own line; otherwise entries are printed in columns similar to `ls` (simple space-separated list).

- log (implemented in `src/log.c`)
	- Syntax: `log (purge | execute <index>)?`
	- Behavior:
		- Maintains a persistent history file (e.g., `.shell_log` in the `shell/` directory).
		- Stores up to 15 most recent shell_cmd strings (oldest overwritten when full).
		- Does not add a command to the history if it is identical to the most recent stored entry.
		- Does not store a shell_cmd whose first atomic command is `log` itself.
		- `log` (no args): prints stored commands oldest-to-newest.
		- `log purge`: clears the history file and in-memory buffer.
		- `log execute <index>`: executes the command at the given index (one-indexed, index relative to the most-recent-first ordering). The executed command is not stored again in history.
		- On syntax errors prints `log: Invalid Syntax!`.
	- Implementation details:
		- History is read on shell startup into an in-memory ring buffer and synced to `.shell_log` on changes.
		- Uses safe string handling to preserve exact shell_cmd text for re-execution.

---

### 🔹 External Command Execution, Redirection & Pipes

- Command execution (implemented across `src/parser.c`, `src/pipes.c`, and a central executor):
	- After parsing, the executor builds a pipeline of `atomic` commands and their requested redirections.
	- Each external command is launched in a child process using `fork()` followed by `execvp()` (POSIX exec family is allowed for later parts; note earlier parts restrict exec* syscalls during initial development as per assignment constraints).
	- If `execvp()` fails, prints `Command not found!` and the child exits with a non-zero status.

- Input redirection (`<`):
	- Each `atomic` command keeps the last seen input redirection.
	- Before `execvp()` the child opens the specified file with `open(..., O_RDONLY)`; on failure prints `No such file or directory` and does not execute the command.
	- Uses `dup2()` to map the opened fd to `STDIN_FILENO` and closes the original fd.

- Output redirection (`>` and `>>`):
	- `>`: open with `open(..., O_WRONLY|O_CREAT|O_TRUNC, 0644)`
	- `>>`: open with `open(..., O_WRONLY|O_CREAT|O_APPEND, 0644)`
	- On failure to create or open, prints `Unable to create file for writing` and skips executing that command.
	- Only the last output redirection on an atomic command takes effect.

- Pipes (`|`) and combined I/O:
	- For a pipeline of N commands, the shell creates N-1 pipes with `pipe()`.
	- Each command `i` is forked. The child sets up input and output fds using `dup2()` to connect to the appropriate pipe ends and also applies redirections if present.
	- Parent closes unused pipe file descriptors and waits for all children to finish (unless the pipeline is launched in the background).
	- File redirection and pipes work together; e.g., `command1 < in | command2 > out` is supported by applying the input redirection to `command1` and output redirection to `command2` in their respective child processes.

---

### 🔹 Sequential (`;`) and Background (`&`) Execution

- Sequential `;` (implemented in `src/pipes.c` / `src/activities.c`):
	- The parser breaks the input into `cmd_group`s separated by `;` and executes them in order.
	- The shell waits for each `cmd_group` (full pipeline) to finish before starting the next one.
	- The shell prompt is shown only after all commands in the sequence have finished.

- Background `&` and job numbers:
	- When a `cmd_group` ends with `&`, the shell starts the command(s) in the background and immediately returns the prompt.
	- Background processes are tracked in an in-memory job list persisted during the session. Each background job is assigned a job number starting at 1 and increasing.
	- On job creation the shell prints: `[job_number] process_id`.
	- The shell periodically (before parsing each new input) checks for completed background jobs using `waitpid(..., WNOHANG)` and reports
		- `command_name with pid process_id exited normally` or
		- `command_name with pid process_id exited abnormally`
	- Background processes have their stdin redirected from `/dev/null` so they cannot read from the terminal.

---

### 🔹 Job Control, Activities, ping, fg, bg, and Signals

- activities (implemented in `src/activities.c`)
	- Syntax: `activities`
	- Prints currently tracked child processes (running or stopped) in the format:
		- `[pid] : command_name - State` where State is `Running` or `Stopped`.
	- The list is kept up-to-date: terminated processes are removed when noticed via `waitpid()`.
	- Output is sorted lexicographically by `command_name` before printing.

- ping (implemented in `src/ping.c`)
	- Syntax: `ping <pid> <signal_number>`
	- Validates numeric PID and signal number. Signals are delivered as `signal_number % 32`.
	- If `kill(pid, sig)` fails with ESRCH the command prints `No such process found`.
	- On success prints `Sent signal <signal_number> to process with pid <pid>`.
	- On invalid numeric input prints `Invalid syntax!`.

- Ctrl-C (SIGINT) handling (`src/signals.c`)
	- The shell installs a SIGINT handler that forwards SIGINT to the foreground process group (if one exists) using `killpg()`.
	- The shell itself ignores termination from SIGINT and continues running after printing a fresh prompt.

- Ctrl-Z (SIGTSTP) handling
	- The SIGTSTP handler forwards SIGTSTP to the foreground process group. When a process stops the shell captures that state and moves the job to the background job list with status `Stopped`.
	- The shell prints: `[job_number] Stopped command_name`.
	- The shell does not itself stop.

- Ctrl-D (EOF)
	- On EOF the shell prints `logout`, sends `SIGKILL` to all child processes, and exits with status 0.

- fg and bg (implemented in `src/jobs.c` / `src/activities.c`)
	- fg [job_number]: brings a background or stopped job to foreground. If the job is stopped it sends `SIGCONT` and then waits for it to finish or stop again. If no job number is provided the newest job is used. If missing, prints `No such job`.
	- bg [job_number]: resumes a stopped background job by sending `SIGCONT`. Prints `[job_number] command_name &` on success. If the job is running prints `Job already running`. If number invalid prints `No such job`.

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
