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

---
size_t cannot be negative thats why len is set with size_t. ssize_t can be negative.
they both just hold sizes


**explaination of fork():**

normal functions run and return once. fork() returns twice since when calling fork() the os makes a near identical copy of entire running program.
2 processes the parent which is original and child which is the copy. They both sit at the same spot the line after fork() and both continue running from there.
fork() knows which since in child fork() returns 0 saying "im the child" basically and in parent fork() returns child's process ID or PID which should be > 0 saying "im the parent". 
If error occurred and no child was made fork() returns -1.
pid_t is just an int type for holding PIDs

we need fork() because if we run a separate program in shell like "ls" we want a separate process while our original shell prompt stays alive to prompt again. So fork()
makes a child to run "ls" and after its done it finishing and we get another prompt. Otherwise, if we don't got that child process the prompt becomes the "ls" program and shell disappears
after "ls" ends.

using execvp():
if called such as "execvp("ls", args)" the os replaces the child's program with "ls" its the same process but just with a new program.
So, if execvp succeeds, it never returns, since the code that called it no longer exists since it got overwritten in this case by "ls". SO any line after a successful execvp only runs if
execvp failed eg. the program doesn't exist. This is good for handling "command not found".

a small case for execvp is that it needs the command split into an array of words and not one string.
so when we type in "ls -l" getline actually hands us a single string "ls -l\n" but this wont work with execvp and wants it as separate pieces "["ls", '-l", NULL] which is the
program name, then each argument after, the na NULL to mark the end. This is why need strtok to chop the input line words first.

using strtok(): declared by string.h
string tokenizer breaks a string into pieces or "tokens" wherever it finds separators and for us that will be spaces and newlines.
So in our code the first call "strtok(line, " \n") this is to pass the string and the separators. Returns a pointer to the first word.
later on "strtok(NULL, " \n") passes NULL (it remembers where it left off) Returns the next word each time
so then when there are no more words it returns NULL to end

using waitpid: declared by sys/wait.h

using perror:
prints an error message that describes what went wrong since it reads the systems error info. in our case would be like "fork: Cannot allocate memory"


key concept of cd:
command cd cannot be used with fork() since it will change its own directory.
meaning for example. "cd /tmp" the CHILD changes its own directory to /tmp meanign that when the CHILD disappears and finishes, the PARENT (the og shell) is waiting 
and waiting and is still sitting in the original directory. So the key concept here is that the CHILD cannot reach back and move its PARENT. So "cd" has to be handled in the og shell
itself and directly calls chdir() to change its own directory.

strcmp(args[0], "cd") == 0 is to compare two strings and it reutns 0 when it is equal. strcmp returns the difference which is 0 for identical strings

since the criteria wants cd ~ to work where ~ means home directory the idea is if the argument starts with ~, we can replace that ~ with the value of $HOME.
So essentially ~ becomes /home/carlmontescalros and basically ~/CSC360 becomes /home/carlmontesclaros/CSC360

snprintf -> writes into string buffer isntead of the screen. needed for this line "snprintf(target, sizeof(target), "%s", ...)" which fills target using the same "%s"
so the sizeof(target) tells it the buffer size so it never overflows



background job concepts/process:
bg <command> 1.1.1.1 runs <command> in the background and the shell does not wait and gives us back to new prompt right away.
bg list -> should list all the background jobs taht are still running
if a background job does finish the shell should tell us <pid>: <command> has terminated when we press enter the next time.

the process is similar to foreground commands we just implemented such as fork, exec, wait but the difference is that the parent does not wait. for background job the parent forks launches he program then loops back to the prompt immediately

the hardest part about back ground is that we have to remember the jobs and with the specification in mind where bglist shows all current running jobs and when one finishes it should announce in the shell prompt. for the specs it can be theoreticallty unlimited amount of jobs therefore, cannot use arrays and variables must be using linked lists for the structure.
we will have one pointer called head that points at the first box or job. and we will move onto the next pointer onto the next box or job until we hit NULL. Adding a new job we just create a new node and link to it and to remove a finished job we just unlink and free the memory.

