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
        fprintf(stderr, "       kit [-o outfile]\n");
        return -1;
    }

    /* check for the output flag "-o" */
    int opt; 
    int flag_idx = 0; // keeps track of the index of output file

    char *output = NULL;
 
    for (int i = 0; i < argc -1; i++) {
        printf("argv[%d]: %s\n",i, argv[i]);
    }
    putchar('\n');

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
        case 'o':
         
            for (int i = 0; i < argc -1; i++) {
                printf("argv[%d]: %s\n",i, argv[i]);
            }
            putchar('\n');

            output = optarg;
            flag_idx = optind - 1;
            printf("flag_dx: %d\n", flag_idx);
            break;

        default: 
            fprintf(stderr, "usage: kit [-o outfile] infile1 [...infile2....]\n");
            fprintf(stderr, "       kit [-o outfile]\n");
            exit(EXIT_FAILURE);
       }

        putchar('\n');
    }


    printf("output file: %s | index: %d\n", output, flag_idx);


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

    // It should skeep indicies of things it previously read.
    // Specifically, -o and its arg


    for (int i = 0; i < argc -1; i++) {
        printf("argv[%d]: %s\n",i, argv[i]);
    }
    putchar('\n');



    int idx = 1;

    while(1) {
        if (idx == flag_idx || idx == flag_idx - 1 || idx == argc - 1) {
            continue;
        }

        printf("argv[%d]: %s\n", idx, argv[idx]);
        idx++;

    }

    return 0;
}
