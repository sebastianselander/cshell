#include "shell.h"
#include <string.h>
#include "builtins.h"
#include "utils.h"

typedef struct ExitInfo ExitInfo;

char* read_line() {
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("read line");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

#define TOK_DELIM " \r\t\n\a"
#define TOK_BUFSIZE 64
char** split_line(char* line) {
    int bufsize = TOK_BUFSIZE;
    int position = 0;

    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "Reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void prompt(int exit_code) {
    char *cwd = getcwd(NULL, 0);
    char *green = "\033[32m";
    char *red = "\033[31m";
    char *close = "\033[0m";
    if (exit_code) {
        printf("%s\n%s>%s ", cwd, red, close);
    } else {
        printf("%s\n%s> %s", cwd, green, close);
    }
    free(cwd);
}

ExitInfo shell_launch(char **args, int exit_code) {
    pid_t pid;
    int exit_c;
    bool negate = false;

    if (strcmp(args[0], "!") == 0) {
        negate = true;
        // shift all elements down one step, removing the bang
        for (int i = 1;; i++) {
            args[i-1] = args[i];
            if (args[i] == NULL) {
                break;
            }
        } 
    }

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        pid_t wpid = wait(&exit_c);
        normalize_status(&exit_c);
        if (negate) {
            exit_c = !exit_c;
        }
    }

    ExitInfo exit_info = exit_info_init();
    exit_info.exit_code = exit_c;
    return exit_info;
}

ExitInfo shell_execute(char **args, int exit_code) {
    if (args[0] == NULL) {
        ExitInfo exit_info = {0, false};
        return exit_info;
    }

    for (int i = 0; i < cbsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args, exit_code);
}

void shell_loop() {
    ExitInfo exit_info = exit_info_init();

    while (1) {
        prompt(exit_info.exit_code);
        char *line = read_line();
        char **args = split_line(line);
        exit_info = shell_execute(args, exit_info.exit_code);

        free(line);
        free(args);

        if (exit_info.terminate) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    shell_loop();
}