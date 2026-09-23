#include <stdio.h>
#include <fcntl.h>
// #include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 4096

int check_match(int fd1, int fd2);
int ensure_read(int fd, char *buf, ssize_t size);

int tree(char *path, int target_fd/* struct stat *target_finfo */) {
    struct dirent *dp;
    struct stat finfo;
    DIR *dir;
    int fd;

    /* open the directory */
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "cannot open %s\n", path);
        return -1;
    }

    /* loop through the directory */
    while((dp = readdir(dir)) != NULL) {
        // skip . and ..
        if ( (strcmp(".", dp->d_name) == 0) || (strcmp("..", dp->d_name) == 0) ) {
            continue;
        }

        // build a full path
        char buf[BUFSIZE];
        snprintf(buf, sizeof(buf), "%s/%s", path, dp->d_name);

        // open that "file" 
        if ((fd = open(buf, O_RDONLY)) < 0) {
            fprintf(stderr, "Can not open the file %s\n", buf);
        };

        if (fstat(fd, &finfo) < 0) {
            fprintf(stderr, "Can not get stat for");
            exit(EXIT_FAILURE);
        };
        printf("%5d: [%30s] \t %lu \t %d\n", fd, buf, finfo.st_ino, finfo.st_mode);

        /* symlink? */


        /* compare if regualr files */
        if (S_ISREG(finfo.st_mode)) { // nice macro
            check_match(fd, target_fd);
        }

        /* recursion logic */
        if (S_ISDIR(finfo.st_mode)) {
            // if it is a directory
            // (need to loop through that directory again)
            tree(buf, target_fd);
        }
    }

    return 1;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("that ain't how you use this");
        return -1;
    }
    
    /* get info about the target file */
    struct stat target_finfo;
    int target_fd = open(argv[1], O_RDONLY);
    // fstat(target_fd, &target_finfo);

    // tree broom broom!!
    tree(argv[2], target_fd);

    return 0;
}

// check if fd1 is the same as target file, fd2
int check_match(int fd1, int fd2) {
    // identical := same number of bytes && every byte matches
    // no need to consider sparse allocation
    
    struct stat stat1, stat2;
    char buf1[BUFSIZE];
    char buf2[BUFSIZE];

    if (fstat(fd1, &stat1) < 0) {
        fprintf(stderr, "Can not read fd1: %d", fd1);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd2, &stat2) < 0) {
        fprintf(stderr, "Can not read fd2: %d", fd2);
        exit(EXIT_FAILURE);
    }

    // compare size
    if (stat1.st_size != stat2.st_size) {
        return -1;
    }

    // compare bytes
    int n1, n2;
    while((n1 = ensure_read(fd1, buf1, sizeof(buf1))) > 0) {
        n2 = ensure_read(fd2, buf2, sizeof(buf2));

        if (n1 != n2) {
            return -1;
        }

        if ((n1 < 0) || (n1 < 0)) {
            return -1;
        }
        
        for(int i = 0; i < n1; i++) {
            if (buf1[i] != buf2[i]) {
                lseek(fd1, 0, SEEK_SET);
                lseek(fd2, 0, SEEK_SET);

                return -1;
            }
        }
    }

    printf("match!!\n");

    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);


    return 1;
}

// this deals with the partial read case.
// it returns number of bytes read (this matters because of EOF case)
// if read error, returns -1
int ensure_read(int fd, char *buf, ssize_t size) {
    // read return type is ssize_t
    ssize_t total = 0;
    ssize_t n;

    // read to buf until size amount from fd is read
    while(total < size) {
        // read from where it left off.
        n = read(fd, &(buf[total]), size - total);

        if (n < 0) { // Failed Reading
            fprintf(stderr, "");
            return -1;
        }

        if (n == 0) { // EOF
            return total;
        }

        total += n;
    }
    return total;
}
