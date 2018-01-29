// Shell starter file
// You may make any changes to any part of this file.
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH + 3];
int cmd_count = 0;

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if (length < 0 && errno != EINTR) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}
	else if (length < 0 && errno == EINTR){
		read_command(buff, tokens, in_background);
		return;
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

void add_command(char* tokens[], int in_background)
{
	int i = 0;
	cmd_count++;
	strcpy(history[cmd_count%10], "");
	while(tokens[i] != NULL){
		strcat( history[cmd_count%10], tokens[i] );
		strcat( history[cmd_count%10], " " );
		i++;
	}
	if (in_background){
		strcat(history[cmd_count%10], "&");
	}
}

void retrieve_command()
{
	char out_str[COMMAND_LENGTH + 5];
	int num;
	if(cmd_count <= 10){
		for(num = 1; num <= cmd_count; num++){
			sprintf(out_str, "%d\t %s\n", num, history[num%10]);
			write(STDOUT_FILENO, out_str, strlen(out_str));
		}
	}
	else{
		for(num = cmd_count - 9; num <= cmd_count; num++){
			sprintf(out_str, "%d\t %s\n", num, history[num%10]);
			write(STDOUT_FILENO, out_str, strlen(out_str));
		}
	}
	
}

void get_command(char* buf, int num)
{
	strcpy(buf, history[(num)%10]);
}

void set_command(char* str, int num)
{
	strcpy(history[(num)%10], str);
}

void handle_SIGINT()
{
	char cwd[1000];
	getcwd(cwd, 1000);
	write(STDOUT_FILENO, "\n", strlen("\n"));
	if(cwd == NULL){
		write(STDERR_FILENO, "path name exceeds limit\n", strlen("path name exceeds limit\n"));
	}
	retrieve_command();
	write(STDOUT_FILENO, strcat(cwd, "> "), strlen(strcat(cwd, "> ")));
}


/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	while (true) {
		char cwd[1000];
		getcwd(cwd, 1000);
		if(cwd == NULL){
			write(STDERR_FILENO, "path name exceeds limit\n", strlen("path name exceeds limit\n"));
		}

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		write(STDOUT_FILENO, strcat(cwd, "> "), strlen(strcat(cwd, "> ")));;
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);
		if(tokens[0] == NULL){
			continue;
		}

		if(tokens[0][0] == '!'){
			int cmd_num;
			if(tokens[0][1] == '!'){
				cmd_num = cmd_count;
			}
			else{
				cmd_num = atoi(tokens[0]+1);
			}	
			if (cmd_num <= 0 || cmd_num > cmd_count || cmd_num < cmd_count-9){
				write(STDERR_FILENO, "Invalid command number\n", strlen("Invalid command number\n"));
				continue;
			}
			get_command(input_buffer, cmd_num);
			write(STDOUT_FILENO, input_buffer, strlen(input_buffer));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			int token_count = tokenize_command(input_buffer, tokens);
			if (token_count == 0) {
				continue;
			}
			if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
				in_background = true;
				tokens[token_count - 1] = 0;
			}
		}

		add_command(tokens, in_background);

		while(waitpid(-1, NULL, WNOHANG) > 0);

		if(strcmp(tokens[0], "exit") == 0)
			exit(0);
		else if(strcmp(tokens[0], "pwd") == 0){
			getcwd(cwd, 1000);
			if(cwd == NULL){
				write(STDERR_FILENO, "path name exceeds limit\n", strlen("path name exceeds limit\n"));
			}
			else{
				write(STDOUT_FILENO, cwd, strlen(cwd));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}	
		}
		else if(strcmp(tokens[0], "cd") == 0){
			if (chdir(tokens[1]) == -1){
				write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
		}
		else if(strcmp(tokens[0], "history") == 0)
			retrieve_command();
		else{
			pid_t pid;
			pid = fork();
			if(pid == 0){
				execvp(tokens[0], tokens);
				return 0;
			}
			else if(pid > 0){
				if(!in_background)
					waitpid(pid, NULL, 0);
			}
			else{
				write(STDERR_FILENO, "fork failed\n", strlen("fork failed\n"));
			}			
		}


		// DEBUG: Dump out arguments:write(STDOUT_FILENO, "\n", strlen("\n"));
		// for (int i = 0; tokens[i] != NULL; i++) {
		// 	write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
		// 	write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
		// 	write(STDOUT_FILENO, "\n", strlen("\n"));
		// }
		// if (in_background) {
		// 	write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		// }

		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

	}
	return 0;
}
