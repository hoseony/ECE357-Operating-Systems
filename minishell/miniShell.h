#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define _GNU_SOURCE

int tokenize(char *line, char **buf);
int execute(char **token);
int builtIn_cd(char *dir);
int builtIn_pwd();
int builtIn_exit();

#endif
