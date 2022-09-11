// headers
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// global variable declarations
static char *args[1024];  // this is used to store the arguments of the input string
static char prompt[1024]; // this is used to store the prompt
char *execute[1024];
char *buffer;	// this is used to store the input string
int state;		// this is used to store the state of the input string
char cwd[2048]; // this is used to store the current working directory
pid_t pid;		// this is used to store the process id

void tokeniser(char *);		  // this is used to tokenize the input string
static int execute_command(); // this is used to execute the input string

// This function initializes the global variables
void init()
{
	state = 0; // state of the input string
	cwd[0] = '\0';
	prompt[0] = '\0'; // prompt
	pid = 0;		  // process id
}

// This function is used to create the Shell Prompt
// Escape characters have been used to get a colorful prompt
// similar to my customised bash shell.
void prompt_function()
{
	strcpy(prompt, "\033[1;31msatvik_vemuganti\033[0;37m@\033[0;32m12041710 \033[0;35m(^_^)\033[0;33m$\033[0;37m "); // this is the prompt
	return;
}

// change_directory function is used to implement the cd command
// in my shell. It makes a system call and uses the c-function
// chdir to change the directory.
void change_directory()
{
	char path[1024];		// this is used to store the path
	int i = chdir(args[1]); // system call to change the directory
	if (i == 0)				// if the directory is changed successfuly
	{
		getcwd(path, sizeof(path));					 // get the current working directory
		printf("Directory changed to : %s\n", path); // print the current working directory
	}
	else if (i < 0)												  // if the directory is not changed
		perror("There is no such file or directory there . . ."); // print the error message
}

// this function in combination with the next is the crux of the
// basic shell. It takes the input from the user and tokenizes it
// for further processing.

static int execute_command()
{

	pid = fork(); // fork the process

	if (pid == 0) // if the process is child process
	{
		if (execvp(args[0], args) < 0)
		{
			fprintf(stderr, "%s: Command not found\n", args[0]);
		}
		exit(0);
	}

	else
	{
		waitpid(pid, 0, 0);
	}

	return 1;
}

// This function is used to execute the inbuilt commands. It also calls
// the "execute_command" function when the command to be executed doesn't
// fall under inbuilt commands.

static int inbuilt(char *execute)
{

	char *execute_duplicate; // this is used to store the duplicate of the execute string

	execute_duplicate = strdup(execute); // duplicate the execute string

	tokeniser(execute); // tokenize the execute string

	if (args[0] != NULL)
	{

		if (!strcmp("cd", args[0])) // if the command is cd
		{

			change_directory(); // call the change_directory function
			return 1;
		}

		if (!(strcmp(args[0], "exit"))) // if the command is exit
			exit(0);					//	exit the shell
	}
	return (execute_command()); // call the execute_command function
}

// this function tokenizes the input string based on white-space [" "]

void tokeniser(char *str)
{

	int m = 1; // this is used to store the index of the args array

	args[0] = strtok(str, " ");					  // tokenize the input string
	while ((args[m] = strtok(NULL, " ")) != NULL) // tokenize the input string
		m++;									  // increment the index of the args array
	args[m] = NULL;								  // set the last element of the args array to NULL
}

// main function
int main()
{

	int command_status; // this is used to store the status of the command

	do // do-while type c-loop
	{

		init();											   // initialize the global variables
		prompt_function();								   // call the prompt_function function
		buffer = readline(prompt);						   // read the input string
		if (!(strcmp(buffer, "") && strcmp(buffer, "\n"))) // if the input string is empty
			continue;									   // continue the loop

		if (!(strncmp(buffer, "quit", 4) && strncmp(buffer, "exit", 4))) // if the input string is quit or exit
		{

			state = 1; // set the state of the input string
			break;	   // break the loop
		}

		inbuilt(buffer); // call the inbuilt function

		waitpid(pid, &command_status, 0); // wait for the child process to finish

	} while (!WIFEXITED(command_status) || !WIFSIGNALED(command_status)); // do-while loop

	if (state == 1)
	{

		printf("\nExiting shell . . .\n"); // print the message
		exit(0);						   // exit the shell
	}

	return 0;
}
