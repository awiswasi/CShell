// This is a simple shell using C language.
// Built by Awf Wiswasi, Inspired by Stephen Brennan's lsh.
// GitHub: https://github.com/awiswasi/

// waitpid()
#include <sys/wait.h>
// chdir(), fork(), exec(), pid_t
#include <unistd.h>
// malloc(), realloc(), free(), exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <stdlib.h>
// fprintf(), printf(), stderr, getchar(), perror()
#include <stdio.h>
// strcmp(), strtok()
#include <string.h>

// Built-in Linux shell commands
int cshell_help(char **args);      // help
int cshell_cd(char **args);        // change directory
int cshell_exit(char **args);      // exit

char *builtin_str[] = 
{
    "help",
    "cd",
    "exit"
};

int (*builtin_func[]) (char **) =
{
    &cshell_help,
    &cshell_cd,
    &cshell_exit
};

int cshell_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/**
    @brief CShell built-in command - HELP.
    @param args List of args.
    @return Always returns 1 to continue executing.
*/
int cshell_help(char **args)
{
    int i;
    printf("_____________________\n\n");
    printf("CShell by Awf Wiswasi\n");
    printf("https://github.com/awiswasi/\n");
    
    printf("\nThis is a simple Linux shell built with C language.\n");
    printf("Built-in commands:\n");

    for (i = 0; i < cshell_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("\nFor any questions or support, please contact me: awf.wis@gmail.com\n");
    printf("__________________________________________________________________\n\n");

    return 1;
}

/**
    @brief CShell built-in command - CHANGE DIRECTORY.
    @param args List of args, where args[0]: "cd", and args[1]: the directory.
    @return Always returns 1 to continue executing.
*/
int cshell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "cshell: expected argument to \"cd\"\n");
    }

    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("cshell");
        }
    }

    return 1;
}

/**
    @brief CShell built-in comman - EXIT.
    @param args List of args.
    @return Always returns 0 to terminate execution.
*/
int cshell_exit(char **args)
{
    printf("\033[0m");
    return 0;
}

/**
    @brief Launches a program and waits for termination.
    @param args Null terminated list of arguments (including program).
    @return Always returns 1 to continue execution.
 */
int cshell_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("cshell");
        }

        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("cshell");
    }
    else
    {
        // Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
    @brief Executes CShell built-in commands, or launches a program.
    @param args Null terminated list of arguments.
    @return Returns 1 to continue executing, or 0 to terminate.
*/
int cshell_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < cshell_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return cshell_launch(args);
}

#define CSHELL_RL_BUFSIZE 1024
/**
    @brief Reads a line of input from stdin.
    @return Returns the line from stdin.
*/
char *cshell_read_line(void)
{
    int bufsize = CSHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read a character
        c = getchar();

        // If EOF, replaces it with a NULL and returns.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If buffer is exceeded, then reallocates.
        if (position >= bufsize)
        {
            bufsize += CSHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "cshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define CSHELL_TOK_BUFSIZE 64
#define CSHELL_TOK_DELIM " \t\r\n\a"
/**
    @brief Splits a line into tokens.
    @param line The given line.
    @return Returns a null-terminated array of tokens.
*/
char **cshell_split_line(char *line)
{
    int bufsize = CSHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CSHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += CSHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "cshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CSHELL_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

/**
    @brief Loops through input and executes.
*/
void cshell_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("\033[33mcshell> ");
        line = cshell_read_line();
        args = cshell_split_line(line);
        status = cshell_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
    @brief Program's main function.
    @param argc Argument count.
    @param argv Argument vector.
    @return status code
*/
int main(int argc, char **argv)
{
    // Load config files, if any.

    // Run command loop.
    cshell_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}