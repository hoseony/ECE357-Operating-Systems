#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/*
 * First read the man page, 
 * then validated with this code.
 */

int p2a(void) {
    int a = close(-1);

    printf("return: %d\n", a);
    printf("errno number: %d\n", errno);

    if (errno == EBADF) {
        perror("");
    }

    return 0;
}

int p2b(void) {
    // cool thing I found: /dev/full
   
    int fd = open("/dev/full", O_WRONLY);
    char buf[] = "hello";

    ssize_t w = write(fd, buf, 5);

    printf("write returned: %zd\n", w);
    printf("errno: %d\n", errno);

    if (errno == ENOSPC) {
        perror("");
    }

    return 0;
}

int p2c(void) {
    int fd = open("non-existent file!", O_RDONLY);

    printf("open returned: %d\n", fd);
    printf("errno: %d\n", errno);

    if (errno == ENOENT) {
        perror("");
    }

    return 0;
}

int p2d(void) {
    ssize_t w = write(1, NULL, 1);   

    printf("write returned: %zd\n", w);
    printf("errno: %d\n", errno);

    if (errno == EFAULT) {
        perror("");
    }

    return 0;
}
int main(void) {
    p2d();

    return 0;
}
