#ifndef __BROWSE_UTILS
#define __BROWSE_UTILS

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>

mode_t get_mode(char* fname);
int pop_dir(char *str);
int push_dir(char *str, char *dir);
int exclude_dotdirs(const struct dirent *entry);
int abs_to_rel(char *rel, char *abs);
int suffindx(char *fname);
int split(char *str, char ***words, char *delim);

#define MAX_CMD 4096

typedef enum {
   TAR,
   TXT
} suffix;

mode_t get_mode(char* fname)
{
    struct stat entry_stat;
    if(lstat(fname, &entry_stat) == -1){
        perror("in get_mode");
        return -1;
    }else{
        return entry_stat.st_mode;
    }
}

int pop_dir(char* path)
{
    int depth = 0;
    if(strcmp(path, "/") != 0){
        int last_slash = 0;
        for(int i = 0; i < strlen(path); i++)
           if(path[i] == '/'){
               last_slash = i;
               depth++;
           }
        path[last_slash] = '\0';
    }
    return depth;
}

// add directory to absolute path (returns new depth from root) (error: -1)
int push_dir(char* path, char* dir)
{
    if(!path || !dir || strlen(path) + strlen(dir) + 2 > PATH_MAX)
        return -1;
    strcat(path, "/");
    strcat(path, dir);

    int depth = 0;
    for(int i = 0; i < strlen(path); i++)
        if(path[i] == '/')
            depth++;
    return depth;
}

int exclude_dotdirs(const struct dirent* entry){
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        return 1;
    }else{
        return 0;
    }
}
int abs_to_rel(char* rel, char* abs)
{
    if(!rel || !abs)
        return -1;
    int last_slash = 0;
    for(int i = 0; i < strlen(abs); i++)
        if(abs[i] == '/')
            last_slash = i;
    if(last_slash == 0 && abs[last_slash] != '/'){
        // path is already relative
        strncpy(rel, abs, FILENAME_MAX);
    }
    strncpy(rel, &abs[last_slash]+1, FILENAME_MAX);
    return strlen(rel);
}


inline int suffindx(char *fname)
{
    for(int i = 0; i < strlen(fname); i++){
        if(fname[i] == '.')
            return i;
    }
    return 0;
}


int split(char *line, char ***words, char *delim)
{
    int nwords = 0;
    int last = 0;

    for(int i = 0; i < strlen(line); i++){
        for(int j = 0; j < strlen(delim); j++){
            if(line[i] == delim[j]){
                nwords++;
                if(((*words) = realloc((*words), nwords*sizeof(char*))) == NULL){
                    perror("failed to parse arguments");
                    return -1;
                }
                int len = i-last;
                (*words)[nwords-1] = malloc(len);
                if(last == 0)
                    strncpy((*words)[nwords-1], &line[last], len);
                else
                    strncpy((*words)[nwords-1], &line[last+1], len);
                last = i;
            }
        }
    }
    return nwords;
}




#endif
