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

int check_match(int fd1, int fd2);

int tree(char *path, int target_fd/* struct stat *target_finfo */) {
    struct dirent *dp;
    struct stat finfo;
    DIR *dir;

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
        char buf[4096];
        snprintf(buf, sizeof(buf), "%s/%s", path, dp->d_name);

        // open that "file" 
        int fd = open(buf, O_RDONLY);

        if (fstat(fd, &finfo) < 0) {
            fprintf(stderr, "Can not get stat for");
            exit(EXIT_FAILURE);
        };
        printf("%5d: [%30s] \t %lu \t %d\n", fd, buf, finfo.st_ino, finfo.st_mode);

        /* compare the files */
        check_match(fd, target_fd);

        /* recursion logic */
        if (S_ISDIR(finfo.st_mode)) { // nice macro
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

int check_match(int fd1, int fd2) {
    // identical := same number of bytes && every byte matches
    // no need to consider sparse allocation
    
    // probably should think about some smart way of doing this.

    struct stat stat1, stat2;

    if (fstat(fd1, &stat1) < 0) {
        fprintf(stderr, "Can not read fd1: %d", fd1);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd2, &stat1) < 0) {
        fprintf(stderr, "Can not read fd2: %d", fd2);
        exit(EXIT_FAILURE);
    }

    if (stat1.st_size == stat2.st_size) {
        printf("same size!");
    }

    return 1;
}
