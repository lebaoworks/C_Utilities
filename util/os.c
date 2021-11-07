#include "os.h"
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include <stdio.h>
int mkdirs(char *dir, __mode_t mode)
{
    size_t len = strlen(dir);
    if (len >= PATH_MAX)
        return false;
    if (mode == 0)
        mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    char tmp[PATH_MAX];
    memcpy(tmp, dir, len + 1); // 1 for null
    if (tmp[len-1] == '/')
        tmp[len-1] = 0;
    for (char *p=tmp+1; *p; p++)
        if (*p == '/') {
            *p = 0;
            printf("dir: %s\n", tmp);
            if (mkdir(tmp, mode) != 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    printf("dir: %s\n", tmp);
    if (mkdir(tmp, mode) != 0 && errno != EEXIST)
        return -1;
    return 0;
}

char* dirname(char* path)
{
    char *lastSlash = strrchr(path, '/');
    if (lastSlash == NULL)
        return NULL;
    
    size_t len = (size_t) (lastSlash - path);
    char *ret = (char*) malloc(len+1);
    if (ret == NULL)
        return NULL;

    memcpy(ret, path, len);
    ret[len] = 0;
    return ret;
}