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

    while (getline(&line, &size, fp) != -1) {
        fprintf(stderr, "[ DEBUG ] LINE_READ: %s", line);
        if (line[0] == '#') {
            printf("[ DEBUG ] # detected, skipping...\n");
            continue;
        }

        /* smoe how parse and execute it */
    
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

        execute(token);
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

int execute(char **token) {
    if (strcmp(token[0], "pwd") == 0)
        builtIn_pwd();
    if (strcmp(token[0], "cd") == 0)
        builtIn_cd(token[1]);
    if (strcmp(token[0], "exit") == 0)
        ;
    /* if none of the built-in commands were matched,
     * execute that thing 
     */
    
    switch (fork()) {
        case 0: 
            execvp(token[0], token);
    }

    return 1;
}

/* built-in commands */
int builtIn_cd(char *dir) {
    chdir(dir);
    // printf("%s\n", dir);
    printf("[ DEBUG ] "); builtIn_pwd();
    return 1;
}

int builtIn_pwd() {
    char cwd[BUFSIZE];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 1;
}

int builtIn_exit() {
    return 1;
}