for this we need to use MALLOC and FREE as we need each node to persist and live past single loop iteration. EG. we the job that are still running to wstill be listed by bglist 10 prompts later.

steps by step 1. when bg launches something we will malloc a new node and fill in the PID and command and link it into the list. 2. bglist will walk the list form head to every node after and print each node. 3. we will need  a way to terminate finished jobs in the background. to check if any background child has finished we need to use waitpid wit the WNOHANG option to “check, but dont block if nobody is done” afterwards have the print “has terminated and remove that node from the list.

PHASE 1: node type and three list pieces

step 1: defining the nodes
ideas are using “struct bg_node { ... }” we need ot use struct to bundle several pieces of data as one unit. I will bundle the PID and command string and the next pointer into this struct

struct bg_node *next this line contains a pointer to its own type and that next pointer is changing the boxes together

using typedef “typedef ... bg_node” to create a shorthand name so we dont have to write out struct bg_node everytime

as the head is of list is line bg_node *bg_head = NULL; its just set as empty but we have to put this outside the main since it will need ot persist across loop iterations and to be reachable anywhere

step 2: for //bg block of code
execvp(args[1], &args[1]) ie. bg ping 1.1.1.1 since bg si argos[0] and args[1] is ping we want to run ping and not bg and we set the program array to start at &args[1] which is that pointer trick handing execvp a view of the array starting one slot in.

malloc(sizeof(bg_node)) so we ask the OS to give us exact amount of memory to hold one bg_node box. Returns a pointer to that memory and this memory persists until we free it as said above

node->pid = pid line is to access a field through a pointer and node is pointer to the box and we want to access pid

the strcat(dest, src) append src onto the end of dest. since we start with empty string command[0] = '\0', we then glue on each word with spaces between rebuilding “ping 1.1.1.1” therefore bglist can display it on later

node->next = bg_head; bg_head = node this line is to insert at the font of the list
node->next = bg_head; point the new node next at whatever was previously first
bg_head = node now makes the new node the head

step 3 handing bglist:
we need to use a pattern for this the walk pattern
we start current = bg_head to begin at the first box
loop while current != NULL so we kee going until fall of the end
and each step current must be modified then current = current->next so that it hops onto the next box. essentially following the chain of boxes/nodes and printing their PID and string commands

PHASE 2 want to detect when a job finishes
in spec of assignment it must be announced <pid>: <command> has terminated. the next time enter is pressed
a key concept is that when a child finishes the OS keeps a record of it around until the parents “collects” the exit status with waitpid, but a finished but uncollected child is a called a zombie process which could pile up and bloat the OS so it must be handled by a specific waitpid command with WNOHANG.

a special way to call it is waitpid(-1, &status, WNOHANG) where hte -1 essentially means “any child, i dont care which” and this call returns a positive PUD if some child finished and tells us which child it is, 0 if children are still running but none have finished, -1 if there are no children at all. Essentially we want to loop this and keep collecting finished children processes until there is none left to collect

helper function idea:
“while ((pid = waitpid(...)) > 0)” outer while loops calls waitpid and checks its result in one line. we have it so it assigns the return to pid then tests if pid is > 0 therefore the loop stops if waitpid returns 0 if nothings finished or -1 there are no children. This is good as it collects all currently finished jobs in one sweep which is good since there could be multiple jobs that had been finished since last prompt
when unlinking the node and to remove a box from the chain we make the previous node’s next skip over it and point to the one after
so before: previous----> current -----> after
and after: previous -----------------------> after and current is unlinked
there is a special case and if that node we want to remove is he first one previous == NULL so we never passed the head. there is no previous node so we instead move “bg_head” to itself to point at the next node

after implementation test:
bg sleep 5 and bglist showed it running and then after it finished the next enter it says sleep  has been terminated so the temrination and detection works. we also ran bglist again and it says that the total background jobs was 0. however a bug i stumbled in the code was when accidentally typing “bglist” as “bg list” and my shell ran that as a command called list and forked it. In my shell it said that it failed but it still added list to the job list though it never ran and failed.
***add this to readme file as a minor cosmetic bug
   
Last feature: Ctrl-C signal handling

