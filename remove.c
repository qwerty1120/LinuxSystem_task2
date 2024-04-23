#include "repo_header.h"

int main(int argc, char *argv[]){
    Get_Path();
    Stag_Setting();

    if(argc<2){
        printf("ERROR : <PATH> is not include\n");
        printf("Usage : remove <PATH> record path to staging area, path will track modification.\n");
        exit(1);
    }
    if(strlen(argv[1])>PATHMAX){
        fprintf(stderr, "Input path must not exceed 4,096 bytes.\n");
        exit(1);
    }

    if(realpath(argv[1], FILEPATH)==NULL){
        fprintf(stderr, "ERROR : %s is wrong path.\n", argv[1]);
        exit(1);
    }

    if (strncmp(FILEPATH, EXEPATH, strlen(EXEPATH))
        || !strncmp(FILEPATH, REPOPATH, strlen(REPOPATH))
        || !strcmp(FILEPATH, EXEPATH))  {// 사용자 디렉토리 내부의 경로인지 확인
        fprintf(stderr, "ERROR: path must be in user directory\n - \'%s\' is not in user directory.\n", FILEPATH);
        exit(1);
    }

    struct Node * check = Find_Node(FILEPATH, Q->head);
    if(Check_Status(check, REM_CMD) == 0){
        printf("\".%s\" is already removed in staging area.\n", FILEPATH+strlen(EXEPATH));
        exit(1);
    }
    int fd;

    if((fd=open(STAGPATH, O_RDWR|O_APPEND)) < 0){
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }

    snprintf(BUF, strlen(FILEPATH)+11, "remove \"%s\"\n", FILEPATH);

    if(write(fd, BUF, strlen(BUF)) < 0){
        fprintf(stderr, "write error for %s\n", STAGPATH);
        exit(1);
    }

    printf("remove \".%s\"\n", FILEPATH+strlen(EXEPATH));
    exit(0);
}