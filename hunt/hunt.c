#include <stdio.h>
#include <fcntl.h>
// #include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/stat.h>
#include <string.h>

DIR *dir;
char buf[4096];

int tree(char *direct) {
    struct dirent *dp;
    struct stat finfo;

    if ((dir = opendir(direct)) == NULL) {
        fprintf(stderr, "cannot open %s\n", direct);
        return -1;
    }

    while((dp = readdir(dir)) != NULL) {
        int fd = open(dp->d_name, O_RDONLY, 0666);
        fstat(fd, &finfo);
        
        if ((strcmp(".", dp->d_name) != 0) || (strcmp("..", dp->d_name) != 0)) {

            printf(" [%10s] \t %lu \t %d\n", dp->d_name ,finfo.st_ino, finfo.st_mode);
            
            // now I need to determine if it is a dir or not
            if ((finfo.st_mode & 0b0100000000000000) != 0) {
                // recursion
                snprintf(buf, sizeof(buf), "%s/%s", direct, dp->d_name);
                // to do that I need to recreate the whole path name
                tree(buf);
            }
        }
    }

    return 1;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("that ain't how you use this");
        return -1;
    }
    
    // tree broom broom!!
    tree(argv[2]);

    return 0;
}
