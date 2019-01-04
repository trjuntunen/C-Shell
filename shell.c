#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char* builtin_cmds[] = {"cd", "help", "exit"};

int (*builtin_funcs[]) (char **) = {&shell_cd, &shell_help, &shell_exit};

int builtin_func_count() {
  return sizeof(builtin_cmds) / sizeof(char *);
}

int shell_cd(char** args) {
    if(args[1] == NULL) {
        fprintf(stderr, "Expected argument to \"cd\"\n");
    } else {
        if(chdir(args[1]) != 0) {
            perror("err");
        }
    }
    return 1;
}

int launch(char** args) {
    pid_t pid, wpid;
    int status;

    pid = fork();

    // Child process
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("err");
        }
        exit(1);
    
    // Forking error
    } else if(pid < 0) {
        perror("err");
    
    // Parent process
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char** args) {
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < builtin_func_count(); i++) {
        if (strcmp(args[0], builtin_cmds[i]) == 0) {
            return (*builtin_funcs[i])(args);
        }
    }

    return launch(args);
}

int shell_help(char **args) {
  int i;
  printf("Ted Juntunen's Shell\n");
  printf("The following commands are built in:\n");

  for (i = 0; i < builtin_func_count(); i++) {
    printf("  %s\n", builtin_cmds[i]);
  }

  printf("Use the man command for detailed information on programs.\n");
  return 1;
}

int shell_exit(char **args) { return 0; }

char** parse_line(char* line) {
    int bufsize = 64, position = 0;
    char** tokens = (char**) malloc(bufsize * sizeof(char*));
    char* token;

    if(!tokens) {
        printf("Allocation error.\n");
        exit(1);
    }

    token = strtok(line, " \t\r\n\a");
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if(!tokens) {
                printf("Allocation error.\n");
                exit(1);
            }
            
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

char* read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

void shell_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = read_line();
        args = parse_line(line);
        status = execute(args);
        free(line);
        free(args);
    } while(status);
}

int main() {
    shell_loop();
    return 0;
}
