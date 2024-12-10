# Unix Shell

This is a simple unix shell project, to get used to linux programming, and how processes are created, destroyed and managed.

## Overview

The shell should operate in this basic way:
when you type in a command (in response to its prompt), the shell
creates a child process that executes the command you entered and then prompts
for more user input when it has finished.

The shells you implement will be similar to, but simpler than, the one you run
every day in Unix. 

### Source Code Directories
msh - contains the file msh.c

tester - test harness. 

### Building the Shell
You can build the code by typing:
```
make
```
### Testing the Shell
You can run the provided tests by typing:
```
./test-msh.sh
```

### Development Environment

Tested on github codespaces

### Basic Shell: `msh`

msh our basic shell is an interactive loop: it repeatedly 
prints a prompt `msh> `, parses the input, executes the command specified 
on that line of input, and waits for the command to finish. 
This is repeated until the user types `exit`. 


### Interactive Mode

The shell can be invoked with either no arguments or a single argument;
anything else is an error. Here is the no-argument way:

```
prompt> ./msh
msh> 
```

At this point, `msh` is running, and ready to accept commands. Type away!

The mode above is called *interactive* mode, and allows the user to type
commands directly. 

### Batch Mode

The shell also supports a *batch mode*, which instead reads
input from a file and executes commands from therein. Here is how you
run the shell with a batch file named `batch.txt`:

```
prompt> ./msh batch.txt
```

One difference between batch and interactive modes: in interactive mode, a
prompt is printed (`msh> `). In batch mode, no prompt should be printed.

## Structure

### Paths

**Important:** Note that the shell itself does not *implement* `ls` or other
commands (except built-ins). 

### Built-in Commands

* `exit`: When the user types `exit`, your shell should simply call the `exit`
  system call with 0 as a parameter. It is an error to pass any arguments to
  `exit`. 

* `cd`: `cd` always take one argument (0 or >1 args should be signaled as an
error). To change directories, use the `chdir()` system call with the argument
supplied by the user; if `chdir` fails, that is also an error.

*  `quit`: When the user types `quit`, your shell should simply call the `exit`
  system call with 0 as a parameter. It is an error to pass any arguments to
  `quit`. 
