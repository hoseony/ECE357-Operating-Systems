#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 4096 // set buffer size for read or write 

int main(int argc, char ** argv) {
    // I used this error formatting as this was how it was presented in macOS 
    if (argc == 1) {
        fprintf(stderr, "usage: kit [-o outfile] infile1 [...infile2....]\n");
        fprintf(stderr, "       kit [-o outfile]\n");
        return -1;
    }

    /* check for the output flag "-o" */
    int opt; 
    char *output = NULL; // if there is no flag, output will remain NULL

    // getpot reorders the argv in a order.
    // It moves the flags and argument of the flags to the front.
    // (this behaves differently in macOS, it does not reorder argv).
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
        case 'o':
            output = optarg;
            break;
        default: 
            fprintf(stderr, "usage: kit [-o outfile] infile1 [...infile2....]\n");
            fprintf(stderr, "       kit [-o outfile]\n");
            exit(EXIT_FAILURE);
       }
    }

    /* decide which file descriptor to use for output */
    int out_fd = 1; // if no flag stdout
    
    if (output != NULL) {
        if ((out_fd = open(output, O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0) {
            fprintf(stderr, "Can't open file %s for writing: %s\n",
                    output, strerror(errno));
            return -1;
        }
    } else {
        // if no output file is specified, use stdout
        out_fd = 1;
    }

    /* reading the infiles */
    char buf[BUFFSIZE];
    
    // optind conveniently returns the index
    // of the next element to be processed in argv
    int bytes_read, bytes_wrote;
    int in_fd, is_stdin;


    for(int i = optind; i < argc; i++) {
        // if argv = "-", get the input from stdin
        is_stdin = (strcmp(argv[i], "-") == 0);

        if (is_stdin) {
            in_fd = 0;
        } else {
            if ((in_fd = open(argv[i], O_RDONLY, 0666)) < 0) {
                fprintf(stderr, "Can't open the file %s for reading: %s\n", 
                        argv[i], strerror(errno));
                return -1;
            }
        }

        /* handle partial read */
        while((bytes_read = read(in_fd, buf, sizeof(buf))) > 0) {

            /* handle partial write */
            ssize_t off = 0; // keeps track how many bytes are written

            while(off < bytes_read) {
                ssize_t r = write(out_fd, buf + off, bytes_read);
                if (r == -1) {
                    fprintf(stderr, "Can't write the file %s: %s\n", 
                            output, strerror(errno));
                    return -1;

                }
                off += r;
            }
        }

        /* handle bytes_read error */
        if (bytes_read < 0) {
            fprintf(stderr, "Can't read the file %s: %s\n",
                    argv[i], strerror(errno));
            return -1;
        }

        /* closing files */
        if (is_stdin && (close(in_fd) < 0)) {
            fprintf(stderr, "Can't close the file %s: %s\n",
                    argv[i], strerror(errno));
            return -1;
        }
    }

    if (close(out_fd) < 0) {
        fprintf(stderr, "Can't close the output: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}
