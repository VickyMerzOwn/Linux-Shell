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
int fd;
static char *args[1024];
static char prompt[1024];
char *buffer;
char *execute[1024];
char path[2048];
int state;
pid_t pid;
int status;

void tokeniser(char *);
static int execute_command(char *, int, int, int);

// This function initializes the global variables
void init()
{

	fd = 0;			  // file descriptor
	state = 0;		  // state of the input string
	prompt[0] = '\0'; // prompt
	pid = 0;		  // process id
}

// This function is used to create the Shell Prompt
// Escape characters have been used to get a colorful prompt
// similar to my customised bash shell.
void prompt_function()
{

	strcpy(prompt, "\033[1;31msatvik_vemuganti\033[0;37m@\033[0;32m12041710 \033[0;35m(^_^)\033[0;33m$\033[0;37m ");
}

// change_directory function is used to implement the cd command
// in my shell. It makes a system call and uses the c-function
// chdir to change the directory.
void change_directory()
{
	int i = chdir(args[1]); // system call to change the directory
	if (i == 0)				// if the directory is changed successfuly
	{
		getcwd(path, sizeof(path));
		printf("Directory changed to : %s\n", path);
	}
	else if (i < 0)
		perror("There is no such file or directory there . . .");
}

// this function in combination with the next is the crux of the
// basic shell. It takes the input from the user and tokenizes it
// for further processing.
static int execute_command(char *execute, int input, int first, int last)
{

	int pipe_FD[2], ret; // pipe_FD is used to create a pipe

	if (-1 == (ret = pipe(pipe_FD))) // if pipe is not created
	{
		perror("pipe error: "); // print error message
		return 1;
	}

	pid = fork(); // fork a child process

	if (pid == 0)
	{

		if (first == 1 && last == 0 && input == 0) // if the input is not redirected
		{

			dup2(pipe_FD[1], 1); // redirect the output to the pipe
		}
		else if (first == 0 && last == 0 && input != 0) // if the input is redirected
		{

			dup2(input, 0);		 // redirect the input to the pipe
			dup2(pipe_FD[1], 1); // redirect the output to the pipe
		}
		else
		{

			dup2(input, 0); // redirect the input to the pipe
		}

		if (execvp(args[0], args) < 0)
		{
			fprintf(stderr, "%s: Command not found\n", args[0]); // print error message if the command is not found
		}
		exit(0);
	}

	else
	{

		waitpid(pid, 0, 0); // wait for the child process to finish
	}

	if (last == 1)
		close(pipe_FD[0]); // close the pipe

	if (input != 0)
		close(input); // close the input file descriptor

	close(pipe_FD[1]);	 // close the output file descriptor
	return (pipe_FD[0]); // return the input file descriptor
}

// This function is used to execute the inbuilt commands. It also calls
// the "execute_command" function when the command to be executed doesn't
// fall under inbuilt commands.
static int inbuilt(char *execute, int input, int isfirst_flag, int islast_flag)
{

	char *execute_duplicate; // duplicate of the execute string

	execute_duplicate = strdup(execute); // duplicate the execute string

	tokeniser(execute); // tokenize the execute string

	if (args[0] != NULL) // if the command is not null
	{

		if (!strcmp("cd", args[0])) // if the command is cd
		{

			change_directory(); // call the change_directory function
			return 1;
		}

		if (!(strcmp(args[0], "exit"))) // if the command is exit
			exit(0);					// exit the shell
	}
	return (execute_command(execute_duplicate, input, isfirst_flag, islast_flag)); // call the execute_command function
}

// this function tokenizes the input string based on white-space [" "]

void tokeniser(char *str)
{

	int m = 1; // m is used to store the index of the token

	args[0] = strtok(str, " ");					  // tokenize the input string
	while ((args[m] = strtok(NULL, " ")) != NULL) // while the token is not null
		m++;									  // increment the index
	args[m] = NULL;								  // set the last token to null
}

// this function tokenizes the input string based on pipe delimiter ["|"]

void pipe_tokeniser()
{

	int i;		   // i is used to store the index of the token
	int n = 1;	   // n is used to store the index of the pipe
	int input = 0; // input is used to store the input file descriptor
	int first = 1; // first is used to store the first flag

	execute[0] = strtok(buffer, "|");				 // tokenize the input string
	while ((execute[n] = strtok(NULL, "|")) != NULL) // while the token is not null
		n++;										 // increment the index

	execute[n] = NULL; // set the last token to null

	for (i = 0; i < n - 1; i++) // for loop to execute the commands
	{

		input = inbuilt(execute[i], input, first, 0); // call the inbuilt function
		first = 0;									  // set the first flag to 0
	}

	input = inbuilt(execute[i], input, first, 1); // call the inbuilt function
	return;
}

// main function
int main()
{

	int command_status; // command_status is used to store the status of the command

	do
	{

		init();					   // call the init function to initialise the global variables
		prompt_function();		   // call the prompt_function function to create the prompt
		buffer = readline(prompt); // read the input from the user

		if (!(strcmp(buffer, "") && strcmp(buffer, "\n"))) // if the input is empty
			continue;									   // continue the loop

		if (!(strncmp(buffer, "quit", 4) && strncmp(buffer, "exit", 4))) // if the input is quit or exit
		{

			state = 1; // set the state to 1
			break;	   // break the loop, exit the shell
		}

		pipe_tokeniser(); // call the pipe_tokeniser function to tokenize the input string

		waitpid(pid, &command_status, 0); // wait for the child process to finish

	} while (!WIFEXITED(command_status) || !WIFSIGNALED(command_status)); // while the child process is not terminated

	if (state == 1) // if the state is 1
	{

		printf("\nExiting Shell . . .\n"); // print the message and exit the shell
		exit(0);						   // exit the shell
	}

	return 0;
}
