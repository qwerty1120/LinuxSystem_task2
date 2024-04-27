#include "repo_header.h"
int main(int argc, char *argv[]){
    Get_Path();
    Stag_Setting();

    if(argc>1){
        printf("Usage : status: show staging area status.\n");
        exit(1);
    }

    Print_Status();
}