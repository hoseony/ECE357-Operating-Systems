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
    char *inputFile;

    char *outputFile;
    int outputAppend;

    char *errorFile;
    int errorAppend;
} Redirection_t;


int tokenize(char *line, char **buf);
int execute(char **token, int *lastExitStatus);
int builtIn_cd(char *dir);
int builtIn_pwd();
int builtIn_exit(char **argv, int *lastExitStatus);
void userPrompt(); 
void DEBUG_IO(Redirection_t redir);
void DEBUG_TOKEN(char **token);

#endif
