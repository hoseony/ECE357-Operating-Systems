#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// read write buffer size: 4096
#define BUFFSIZE 4096

int main(int argc, char ** argv) {
    if (argc == 1) {
        fprintf(stderr, "usage: kit [-o outfile] infile1 [...infile2....]\n");
        fprintf(stderr, "       kit [-o outfile]");
        return -1;
    }

    /* check for the output flag "-o" */
    int opt; 
    int idx = 0;
    char *output = NULL;

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
        case 'o':
            output = optarg;
            idx = optind;
            break;
        default: 
            fprintf(stderr, "usage: kit [-o outfile] infile1 [...infile2....]\n");
            fprintf(stderr, "       kit [-o outfile]");
            exit(EXIT_FAILURE);
       }
    }

    printf("output file: %s | index: %d\n", output, idx);

    
    /* decide which file descriptor to use for output */
    int out_fd = 1; // if no flag stdout

    if (output != NULL) { 
        if ((out_fd = open(output, O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0) {
            fprintf(stderr, "can't open file %s for writing: %s\n",
                    output, strerror(errno));
            return -1;
        }
    } 

    /* start reading infiles*/
    char buf[BUFFSIZE];
    
    /* find next file from command-line */
    while(idx < argc) {
        printf("arg[%d]: %s\n", idx, argv[idx]);
        idx++;
    }

    return 0;
}
