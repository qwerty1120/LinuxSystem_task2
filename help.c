#include "repo_header.h"
int main(int argc, char *argv[]){
    if(argc == 1){
        printf("Usage:\n");
        printf("\t> add <PATH> : record path to staging area, path will tracking modification\n");
        printf("\t> remove <PATH> : record path to staging area, path will not tracking modification\n");
        printf("\t> status : show staging area status\n");
        printf("\t> commit <NAME> : backup staging area with commit name\n");
        printf("\t> revert <NAME> : recover commit version with commit name\n");
        printf("\t> log : show commit log\n");
        printf("\t> help : show commands for program\n");
        printf("\t> exit : exit program\n");
    }
    else if(argc == 2){
        if(!strcmp(argv[1], COMMAND_SET[0])){
            printf("Usage: add <PATH> : record path to staging area, path will tracking modification\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[1])){
            printf("Usage: remove <PATH> : record path to staging area, path will not tracking modification\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[2])){
            printf("Usage: status : show staging area status\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[3])){
            printf("Usage: commit <NAME> : backup staging area with commit name\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[4])){
            printf("Usage: revert <NAME> : recover commit version with commit name\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[5])){
            printf("Usage: log : show commit log\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[6])){
            printf("Usage: help : show commands for program\n");
        }
        else if(!strcmp(argv[1], COMMAND_SET[7])){
            printf("Usage: exit : exit program\n");
        }
        else {
            printf("\"%s\" is wrong command\n", argv[1]);
        }
    }
    else {
        printf("too many argument\nUsage:\n\thelp: show commands for program\n\thelp [COMMAND]: show commands instruction\n");
    }
    exit(0);
}