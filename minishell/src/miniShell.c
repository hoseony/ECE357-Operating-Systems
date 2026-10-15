#include "miniShell.h"

/* ========== main function ========== */
int main(int argc, char **argv) {
    char *path = argv[1];
    FILE *fp;
    
    /* check and set corresponding fd */
    if (argc == 1) {
        fp = stdin;
    } else if (argc == 2) {
        fp = fopen(path, "r");

        if (fp == NULL) {
            perror(argv[1]);
            return 1;
        }
    } else {
        fprintf(stderr, "usage: %s [script]\n", argv[0]);
        return 1;
    }

    // userPrompt(); 
    
    /* read all the lines */
    char *line = NULL;
    size_t size = 0;
    int exitStatus = 0;

    while (getline(&line, &size, fp) != -1) {
        DBG("LINE_READ: %s", line);

        if (line[0] == '#') { /* skip # */
            DBG("# detected, skipping...\n");
            continue;
        }

        /* parse and execute */
        char *token[BUFSIZE];
        int size = tokenize(line, token);

        if (size == 0) { /* empty line case, token[j] == NULL */
            DBG("empty line detcted, skipping...\n");
            continue;
        }
        
        DEBUG_TOKEN(token);

        execute(token, &exitStatus, fp);
    }

    free(line);
    if (fp != stdin)
        fclose(fp);

    return exitStatus;
}

/* ========== Token / Parse ========== */
int tokenize(char *line, char **buf) {
    int i = 0;
    char *token = strtok(line, TOKENDELIM);

    while (token != NULL) {
        buf[i++] = token;
        token = strtok(NULL, TOKENDELIM);
    }
    
    buf[i] = NULL;

    return i;
}

/* if token starts with a redirection operator, return that
 * entry from the table. This approach was used to avoid cut
 * and copy code. */
const RedirOption_t *findRedir(char *token) {
    for (int i = 0; i < 5; i++) {
        // redirOptions contains operation, fd and flags
        const char *op = redirOptions[i].operation;
        
        // If first the leading characters are the same
        // as the operation in the redirOptions, return
        // the values from the table. Otherwise, NULL.
        if (strncmp(token, op, strlen(op)) == 0) {
            return &redirOptions[i];
        }
    }
    return NULL;
}

int parseRedirection(char **token, Redirection_t *redir) {
    int j = 0;

    // loop through the tokens
    for (int i = 0; token[i] != NULL; i++) {
        // get the operation, fd and flags 
        const RedirOption_t *opt = findRedir(token[i]);

        /* keep the normal words */
        if (opt == NULL) {
            // reconstructing the token as the code runs
            // (excluding I/O redirection flags)
            token[j++] = token[i];
            continue;
        }

        /* redirection (case where there is no space) */
        char *file = token[i] + strlen(opt->operation);

        /* if that didn't work, go to the next token */
        if (file[0] == '\0') {
            i++;
            file = token[i];
            
            /* error handling */
            if (file == NULL) {
                fprintf(stderr, "miniShell: expected file after %s\n", opt->operation);
                return -1;
            }
        }

        redir->file[opt->fd] = file;
        redir->append[opt->fd] = opt->flags;
    }

    token[j] = NULL;
    return 0;
}

int redirectIO(Redirection_t *redir) {
    for (int i = 0; i < 3; i++) {
        /* no redirection specified */
        if (redir->file[i] == NULL) {
            continue;
        }
        
        /* open the file with the flags saved */
        int fd = open(redir->file[i], redir->append[i], 0666);

        if (fd < 0) {
            fprintf(stderr, "Can not open %s: %s\n", redir->file[i], strerror(errno));
            _exit(1);
        }

        dup2(fd, i);
        close(fd);
    }

    return 1;
}

