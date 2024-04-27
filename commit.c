#include "repo_header.h"

int main(int argc, char *argv[]){
    Get_Path();
    Stag_Setting();

    if(argc<2){
        printf("ERROR : <NAME> is not include\n");
        printf("Usage : commit <NAME>: backup staging area with commit name.\n");
        exit(1);
    }

    int cnt;
    struct stat stbuf;
    struct dirent **namelist;
    char buf[PATHMAX];

    if ((cnt = scandir(".repo", &namelist, NULL, alphasort)) == -1) {
        fprintf(stderr, "ERROR : scandir error for .repo\n");
        exit(1);
    }
    for (int i = 0; i < cnt; i++) {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
            continue;
        if(!strcmp(argv[1], namelist[i]->d_name)){
            printf("\"%s\" commit is already exist in repo.\n", argv[1]);
            exit(1);
        }
    }
    Commit(argv[1]);
}