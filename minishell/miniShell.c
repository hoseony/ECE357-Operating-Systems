#include "miniShell.h"

#define BUFSIZE 4096         /* set buffer size for any buffer used in this program */
#define TOKENDELIM " \r\n\t" /* this is used for strtok */

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

    /* read all the lines */
    char *line = NULL;
    size_t size = 0;

    int lastExitStatus = 0; /* keep track of last exist status, used for builtIn_exit */

    while (getline(&line, &size, fp) != -1) {
        fprintf(stderr, "[ DEBUG ] LINE_READ: %s", line);

        /* skip # */
        if (line[0] == '#') {
            printf("[ DEBUG ] # detected, skipping...\n");
            continue;
        }

        /* parse and execute */
    
        // tokenization
        char *token[BUFSIZE];
        int size = tokenize(line, token);

        // empty line case, token[j] == NULL
        if (size == 0) {
            printf("\t  empty line detcted, skipping...\n");
            continue;
        }
        
        // debug tokenize
        printf("[ DEBUG ] TOKENIZE: ");
        for (int j = 0; j < size; j++) {
            printf("%s, ", token[j]);
        }
        printf("\n");

        execute(token, &lastExitStatus);
    }

    free(line);
    if (fp != stdin)
        fclose(fp);

    return 0;
}

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

int execute(char **token, int *lastExitStatus) { /* argv */
    /* I/O redirection, find the  */
    char *outputFile = NULL; 
    char *inputFile = NULL;

    int j = 0;

    for (int i = 0; token[i] != NULL; i++) {
        if (strcmp(token[i], ">") == 0) {
            if (token[i + 1] == NULL) {
                fprintf(stderr, "miniShell: syntax error, expected file after > \n");
                return -1;
            }
            outputFile = token[i + 1];
            i++; // skip the output file
        } else if (strcmp(token[i], "<") == 0) {
            if (token[i + 1] == NULL) {
                fprintf(stderr, "miniShell: syntax error, expected file after < \n");
                return -1;
            }
            inputFile = token[i + 1];
            i++; // skip the input file
        } else {
            // if it is neither of the redirectino, then it is an actual command
            // reconstructing the command without any redirection info
            token[j] = token[i];
            j++;
        }
    }
    token[j] = NULL;

    /* run built-in commands */
    if (strcmp(token[0], "pwd") == 0) {
        builtIn_pwd();
        return 1;
    }
    if (strcmp(token[0], "cd") == 0) {
        builtIn_cd(token[1]);
        return 1;
    }
    if (strcmp(token[0], "exit") == 0) {
        builtIn_exit(token, lastExitStatus);
        return 1; // left it just in case...
    }

    /* if none of the built-in commands were matched, run the program */
    int pid, status;
    switch (pid = fork()) {
        case -1:
            fprintf(stderr, "[ DEBUG ] failed fork\n");
            break;

        case 0: 
            /* handle I/O redirection */
            if (inputFile != NULL) {
                int inFd = open(inputFile, O_RDONLY);
                if (inFd < 0) {
                    fprintf(stderr, "Can not open %s: %s", inputFile, strerror(errno));
                    return 1;
                }

                dup2(inFd, STDIN_FILENO);
            }

            if (outputFile != NULL) {
                int outFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (outFd< 0) {
                    fprintf(stderr, "Can not open %s: %s", outputFile, strerror(errno));
                    return 1;
                }

                dup2(outFd, STDOUT_FILENO);
            }

            fprintf(stderr, "[ DEUBG ] running child\n");
            int returnState = execvp(token[0], token);
            /* error handling */

            if (returnState < 0) {
                fprintf(stderr, "[ ERROR ] execvp returned with an error: %s\n", strerror(errno));
            }

            break;

        default:
            wait(&status);
            fprintf(stderr, "[ DEBUG ] waiting done. status: %d\n", status);
    }

    return 1;
}

/* built-in commands */
int builtIn_cd(char *dir) {
    /* default: home directory */
    if (dir == NULL) {
        char *value;
        value = getenv("HOME");
        printf("[ DEBUG ] getenv: %s\n", value);
        chdir(value);
        return 1;
    }

    /* else, use dir */
    chdir(dir);
    printf("[ DEBUG ] "); builtIn_pwd();
    return 1;
}

int builtIn_pwd() {
    char cwd[BUFSIZE];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 1;
}

int builtIn_exit(char **argv, int *lastExitStatus) {
    /* if no argument, */
    if (argv[1] == NULL) {
        exit(*lastExitStatus);
    }

    int exitStatus = atoi(argv[1]);
    // printf("%d\n", exitStatus);

    exit(exitStatus);
    return 1;
}

