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
static char *args[1024];
static char prompt[1024];
char *execute[1024];
char cwd[2048];
char path[2048];
int fd;
char *buffer;
int state, len;
pid_t pid;
int environmment_flag;
int redirect_flag;
int status;
char *redirection_file;

void tokeniser(char *);
void tokenize_redirect_output(char *);
static int execute_command(char *, int, int, int);

// this function initializes the global variables
void init()
{

	fd = 0;			   // file descriptor
	state = 0;		   // state of the input string
	redirect_flag = 0; // status of redirect
	cwd[0] = '\0';
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

// This function is used to skip the white spaces in the input string.
// It returns the pointer to the first non-white space character.
// It is needed to implement redirection.
char *skipwhite(char *str)
{

	int i = 0;
	int j = 0;
	char *temp_state; // this is used to store the state of the input string

	if (NULL == (temp_state = (char *)malloc(sizeof(str) * sizeof(char)))) // allocating memory to temp_state
	{
		return NULL;
	}

	while (str[i++])
	{

		if (str[i - 1] != ' ')
			temp_state[j++] = str[i - 1];
	}
	temp_state[j] = '\0';
	return temp_state;
}

// change_directory function is used to implement the cd command
// in my shell. It makes a system call and uses the c-function
// chdir to change the directory.
void change_directory()
{
	int i = chdir(args[1]);
	if (i == 0)
	{
		getcwd(path, sizeof(path));					 // get the current working directory
		printf("Directory changed to : %s\n", path); // print the current working directory
	}
	else if (i < 0)
		perror("There is no such file or directory there . . ."); // print error message
}

// this function in combination with the next is the crux of the
// basic shell. It takes the input from the user and tokenizes it
// for further processing.
static int execute_command(char *execute, int input, int first, int last)
{

	int mypipefd[2], ret, input_fd, output_fd; // mypipefd is used to create a pipe

	if (-1 == (ret = pipe(mypipefd)))
	{
		perror("pipe error: "); // print error message if pipe is not created
		return 1;
	}

	pid = fork(); // fork a child process

	if (pid == 0)
	{

		if (first == 1 && last == 0 && input == 0)
		{

			dup2(mypipefd[1], 1); // redirects only the output to the pipe
		}
		else if (first == 0 && last == 0 && input != 0)
		{

			dup2(input, 0);		  // redirect the input to the pipe
			dup2(mypipefd[1], 1); // redirect the output to the pipe
		}
		else
		{

			dup2(input, 0); // redirects only the input to the pipe
		}

		if (strchr(execute, '>'))
		{
			redirect_flag = 1;				   // set the redirection flag
			tokenize_redirect_output(execute); // tokenize the redirection output
		}

		if (redirect_flag)
		{

			if ((output_fd = creat(redirection_file, 0644)) < 0)
			{

				fprintf(stderr, "Failed to open %s for writing\n", redirection_file); // print error message if file is not created
				return (EXIT_FAILURE);												  // return failure
			}
			dup2(output_fd, 1); // redirect the output to the file
			close(output_fd);	// close the file
			redirect_flag = 0;	// reset the redirection flag
		}

		if (execvp(args[0], args) < 0)
		{
			fprintf(stderr, "%s: Command not found\n", args[0]); // print error message if command is not found
		}
		exit(0); // exit the child process
	}

	else
	{

		waitpid(pid, 0, 0); // wait for the child process to finish
	}

	if (last == 1)
		close(mypipefd[0]); // close the pipe if the last command is executed

	if (input != 0)
		close(input); // close the input file descriptor

	close(mypipefd[1]);	  // close the pipefd[1] if the last command is executed
	return (mypipefd[0]); // return the file descriptor
}

// This function is used to execute the inbuilt commands. It also calls
// the "execute_command" function when the command to be executed doesn't
// fall under inbuilt commands.
static int inbuilt(char *execute, int input, int isfirst_flag, int islast_flag)
{

	char *execute_duplicate;

	execute_duplicate = strdup(execute); // duplicate the input string

	tokeniser(execute); // tokenize the input string

	if (args[0] != NULL)
	{
		if (!(strcmp(args[0], "exit") && strcmp(args[0], "quit"))) // check if the command is exit or quit
			exit(0);											   // exit the shell

		if (!strcmp("cd", args[0])) // check if the command is cd

		{

			change_directory(); // call the change_directory function
			return 1;
		}
	}
	return (execute_command(execute_duplicate, input, isfirst_flag, islast_flag)); // call the execute_command function
}

// This function is used to parse the input when only output redirection
//[">"] is present
void tokenize_redirect_output(char *execute)
{
	char *val[128];						 // array to store the tokens
	char *execute_duplicate, *s1;		 // variables to store the input and the duplicate input
	execute_duplicate = strdup(execute); // duplicate the input string

	int m = 1;								 // variable to store the index of the array
	val[0] = strtok(execute_duplicate, ">"); // tokenize the input string
	while ((val[m] = strtok(NULL, ">")) != NULL)
		m++; // store the tokens in the array

	s1 = strdup(val[1]);
	redirection_file = skipwhite(s1);

	tokeniser(val[0]); // tokenize the input string
	return;
}

// this function tokenizes the input string based on white-space [" "]

void tokeniser(char *str)
{

	int m = 1; // variable to store the index of the array

	args[0] = strtok(str, " ");					  // tokenize the input string
	while ((args[m] = strtok(NULL, " ")) != NULL) // store the tokens in the array
		m++;									  // increment the index of the array
	args[m] = NULL;								  // set the last element of the array to NULL
}

/* This function tokenizes the input string based on pipe ["|"] */

void pipe_tokeniser()
{

	int i;
	int m = 1;
	int input = 0; // variable to store the input file descriptor
	int first = 1;

	execute[0] = strtok(buffer, "|"); // tokenize the input string
	while ((execute[m] = strtok(NULL, "|")) != NULL)
		m++; // increment the index of the array

	execute[m] = NULL; // set the last element of the array to NULL

	for (i = 0; i < m - 1; i++)
	{

		input = inbuilt(execute[i], input, first, 0); // call the inbuilt function
		first = 0;									  // set the first flag to 0
	}

	input = inbuilt(execute[i], input, first, 1); // call the inbuilt function defined above
	return;
}

/* Main function begins here */

int main()
{

	int command_status;
	char new_line = 0;

	do
	{

		init();			   // call the init function to initialise the global variables
		prompt_function(); // call the prompt_function function to print the prompt
		buffer = readline(prompt);

		if (!(strcmp(buffer, "") && strcmp(buffer, "\n")))
			continue;

		if (!(strncmp(buffer, "quit", 4) && strncmp(buffer, "exit", 4)))
		{

			state = 1;
			break;
		}

		pipe_tokeniser();

		waitpid(pid, &command_status, 0);

	} while (!WIFEXITED(command_status) || !WIFSIGNALED(command_status));

	if (state == 1)
	{

		printf("\nExiting Shell . . .\n");
		exit(0);
	}

	return 0;
}