the concept of CTRL-C signal handling is that if we have a long foreground program running for example like ping 1.1.1.1 and if we press CTRL-C it sends a SIGINT or interrupt signal. the default behavior is that the signal kills whatever process receives the signal so if this gets sent to the whole process group like the ping child and the shell the signal form CTRL-C would kill ping and the shell and puts us back in bash. I want to fix this so that my preferred behavior is that it only kills the foreground program but leave the shell running.

so my fix should 1. the shell ignores SIGINT so CTRL-C doesnt kill the shell. A standard approach is by installing a signal handler which is a small function that runs when the SIGINT signal arrives. 2. we will still have the foreground child running to be termianted by the signal

key idea is when using fork, the child also inherits the shell’s signal settings so if we set the shell to ignore SIGINT the child would also inherit that and not die as well. So to fix this issue in the child right before execvp, reset SIGINT back to default so that the child gets terminated on CTRL-C while parent or shell ignores it

for signal handling we need <signal.h> header
signal(which_signal, what_to_do) using this we say that SIGINT CTRL-C signal and SIG_IGN to ignore that signal so the shell shrugs off the signal instead of dying.

now in where we fork and the child runs execvp(args[0], args) before that line, we want to add the same code above however, using signal(SIGINT, SIG_DFL) where SIG_DFL means keep this CTRL C signal in default behavior.

also need to do the line above since our bg path also forks therefore needing the “signal(SIGINT, SIG_DFL)” default signal behavior for the bg child.

feature properly implemented. Tested using ping 1.1.1.1 and when pressing CTRL-C program was terminated, and used ls in the shell to see if it works and it did.

*small detail to fix -> pressing CTRL C on empty prompt just gives us ^C polish for behavior of making a new line instead.,


IDEA solution for CTRL C making new line instead of ^C on empty prompts
instead of using SIG_IGN to completely ignore the signal instead write a small handler function to be directed to when SIGINT arrives. Basically tell OS to run this function instead of the default kill.
*IMPORTANT -> signal handler interrupts programs at unpredictable moments so CTRL C SIGINT signal could come when the shell is in middle of printf or mid malloc and this is dangerous because if the interrupt happens while printf or malloc is running in main code, it can corrupt things. SO, ensure that when the signal comes the handler function needs to run when SAFE specific list of functions like write. printf is not safe.

essentially to fix this we need to make this handler function very simple and bare minimum. will just have the handler print a newline using write since it is safe.

ideas for this solution
void sigint_handler(int signum) -> we just take one int which is SIGINT so we just ignore it
(void)signum; signum needed in the signature but when compiling there may be some warning since we dont use so added (void) to essentially say ignore this.
the special signal safe way to write an output “write(STDOUT_FILENO, "\n", 1)” write takes (where to write which is STDOUT_FILENO is the screen value 1, the string to write which is “\n” new line, and how many bytes which is 1).
also need to replace the old SIGINT solution so replaced the signal(SIGINT, SIG_IGN) to  signal(SIGINT, signal_handler) which is the handler solution on top of the main.

problem ran into -> when SIGINT interrupts the getline, it returns -1 which looks like EOF and set me a special error code errno to EINTR. turned out that code treated -1 as CTRL D which exits the shell so CTRL C exits my shell so I need to differentiate this CTRL C from CTRL D.

problem solution -> adjusted EOF using #include <errno.h> header
just had to change  
if (nread == -1) {
                        printf("\n");
                        break;
                }
to 
 if (nread == -1) {
                        if (errno == EINTR) {
                                clearerr(stdin); //essentially saying it was CTRL C signal reset + reprompt
                                continue;
                        }
                        printf("\n");
                        break; //this the real EOF CTRL D exit
                }
since if getline returned -1 because of a SIGINT signal we use clearerr(stdin) to reset the error flag and continue to loop back to fresh prompt so only CTRL D a real EOF falls through break and exits.

fire it works well. test concluded 
ping 1.1.1.1 + CTRL C -> ping terminated shell works
CTRL C on empty prompt created new fresh prompts on a new line and reloops
CTRL D exits the shell

SHELL PROJECT COMPELTE
