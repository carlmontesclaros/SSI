#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //standard for getcwd, fork, exec, getlogin, gethostname, etc....
#include <string.h> //for strings standard header
#include <sys/wait.h> //
#include <signal.h> // for signal handling of CTRL-C
#include <errno.h>

// one background job in linked list
typedef struct bg_node {

	pid_t pid; //PID of the background job
	char command[1024]; //the command text for prompt "bg list, etc..."
	struct bg_node *next; //pointer to the next job

} bg_node;
bg_node *bg_head = NULL; // the head node and head of list of jobs (frist job) NULL means its empty right now


//helper funtion for checking finished background jobs + announce and remove from list
void check_background_jobs(void) {

	int status;
	pid_t pid;

	//loop of collecting child that has finished using WNOHANG
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		//finding pid in list to print pid and command
		bg_node *current = bg_head;
		bg_node *previous = NULL;

		while (current != NULL) {
			if (current->pid == pid) {
			//announcing terminated child
			printf("%d: %s has terminated.\n", current->pid, current->command);

			//unlinking node from list
			if (previous == NULL) { // special case to check if we moved past head node
				bg_head = current->next;
			} else {
				previous->next = current->next;
			}

			free(current); // to give memory back
			break;
		}

		previous = current;
		current = current->next;

		}
	}
}

// signal handling function so that CTRL C makes new line very minimal
void sigint_handler(int signum) { //one int  which is always SIGINT -> ignore

	(void)signum; //dont use arg to silence the warning
	write(STDOUT_FILENO, "\n", 1); //safe way of printing new line

}


int main(void) {

	char *line = NULL;
	size_t len  = 0;

	char cwd[1024]; //this is for the directory fixed size arrays buffers
	char hostname[256]; //as name says for hostname fixed size array buffers

	signal(SIGINT, sigint_handler); //tells signal SIGINT CTRL-C and SIG_IGN to ignore the signal
	while (1) {

		check_background_jobs(); //so that next time we hit enter the helper function announces
		char *username = getlogin(); //getlogin for username
		gethostname(hostname, sizeof(hostname)); //sizeof so we dont corrupt memory
		getcwd(cwd, sizeof(cwd)); //same sizeof avoiding memory bugs

		printf("%s@%s: %s > ", username, hostname,  cwd);
		fflush(stdout);

		ssize_t nread = getline(&line, &len, stdin);

		if (nread == -1) {
			if (errno == EINTR) {
				clearerr(stdin); //essentially saying it was CTRL C signal reset + reprompt
				continue;
			}
			printf("\n");
			break; //this the real EOF CTRL D exit
		}

		// to split the line into an array of words or tokens
		char *args[64]; //array of 64 string pointers. each slot will point to one word from input. 64 a generous cap on how many words a command can have. this is for execvp
		int count = 0;
		char *token = strtok(line, " \n");
		while (token != NULL && count < 63) { //count < 63 to guard from overflowing leave one for NULL to signal end

			args[count] = token; //each  word found gets stored in args[count]
			count++; // increases count size so we can make sure we know we can use that count < 63 from overflow
			token = strtok(NULL, " \n"); // when strtok returns NULL loop ends due to parameter above
		}

		args[count] = NULL; //so that execvp can figure out where the arguement list ends by looking for NULL

		// if user just presses enter with no words to skip
		if (count == 0) continue;

		//this for cd
		if (strcmp(args[0], "cd") == 0) { //compares 2 strings and retuns 0 if they are identical
			char target[1024]; //target into buffer needed for the tilde case since we combine two strings home + other arguments
			char *home = getenv("HOME");

			if (args[1] == NULL) { //since args[0] is cd then args[1] == NULL and just detects no arguement is given
				// no arguement just go back to home
				snprintf(target, sizeof(target), "%s", home);

			} else if (args[1][0] == '~') { //args[1] is argument string, args[1][0] is its first character essentully asks if it starts with tilde ~
				//expand a leading ~ into home and keey the rest
				snprintf(target, sizeof(target), "%s%s", home, args[1] + 1); //args[1] points at the start which is the ~ and adding 1 move pointer forward one character pointing at "/CSC360" so just after the tilde

			} else {
				snprintf(target, sizeof(target), "%s", args[1]);
			}

			if (chdir(target) != 0) { // attempts the change and if it returns nonzero the perror prints why

				perror("cd");

			}

			continue; //is important after handling cd to jump back to top of the loop otherwise code would try fork below

		}

		//for bg running programs in background
		if (strcmp(args[0], "bg") == 0) {
			if (args[1] == NULL) {
				printf("bg: missing command\n");
				continue;
			}

			pid_t pid = fork();

			if (pid < 0) {
				perror("fork");
			} else if (pid == 0) {

				signal(SIGINT, SIG_DFL); //default signal behavior to kill process for CHILD
				//we are in CHILD and run the program
				execvp(args[1], &args[1]); // skips "bg" since we want to run ping eg "bg ping 1.1.1.1"
				printf("<%s>: No such file or directory\n", args[1]);
				exit(1);

			} else {

				//we are in PARENT we dont wait and record job in the list
				bg_node *node = malloc(sizeof(bg_node)); //asks the os for exact amount of memory of bg node
				node->pid = pid; //accesses pid field in node

				//building command string from args[1] onwards
				node->command[0] = '\0';
				for (int i = 1; args[i] != NULL; i++) {

					strcat(node->command, args[i]);
					if (args[i + 1] != NULL) {
						strcat(node->command, " ");
					}
				}

				//link new node to front of the list
				node->next = bg_head; //points new nodes next at wahtever was perviosuly first
				bg_head = node; // make the new node the head
			}
			continue;
		}

		// for bglist to show all running background jobs walks list and prints each job
		if (strcmp(args[0], "bglist") == 0) {

			int count = 0; //to keep track of number of jobs as we print the list
			bg_node *current = bg_head; //the head to begin at first box

			while (current != NULL) { //to keep going until we find NULL or end

				printf("%d: %s\n", current->pid, current->command);
				count++;
				current = current->next; //follow the chain to hop on the next node
			}

			printf("Total Background jobs: %d\n", count);
			continue;

		}

		// forking a child to run the command
		pid_t pid = fork(); //initializing pid for fork()

		if (pid < 0) {

			perror("fork error"); //describes what went wrong

		} else if (pid == 0) {

			signal(SIGINT, SIG_DFL); //so that the forked child still gets terminated by CTRL C signal
			//is CHILD and become the requested program
			execvp(args[0], args); //args[0] is program name which is in CHILD and args is the whole array p in execvp searches for  the file in your path for the program so u are able ot say ls instead of full /bin/ls
			//only reached if execvp failed
			printf("<%s>: No such file or directory\n", args[0]);
			exit(1); //if execvp fails CHILD must terminate otherwise the CHILD loops back and starts prompting as well
		} else {

			// we in the PARENT and need for child to finish doing its business
			int status;
			waitpid(pid, &status, 0); //parents pauses here until chidl with pid finishes and &status records how the child ended. 0 means normal wait block until done
		}

	}

	free(line);
	return 0;

}

