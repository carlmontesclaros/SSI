# Hello and Welcome! This Simple Shell Interpreter was a Project in one of courses and was a fun way to refresh C programming

This README.md file contains the features that was specified as well as my whole thought process from almost at the very start of the 
project until the end. The features and knows issues is listed below.

To build, run make to compile and produces executable called ssi per grading spec.
Also included make clean to remove the compiled executable

Implemented Features:

Foreground Execution

Runs programs via using fork/execvp/waitpid.
Can correctly run ls, pwd, date with the correct outputs.
Can execute commands with parameters ls -alh, ls -alh ./, uname -a, etc... with the correct output and content.
Can run long running programs like ping 1.1.1.1 and using CTRL-C can stop it correctly.
Can handle the case where external programs that don't exist with the output <name>: No such file or directory.

Changing Directories

Can print current working directory by pwd
Can change absolute paths like cd /tmp, relative paths like cd .
cd with no arguement goes back to home direcotry
~ works and can be used such as cd ~
The prompt always shows the current working directory

Background Execution

bg runs program in the background and shell does not have to wait
bglist command shows all running background and has a total count
Jobs that are terminated is announced in shell as <pid>: <command> has terminated on the next prompt and removed from the list

Signal Handling

SIGINT terminates foreground programs and not the shell
At empty prompt CTRL-C shows a new prompt on a new line
CTRL-D at empty prompt exits the shell, CTRL-C does not exit shell

ISSUE(S):

Found this out when I accidentally typed "bg list" instead of "bglist" and it treats "list" as a background command.
It fails and has a not found message announce but still appears in the bglist then is reaped and removed on the next prompt.
As far as I know, nothing serious about this issue and is a minor issue.

