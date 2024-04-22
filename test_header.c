#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
//#include <openssl/md5>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <wait.h>

#define true 1
#define false 0

#define ADD_CMD 1
#define REM_CMD 2

#define PATHMAX 4096
#define STRMAX 255

typedef char bool;
int COMMAND_CNT=8;

char EXEPATH[PATHMAX] = "/home/lsnov/task2";
char REPOPATH[PATHMAX];
char COMMITPATH[PATHMAX];
char STAGPATH[PATHMAX];
char FILEPATH[PATHMAX];
char inputBuf[PATHMAX*4];
char BUF[PATHMAX];

char *COMMAND_SET[] = {
        "add",
        "remove",
        "status",
        "commit",
        "revert",
        "log",
        "help",
        "exit",
};

struct Node{
    int mode;
    bool isdir;
    char realpath[PATHMAX];
    struct Node *next;
    struct Node *prev;
    struct Node *child;
    struct Node *parent;
};

struct List{
    struct Node *head;
    struct Node *tail;
};

struct List *Q;

struct Node* Find_Node(char *path){
    struct Node *curr = Q->head->next;
    while(curr->next!=NULL){
        if(!strcmp(curr->realpath, path)){
            return curr;
        }
        curr=curr->next;
    }
    return NULL;//없는걸 삭제하려고함
}
//list 초기화 세팅
void List_Init(){
    Q = (struct List*)malloc(sizeof(struct List));
    Q->head = (struct Node*)malloc(sizeof(struct Node));
    Q->tail = (struct Node*)malloc(sizeof(struct Node));
    Q->head->child = (struct Node*)malloc(sizeof(struct Node));
    Q->head->parent = (struct Node*)malloc(sizeof(struct Node));
    Q->tail->next = NULL;
    Q->head->prev = NULL;
    Q->head->child = NULL;
    Q->head->parent = NULL;
    Q->head->next = Q->tail;
    Q->tail->prev = Q->head;
    strcpy(Q->head->realpath, EXEPATH);
    Q->head->isdir = false;
}

//int Insert_Node(char *path, struct Node *curr, int newDir){
//    if(Find_Node(path)!=NULL){
//        return -1;
//    }
//
//    struct Node *new = (struct Node*)malloc(sizeof(struct Node));
//    strcpy(new->realpath, path);
//    if(newDir == true){
//
//    }
//    else if(newDir == false)
//        if(curr->parent != NULL) new->parent = curr->parent;//경우에 따라 curr이 새로운 dir이고 new가 child가 되어야하는 상황처리 해줘야함
//        else new->parent = curr;// Q의 head 인 경우
//
//        new->prev= curr->prev;
//        curr->prev->next = new;
//        curr->prev=new;
//        new->next = curr;
//    return 0;
//}
//struct Node* Insert_Dir(struct Node *curr, char * path){ // curr == 해당 리스트의 tail
//    struct Node *new = (struct Node *)malloc(sizeof(struct Node));
//
//    if(curr->parent != NULL) new->parent = curr->parent;
//    else new->parent = curr;
//
//    int cnt;
//    struct stat stbuf;
//    struct dirent **namelist;
//    char buf[PATHMAX];
//
//    if(lstat(path, &stbuf) < 0) {
//        fprintf(stderr, "lstat error for %s\n", path);
//        exit(1);
//    }
//
//    if(S_ISDIR(stbuf.st_mode)) {
//        Insert_Node(path)+1;
//
//        if ((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
//            fprintf(stderr, "ERROR : scandir error for %s\n", path);
//            exit(1);
//        }
//        for (int i = 0; i < cnt; i++) {
//            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
//                continue;
//            sprintf(buf, "%s/%s", path, namelist[i]->d_name);
//            Insert_Recur(buf);
//        }
//    }
//}

void Insert_Node(struct Node *curr, char *path){
    struct Node * new = (struct Node *)malloc(sizeof(struct Node));

    strcpy(new->realpath, path);

    if(curr->child != NULL){
        new->parent = curr;
        curr->child->next = new;
        new->prev = curr->child;
        curr->child = new;
    }
    else {
        curr->child = new;
        new->parent = curr;
    }
}

void Insert_Recur(struct Node *curr, char *path){
    int cnt;
    struct stat stbuf;
    struct dirent **namelist;
    char buf[PATHMAX];

    if(lstat(path, &stbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", path);
        exit(1);
    }
    if(S_ISDIR(stbuf.st_mode)) {
        Insert_Node(curr, path);
        curr->child->isdir=true;

        if ((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
            fprintf(stderr, "ERROR : scandir error for %s\n", path);
            exit(1);
        }
        for (int i = 0; i < cnt; i++) {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
                continue;
            sprintf(buf, "%s/%s", path, namelist[i]->d_name);
            Insert_Recur(curr->child, buf);
        }
    }
    else if(S_ISREG(stbuf.st_mode)) {
        Insert_Node(curr, path);
        curr->child->isdir = false;
    }
}

void List_Setting(){
    int cnt;
    struct dirent **namelist;
    char buf[PATHMAX*2];

    List_Init();

    if ((cnt = scandir(EXEPATH, &namelist, NULL, alphasort)) == -1) {
        fprintf(stderr, "ERROR : scandir error for %s\n", EXEPATH);
        exit(1);
    }
    for (int i = 0; i < cnt; i++) {
        if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
            continue;
        sprintf(buf, "%s/%s", EXEPATH, namelist[i]->d_name);
        Insert_Recur(Q->head, buf);
    }
}
int main(){
    List_Setting();

}