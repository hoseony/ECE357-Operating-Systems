#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUF_SIZE 4096

int main() {
    /* establish pipe */
    int fds[2]; // fd[0] = read, fd[1] = write
    if (pipe(fds) < 0) {
        // pipe failed
        perror("pipe");
        exit(1);
    }

    pid_t reader_pid, writer_pid;
    int n = 0;
    char buf[BUF_SIZE];

    switch(reader_pid = fork()) {
        case -1: // fork failed
            _exit(1);
        case 0: // child
            close(fds[1]); // reader never writes
            while((n = read(fds[0], buf, BUF_SIZE)) > 0) {
                fwrite(buf, sizeof(char), n, stdout);
            }

            exit(0);
        default:

            break;
    }

    switch(writer_pid = fork()) {
        case -1: // fork failed
            _exit(1);
        case 0: // child
            close(fds[0]); // writer never reads
            char msg[555];
            memset(msg, '5', sizeof(msg));
            write(fds[1], msg, 555);

            exit(0);
        default:
            break;
    }
    close(fds[0]);
    close(fds[1]);

    waitpid(reader_pid, NULL, 0);
    waitpid(writer_pid, NULL, 0);

    return 0;
}
