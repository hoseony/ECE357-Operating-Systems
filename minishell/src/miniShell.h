#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
// #define _GNU_SOURCE

typedef struct {
    char *file[3]; /* index 0, 1, 2 for stdin, stdou stderr */
    int append[3]; /* same convention, it stores open flags */
} Redirection_t;

typedef struct {
    const char *operation;
    int fd;
    int flags;
} RedirOption_t;

const RedirOption_t redirOptions[] = {
    {"2>>", STDERR_FILENO, O_WRONLY | O_CREAT | O_APPEND},
    {"2>",  STDERR_FILENO, O_WRONLY | O_CREAT | O_TRUNC},
    {">>",  STDOUT_FILENO, O_WRONLY | O_CREAT | O_APPEND},
    {">",   STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC},
    {"<",   STDIN_FILENO,  O_RDONLY},
};

#define BUFSIZE 4096         /* set buffer size for any buffer used in this program */
#define TOKENDELIM " \r\n\t" /* this is used for strtok */

#define DEBUG "\x1b[32m [ DEBUG ] \x1b[0m" /* green */
#define IO    "\x1b[34m [ I/O ] \x1b[0m"   /* blue */
#define ERROR "\x1b[31m [ ERROR ] \x1b[0m" /* red */

#ifdef DEBUG_MODE
#define DBG(...) do { fprintf(stderr, DEBUG); fprintf(stderr, __VA_ARGS__); } while (0)
#define PIO(...) do { fprintf(stderr, IO); fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define DBG(...) ((void)0)
#define PIO(...) ((void)0)
#endif


int tokenize(char *line, char **buf);
int execute(char **token, int *lastExitStatus, FILE *fp);
int builtIn_cd(char *dir);
int builtIn_pwd();
int builtIn_exit(char **argv, int *lastExitStatus);
void userPrompt(); 
void DEBUG_IO(Redirection_t redir);
void DEBUG_TOKEN(char **token);

#endif
