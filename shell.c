#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_BUFSIZE 64

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char* builtin_cmds[] = {"cd", "help", "exit"};

int (*builtin_funcs[]) (char **) = {&shell_cd, &shell_help, &shell_exit};

int builtin_func_count() {
  return sizeof(builtin_cmds) / sizeof(char *);
}


/**
 * Change directory shell functionality. 
 */
int shell_cd(char** args) {
    if(args[1] == NULL) {
        fprintf(stderr, "Missing expected argument to \"cd\".\n");
    } else {

        // Change the directory with chdir().
        if(chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}


/**
 * Executes the given arguments.
 */
int launch(char** args) {
    pid_t pid, wpid;
    int status;

    // Split the current process into 2 concurrent processes.
    pid = fork();

    // The new child process.
    if(pid == 0) {
        
        // Calls execvp() to replace current program with a new one.
        if(execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(1);
    }

    // There was a forking error.
    else if(pid < 0) {
        perror("shell");
    }

    // Original parent process
    else {
        do {
            // Parent process waits for child to execute.
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // Return 1 as to prompt for user input again.
    return 1;
}


/**
 * Check if the commands given are correct and 
 * call the function to execute them if they are.
 */
int execute(char** args) {
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    // Check if the user entered one of the built-in commands.
    for(i = 0; i < builtin_func_count(); i++) {
        if(strcmp(args[0], builtin_cmds[i]) == 0) {
            
            // Return the matching built-in shell function.
            return (*builtin_funcs[i])(args);
        }
    }

    // If no built-in command is matched, launch the process.
    return launch(args);
}


/**
 * Prints shell info for user assistance. 
 */
int shell_help(char **args) {
  int i;
  printf("Ted Juntunen's Shell\n");
  printf("The following commands are built in:\n");

  for(i = 0; i < builtin_func_count(); i++) {
    printf("  %s\n", builtin_cmds[i]);
  }

  printf("Use the man command for detailed information on programs.\n");
  return 1;
}


int shell_exit(char **args) { return 0; }

/**
 * Parse and tokenize the given user command
 * and return an array of char* of the split
 * words.
 */
char** parse_cmd(char* cmd) {
    const char* DELIMITER = " \t\r\n\a";
    int bufsize = DEFAULT_BUFSIZE;
    int index = 0;
    char* tok;
    char** tokens = (char**) malloc(bufsize * sizeof(char*));

    // If tokens is NULL print error and exit.
    if(!tokens) {
        printf("Allocation error.\n");
        exit(1);
    }

    // Create a token of the first element of cmd.
    tok = strtok(cmd, DELIMITER);

    while(tok != NULL) {
        tokens[index++] = tok;

        // Increase the bufsize and reallocate memory if necessary.
        if(index >= bufsize) {
            bufsize += DEFAULT_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if(!tokens) {
                printf("Allocation error.\n");
                exit(1);
            }
            
        }
        tok = strtok(NULL, DELIMITER);
    }
    
    // Set NULL terminator for array.
    tokens[index] = NULL;

    return tokens;
}


/**
 * Read in a command and return it as a char*.
 */
char* read_cmd() {
    char *cmd = NULL;
    size_t bufsize;
    getline(&cmd, &bufsize, stdin);
    return cmd;
}


/**
 * The main loop that runs the shell. Takes in  
 * user commands, parses them, and executes them.
 */
void shell_loop() {
    char *cmd;
    char **args;
    int status;

    do {
        printf("> ");

        // Read the command the user inputs.
        cmd = read_cmd();

        // Parse the command into tokens.
        args = parse_cmd(cmd);

        // Execute the commands.
        status = execute(args);

        free(cmd);
        free(args);

    } while(status);
}


int main() {
    shell_loop();
    return 0;
}
