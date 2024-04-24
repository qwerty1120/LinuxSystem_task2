#include "repo_header.h"

int main(int argc, char *argv[]){
    Get_Path();
    Stag_Setting();
    Commit_Setting();

    if(argc<2){
        printf("ERROR : <NAME> is not include\n");
        printf("Usage : add <NAME> backup staging area with commit name.\n");
        exit(1);
    }
    if(strlen(argv[1])>PATHMAX){
        fprintf(stderr, "Input path must not exceed 4,096 bytes.\n");
        exit(1);
    }
    if(Name_Check(argv[1]) < 0){
        printf("%s is already exist in repo\n", argv[1]);
        exit(1);
    }
    if(Multiple_Check(argv[1]) < 0){
        printf("Nothing to commit\n");
        exit(1);
    }
    else {
        Print_Commit(argv[1]);
    }
}