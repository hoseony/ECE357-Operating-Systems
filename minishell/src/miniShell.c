#include "miniShell.h"

#define BUFSIZE 4096         /* set buffer size for any buffer used in this program */
#define TOKENDELIM " \r\n\t" /* this is used for strtok */

/* added some labels */
#define DEBUG "\x1b[32m [ DEBUG ] \x1b[0m" /* green */
#define IO    "\x1b[34m [ I/O ] \x1b[0m"   /* blue */
#define ERROR "\x1b[31m [ ERROR ] \x1b[0m" /* red */

/* I learend something new */
#ifdef DEBUG_MODE
#define DBG(...) do { fprintf(stderr, DEBUG); fprintf(stderr, __VA_ARGS__); } while (0)
#define PIO(...) do { fprintf(stderr, IO); fprintf(stderr, __VA_ARGS__); } while (0)
#else
#define DBG(...) ((void)0)
#define PIO(...) ((void)0)
#endif

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

        execute(token, &exitStatus);
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

int parseRedirection(char **token, Redirection_t *redir) {
    int j = 0;
   
    /* handles both > out.txt and >out.txt */
    for (int i = 0; token[i] != NULL; i++) {
        if (strncmp(token[i], "2>>", 3) == 0) { /* stderr append */
            if (token[i][3] != '\0') {
                redir->errorFile = token[i] + 3;
            } else {
                if (token[i + 1] == NULL) {
                    fprintf(stderr, "miniShell: expected file after 2>>\n");
                    return -1;
                }
                redir->errorFile = token[i + 1];
                i++;
            }

            redir->errorAppend = 1;
        } 

        else if (strncmp(token[i], "2>", 2) == 0) { /* stderr trunc */
            if (token[i][2] != '\0') {
                redir->errorFile = token[i] + 2;
            } else {
                if (token[i + 1] == NULL) {
                    fprintf(stderr, "miniShell: expected file after 2>\n");
                    return -1;
                }
                redir->errorFile = token[i + 1];
                i++;
            }

            redir->errorAppend = 0;
        }

        else if (strncmp(token[i], ">>", 2) == 0) { /* stdout append */
            if (token[i][2] != '\0') {
                redir->outputFile = token[i] + 2;
            } else {
                if (token[i + 1] == NULL) {
                    fprintf(stderr, "miniShell: expected file after >>\n");
                    return -1;
                }
                redir->outputFile = token[i + 1];
                i++;
            }

            redir->outputAppend = 1;
        }

        else if (strncmp(token[i], ">", 1) == 0) {  /* stdout trunc */
            if (token[i][1] != '\0') {
                redir->outputFile = token[i] + 1;
            } else {
                if (token[i + 1] == NULL) {
                    fprintf(stderr, "miniShell: expected file after >\n");
                    return -1;
                }
                redir->outputFile = token[i + 1];
                i++;
            }
            redir->outputAppend = 0;
        }

        else if (strncmp(token[i], "<", 1) == 0) {  /* stdin */
            if (token[i][1] != '\0') {
                redir->inputFile= token[i] + 1;
            } else {
                if (token[i + 1] == NULL) {
                    fprintf(stderr, "miniShell: expected file after <\n");
                    return -1;
                }
                redir->inputFile= token[i + 1];
                i++;
            }
        }

        else {
            // if it is neither of the redirectino, then it is an actual command
            // reconstructing the command without any redirection info
            token[j++] = token[i];
        }
    }

    token[j] = NULL;
    return 0;
}

int redirectIO(Redirection_t *redir) {
    if (redir->inputFile != NULL) { /* input */
        int inFd = open(redir->inputFile, O_RDONLY);
        if (inFd < 0) {
            fprintf(stderr, "Can not open %s: %s", redir->inputFile, strerror(errno));
            _exit(1);
        }

        dup2(inFd, STDIN_FILENO);
        close(inFd);
    }

    if (redir->outputFile != NULL) { /* output */
        int outFd;
        if (redir->outputAppend == 1) {
            outFd = open(redir->outputFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
            DBG("opened with append\n");
        } else {
            outFd = open(redir->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }

        if (outFd < 0) { /* input */
            fprintf(stderr, "Can not open %s: %s", redir->outputFile, strerror(errno));
            _exit(1);
        }

        dup2(outFd, STDOUT_FILENO); // redirect stdout
        close(outFd);
    }

    if (redir->errorFile != NULL) { /* error */
        int errorFd;
        if (redir->errorAppend == 1) {
            errorFd = open(redir->errorFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
        } else {
            errorFd = open(redir->errorFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }

        if (errorFd < 0) {
            fprintf(stderr, "Can not open %s: %s", redir->errorFile, strerror(errno));
            _exit(1); 
        }

        dup2(errorFd, STDERR_FILENO);
        close(errorFd);
    }

    return 1;
}

int execute(char **token, int *exitStatus) {
    /* I/O redirection, find the  */
    Redirection_t redir = {0};
    if (parseRedirection(token, &redir) < 0) {
        return 1;
    }

    if (token[0] == NULL) {
        fprintf(stderr, "miniShell: no command was specified");
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
            fprintf(stderr, ERROR "failed fork\n");
            break;

        case 0: /* child process */
            DBG("running child: %s\n", token[0]);
            redirectIO(&redir); /* handle I/O redirection */

            /* run the program */
            execvp(token[0], token);
            fprintf(stderr, ERROR "execvp failed: %s\n", strerror(errno));
            _exit(127);

        default:
            wait3(&status, 0, &usage);
            clock_gettime(CLOCK_MONOTONIC, &end);

            double realTime, userTime, sysTime;
            realTime = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1000000000.0);
            userTime = (usage.ru_utime.tv_sec) + (usage.ru_utime.tv_usec / 1000000.0);
            sysTime = (usage.ru_stime.tv_sec) + (usage.ru_stime.tv_usec / 1000000.0);

            if (WIFEXITED(status)) {
                *exitStatus = WEXITSTATUS(status);
                fprintf(stderr, "pid %d (child) exited with return %d\n", pid, *exitStatus);
            } else if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status)
                *exitStatus = 128 + WTERMSIG(status);
                fprintf(stderr, "pid %d (child) exited with signal %d\n", pid, *exitStatus);
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
    PIO("input:  %s\n", redir.inputFile  ? redir.inputFile  : "NULL");
    PIO("output: %s\n", redir.outputFile ? redir.outputFile : "NULL");
    PIO("error:  %s\n", redir.errorFile  ? redir.errorFile  : "NULL");
    PIO("out append: %d\n", redir.outputAppend);
    PIO("err append: %d\n", redir.errorAppend);
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