/* ========== execution ========== */
int execute(char **token, int *exitStatus, FILE *fp) {
    /* I/O redirection, find the  */
    Redirection_t redir = {0};
    if (parseRedirection(token, &redir) < 0) {
        return 1;
    }

    if (token[0] == NULL) {
        fprintf(stderr, "miniShell: no command was specified\n");
        return 1;
    }

    DEBUG_IO(redir);

    /* run built-in commands */
    if (strcmp(token[0], "pwd") == 0) {
        *exitStatus = builtIn_pwd();
        return 1;
    }
    if (strcmp(token[0], "cd") == 0) {
        *exitStatus = builtIn_cd(token[1]);
        return 1;
    }
    if (strcmp(token[0], "exit") == 0) {
        builtIn_exit(token, exitStatus);
        return 1; // left it just in case...
    }

    /* if none of the built-in commands were matched, run the program */
    int pid, status;
    struct rusage usage;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    switch (pid = fork()) {
        case -1: /* error */
            fprintf(stderr, "ERROR: failed fork\n");
            break;

        case 0: /* child process */
            if (fp != stdin)
                close(fileno(fp));

            DBG("running child: %s\n", token[0]);
            redirectIO(&redir); /* handle I/O redirection */

            /* run the program */
            execvp(token[0], token);
            fprintf(stderr, "ERROR: execvp failed: %s\n", strerror(errno));
            _exit(127);

        default:
            /* get time */
            wait3(&status, 0, &usage);
            clock_gettime(CLOCK_MONOTONIC, &end);

            double realTime, userTime, sysTime;
            realTime = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1000000000.0);
            userTime = (usage.ru_utime.tv_sec) + (usage.ru_utime.tv_usec / 1000000.0);
            sysTime = (usage.ru_stime.tv_sec) + (usage.ru_stime.tv_usec / 1000000.0);

            /* check exit status */
            if (WIFEXITED(status)) {
                *exitStatus = WEXITSTATUS(status);
                fprintf(stderr, "pid %d (child) exited with return %d\n", pid, *exitStatus);
            } else if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                *exitStatus = 128 + sig;
                fprintf(stderr, "pid %d (child) exited with signal %d (%s)\n", pid, sig, strsignal(sig));
            }

            fprintf(stderr, "Real: %.3fs User: %.3fs Sys: %.3fs\n", realTime, userTime, sysTime);
    }

    return 1;
}

/* ========== built-in commands ========== */
int builtIn_cd(char *dir) {
    /* default: home directory */
    if (dir == NULL) {
        dir = getenv("HOME");
        
        if (dir == NULL) {
            fprintf(stderr, "miniShell: HOME not set\n");
            return 1;
        }
    }

    if (chdir(dir) < 0) {
        fprintf(stderr, "miniShell: cd: %s: %s\n", dir, strerror(errno));
        return 1;
    }

    return 0;
}

int builtIn_pwd() {
    char cwd[BUFSIZE];

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "miniShell: pwd: %s\n", strerror(errno));
        return 1;
    }
    fprintf(stdout, "%s\n", cwd);
    return 0;
}

int builtIn_exit(char **argv, int *exitStatus) {
    /* if no argument, */
    if (argv[1] == NULL) {
        exit(*exitStatus);
    }

    *exitStatus = atoi(argv[1]);
    DBG("%d\n", *exitStatus);

    exit(*exitStatus);
    return 1;
}

void userPrompt() {
    char cwd[BUFSIZE];
    getcwd(cwd, sizeof(cwd));
    fprintf(stdout, "~%s > ", cwd);
}

/* ========== helper functions ========== */
void DEBUG_IO(Redirection_t redir) {
    PIO("input:  %s\n", redir.file[0] ? redir.file[0] : "NULL");
    PIO("output: %s\n", redir.file[1] ? redir.file[1] : "NULL");
    PIO("error:  %s\n", redir.file[2] ? redir.file[2] : "NULL");
}

void DEBUG_TOKEN(char **token) {
#ifdef DEBUG_MODE 
    // debug tokenize
    printf(DEBUG "TOKENIZE: ");
    for (int j = 0; token[j] != NULL; j++) {
        printf("%s, ", token[j]);
    }
    printf("\n");
#endif
}
