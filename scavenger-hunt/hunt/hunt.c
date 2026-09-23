#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 4096

/* ========== Function Prototypes ========== */
int check_match(int fd1, int fd2);
int ensure_read(int fd, char *buf, ssize_t size);
int tree(char *path, int target_fd, struct stat *target_stat);
int check_lseek(int fd1, int fd2);
/* ========================================= */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("that ain't how you use this");
        return -1;
    }
    
    /* get info about the target file */
    struct stat target_stat;
    int target_fd;

    if ((target_fd = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Can not open %s: %s\n", argv[1], strerror(errno));
        return -1;
    }
    
    if (fstat(target_fd, &target_stat) < 0) {
        fprintf(stderr, "Can not stat target %s: %s\n", argv[1], strerror(errno));
        close(target_fd);
        return -1;
    }

    tree(argv[2], target_fd, &target_stat);
    close(target_fd);

    return 0;
}

int tree(char *path, int target_fd, struct stat *target_stat) {
    struct dirent *dp;
    struct stat finfo;
    DIR *dir;
    int fd;

    /* open the directory */
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "Can not open directory %s: %s\n", path, strerror(errno));
        return -1;
    }

    /* loop through the directory */
    while((dp = readdir(dir)) != NULL) {
        // skip . and ..
        if ( (strcmp(".", dp->d_name) == 0) || (strcmp("..", dp->d_name) == 0) ) {
            continue;
        }

        // build a full path
        char path_buf[BUFSIZE];
        int n = snprintf(path_buf, sizeof(path_buf), "%s/%s", path, dp->d_name);
        if (n < 0 || (unsigned long)n >= sizeof(path_buf)) {
            fprintf(stderr, "Path too long\n");
            continue;
        }


        if (lstat(path_buf, &finfo) < 0) {
            fprintf(stderr, "Can not get stat for %s: %s\n", path_buf, strerror(errno));
            continue;
        }

        printf("[%30s] \t %lu \t %d\n", path_buf, finfo.st_ino, finfo.st_mode);

        /* symlink specific handling */
        if (S_ISLNK(finfo.st_mode)) {
            struct stat link_stat;
            fprintf(stdout, "\tlink!\n");

            /* Follow the symlink
             *  - if it is the same inode as the original file, report
             *  - if check_match returns true, match
             */

            if (stat(path_buf, &link_stat) < 0) {
                fprintf(stderr, "Can not get stat for %s: %s\n", path_buf, strerror(errno));
            } else {
                printf("\t%lu\n", link_stat.st_ino);

                // compare link_stat with target_stat
                if ((link_stat.st_ino == target_stat->st_ino) && (link_stat.st_dev == target_stat->st_dev)) {
                    fprintf(stdout, "\tSYMLINK TO ORIGINAL FILE\n");
                    continue;
                }

                // if symlink goes to regular file, check
                if (S_ISREG(link_stat.st_mode)) {
                    if (link_stat.st_size != target_stat->st_size) {
                        continue;
                    }

                    if ((fd = open(path_buf, O_RDONLY)) < 0) {
                        fprintf(stderr, "Can not open the file %s: %s\n", path_buf, strerror(errno));
                        continue;
                    };

                    if (check_match(fd, target_fd) > 0) {
                        char link_buf[BUFSIZE];

                        ssize_t len = readlink(path_buf, link_buf, sizeof(link_buf) - 1);
                        
                        if (len < 0) {
                            fprintf(stderr, "Can not readlink %s: %s\n", path_buf, strerror(errno));
                        } else {
                            link_buf[len] = '\0';
                            printf("\tRegular Match: %s\n", link_buf);
                        }
                    }

                    close(fd);
                }
            }

            continue; // skip
        }

        /* compare if regualr files */
        if (S_ISREG(finfo.st_mode)) {
            /* hard link */
            if ((finfo.st_ino == target_stat->st_ino) && (finfo.st_dev == target_stat->st_dev)) {
                
                if (target_stat->st_nlink > 1) {
                    fprintf(stdout, "\tHARD LINK TO TARGET: %s\n", path_buf);
                }

                continue;
            }

            // this just can not be identical
            if (finfo.st_size != target_stat->st_size) {
                continue;
            }

            // open that "file" 
            if ((fd = open(path_buf, O_RDONLY)) < 0) {
                fprintf(stderr, "Can not open the file %s: %s\n", path_buf, strerror(errno));
                continue;
            };

            /* check match */
            if (check_match(fd, target_fd) > 0) {
                printf("\tSYMLINK CONTENTS\n");
            }

            close(fd);
        }

        /* recursion logic */
        if (S_ISDIR(finfo.st_mode)) {
            // if it is a directory, loop throught that directory
            tree(path_buf, target_fd, target_stat);
        }

    }

    closedir(dir);
    return 1;
}


// check if fd1 is the same as target file, fd2
// returns 0 if different, -1 if error, 1 if same
int check_match(int fd1, int fd2) {
    // identical := same number of bytes && every byte matches
    // no need to consider sparse allocation
    
    struct stat stat1, stat2;
    char buf1[BUFSIZE];
    char buf2[BUFSIZE];

    /* get stat for fd1 and fd2 */
    if (fstat(fd1, &stat1) < 0) {
        fprintf(stderr, "Can not read fd1 %d: %s\n", fd1, strerror(errno));
        return -1;
    }

    if (fstat(fd2, &stat2) < 0) {
        fprintf(stderr, "Can not read fd2 %d: %s\n", fd2, strerror(errno));
        return -1;
    }

    /* copmare size */
    if (stat1.st_size != stat2.st_size)
        return 0;

    /* compare bytes */
    int n1, n2;
    if (check_lseek(fd1, fd2) < 0) {
        return -1;
    }

    while(1) {
        n1 = ensure_read(fd1, buf1, sizeof(buf1));
        if (n1 < 0) 
            return -1;

        n2 = ensure_read(fd2, buf2, sizeof(buf2));
        if (n2 < 0) 
            return -1;

        if (n1 != n2) 
            return 0;

        if (n1 == 0) 
            break;
        
        for(int i = 0; i < n1; i++) {
            if (buf1[i] != buf2[i]) {
                return 0;
            }
        }
    }

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
            fprintf(stderr, "Can not read the file \n");
            return -1;
        }

        if (n == 0) { // EOF
            return total;
        }

        total += n;
    }

    return total;
}

int check_lseek(int fd1, int fd2) {
    if (lseek(fd1, 0, SEEK_SET) == (off_t)-1) {
        fprintf(stderr, "lseek fd1: %s\n", strerror(errno));
        return -1;
    }

    if (lseek(fd2, 0, SEEK_SET) == (off_t)-1) {
        fprintf(stderr, "lseek fd2: %s\n", strerror(errno));
        return -1;
    }

    return 1;
}
