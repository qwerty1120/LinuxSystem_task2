#include "repo_header.h"
int main(int argc, char *argv[]){
    Get_Path();
    if(argc == 1){
        Print_Log("");
    }
    else if(argc == 2){
        Print_Log(argv[1]);
    }
}