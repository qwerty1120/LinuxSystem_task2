#include "repo_header.h"

int main(int argc, char *argv[]){
    Get_Path();
    Stag_Setting();
    Make_Commit(Q->head, argv[1]);
}