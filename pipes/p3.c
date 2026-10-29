#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define BUF_SIZE 4096

int handled = 0;

void handler(int sig) {
    handled++;
}

int main(int argc, char **argv) {
    /* command line arguments */
    if (argc < 3) {
        fprintf(stderr, "usage: %s writeSize iterations [restart]\n", argv[0]);
        exit(1);
    }

    int writeSize = atoi(argv[1]);
    int itr = atoi(argv[2]);
    int restart = (argc > 3 && strcmp(argv[3], "restart") == 0);

    if (writeSize <= 0 || itr <= 0) {
        fprintf(stderr, "error: writeSize and iterations must be positive\n");
        exit(1);
    }

    /* establish pipe */
    int fds[2]; // fd[0] = read, fd[1] = write
    if (pipe(fds) < 0) { // pipe failed
        perror("pipe");
        exit(1);
    }


    pid_t reader_pid, writer_pid;
    int n = 0;
    char buf[BUF_SIZE];
    int hist[556] = {0};
    long long int total = 0; // probably unnecessarily long...

    
    /* reader side */
    switch(reader_pid = fork()) {
        case -1: // fork failed
            fprintf(stderr, "error: Can not fork reader_pid: %s\n", strerror(errno));
            _exit(1);
        case 0: // child
            close(fds[1]); // reader never writes
            while((n = read(fds[0], buf, 555)) > 0) {
                hist[n] ++;
                total += n;
            }

            if (n == 0) {
                printf("read returned EOF\n");
            } else {
                fprintf(stderr, "error: read: %s\n", strerror(errno));
            }

            for(int i = 0; i <= 555; i++) {
                if (hist[i] > 0) {
                    fprintf(stdout, "%d reads of %d bytes\n", hist[i], i);
                }
            }
            fprintf(stdout, "%lld total bytes read\n", total);

            exit(0);
        default:
            break;
    }


    /* block sigusr1 until writer establishes */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);

    /* writer side */
    switch(writer_pid = fork()) {
        case -1: // fork failed
            fprintf(stderr, "error: Can not fork writer_pid: %s\n", strerror(errno));
            _exit(1);
        case 0: { // child
            close(fds[0]); // writer never reads

            /* setting up buffers */
            // malloc because wbuf[writeSize] doesn't work with high number
            int *wcount = calloc(writeSize + 1, sizeof(int));
            char *wbuf = malloc(writeSize);

            if ((wcount == NULL) | (wbuf == NULL)) {
                fprintf(stderr, "error: can not allocate");
                exit(1);
            }

            memset(wbuf, 'A', writeSize);

           /* sigaction, establish handler and unblock */
            struct sigaction sa;
            sa.sa_handler = handler;
            sa.sa_flags = restart ? SA_RESTART : 0;
            sigemptyset(&sa.sa_mask);
            sigaction(SIGUSR1, &sa, NULL);
            sigprocmask(SIG_UNBLOCK, &set, NULL);

            /* start writing */
            int eintr = 0;
            long long int wtotal = 0;
            for (int i = 0; i < itr; i++) {
                int w = write(fds[1], wbuf, writeSize);

                if (w < 0) {
                    if (errno == EINTR) {
                        eintr++;
                        continue;
                    }
                    fprintf(stderr, "error: can not write: %s\n", strerror(errno));
                    exit(1);
                }
                wcount[w]++;
                wtotal += w;
            }

            /* block SIGUSR1 */
            sigprocmask(SIG_BLOCK, &set, NULL);

            printf("%d EINTRs\n", eintr);
            printf("%d SIGUSR1 handled\n", handled);
            for (int i = 0; i <= writeSize; i++) {
                if (wcount[i] > 0) {
                    printf("%d writes of %d bytes\n", wcount[i], i);
                }
            } 
            printf("%lld total bytes written\n", wtotal);


            free(wbuf);
            free(wcount);
            exit(0);
        }
        default:
            break;
    }
    close(fds[0]);
    close(fds[1]);

    /* parent code */
    long long int sent = 0;
    struct timespec sp = {0, 5000};

    while (waitpid(writer_pid, NULL, WNOHANG) != writer_pid) {
        if (kill(writer_pid, SIGUSR1) == -1) {
            if (errno == ESRCH) {
                fprintf(stderr, "kill returned ESRCH, writer is done\n");
            } else {
                fprintf(stderr, "kill failed: %s\n", strerror(errno));
            }
            break;
        }
        sent ++;
        nanosleep(&sp, NULL);
    }

    waitpid(reader_pid, NULL, 0);

    printf("writer is done\n");
    printf("%lld SIGUSR1 sent\n", sent);

    return 0;
}
