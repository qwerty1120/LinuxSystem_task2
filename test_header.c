#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <wait.h>
//#include <openssl/md5>

int COMMAND_CNT=8;

char EXEPATH[PATHMAX];
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
}

//필요한 repo path들 가져오기
void Get_Path(){
    getcwd(EXEPATH,PATHMAX);
    strcpy(REPOPATH,EXEPATH);
    strcat(REPOPATH,"/.repo");

    snprintf(COMMITPATH, strlen(REPOPATH)+13, "%s/.commit.log", REPOPATH);
    snprintf(STAGPATH, strlen(REPOPATH)+14, "%s/.staging.log", REPOPATH);
}

//특정 구분자가 있을 때 까지 읽기
int Read_Delim(int fd, char *buf, char delim){
    char charbuf;
    int i = 0, ret;
    while((ret = read(fd, &charbuf, sizeof(char))) > 0){
        buf[i++] = charbuf;
        if(charbuf == delim){
            buf[i-1] = '\0';
            return ret;
        }
    }
    return ret;
}

//한 글자 읽는 데에 사용
char Read_One (int fd){
    char charbuf;
    read(fd, &charbuf, sizeof(char));
    return charbuf;
}

//한 줄을 읽어서 리스트에 추가 필요성 결정
int Read_Line(int fd, char *buf){
    int ret = -1;
    int check;

    if((check = Read_Delim(fd, buf, ' ')) == 0) {
        return 0;
    }
    else if(check < 0){
        fprintf(stderr, "read error for %s\n", STAGPATH);
        exit(1);
    }

    if(!strcmp(buf, COMMAND_SET[0])){
        ret = ADD_CMD;
    }
    else if(!strcmp(buf, COMMAND_SET[1])){
        ret = REM_CMD;
    }
    Read_One(fd);
    Read_Delim(fd, buf, '\"');
    Read_One(fd);

    return ret;
}

//리스트에 이미 노드가 있는지 탐색
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

//노드 삽입, 이미 있으면 리턴 -1
int Insert_Node(char *path){
    if(Find_Node(path)!=NULL){
        return -1;
    }

    struct Node *new = (struct Node*)malloc(sizeof(struct Node));
    strcpy(new->realpath, path);

    new->prev= Q->tail->prev;
    Q->tail->prev->next = new;
    Q->tail->prev=new;
    new->next = Q->tail;
    return 0;
}
//Dir 삽입, 이미 있으면 리턴 -1
int Insert_Dir(char *path){//todo
    if(Find_Node(path)!=NULL){
        return -1;
    }

    struct Node *new = (struct Node*)malloc(sizeof(struct Node));
    strcpy(new->realpath, path);
    new->isdir = true;
    if(Q->tail->prev != Q->head) new->parent = Q->tail->prev->parent;
    else new->parent = Q->head;

    new->prev= Q->tail->prev;
    Q->tail->prev->next = new;
    Q->tail->prev=new;
    new->next = Q->tail;
    return 0;
}

//노드를 재귀적으로 삽입
int Insert_Recur(char *path){
    int cnt, ret = 0;
    struct stat stbuf;
    struct dirent **namelist;
    char buf[PATHMAX];

    if(lstat(path, &stbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", path);
        exit(1);
    }
    if(S_ISDIR(stbuf.st_mode)) {
        ret+=Insert_Node(path)+1;

        if ((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
            fprintf(stderr, "ERROR : scandir error for %s\n", path);
            exit(1);
        }
        for (int i = 0; i < cnt; i++) {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
                continue;
            sprintf(buf, "%s/%s", path, namelist[i]->d_name);
            ret += Insert_Recur(buf);
        }
    }
    else if(S_ISREG(stbuf.st_mode)) {
        ret = Insert_Node(path) + 1;
    }
    return ret;
}

//노드 제거, 없는 노드일 경우 리턴 -1
int Remove_Node(char *path){
    struct Node *rem = Find_Node(path);
    if(rem == NULL){
        return -1;
    }
    rem->prev->next=rem->next;
    rem->next->prev=rem->prev;
    free(rem);
    return 0;
}

//노드를 재귀적으로 삭제
int Remove_Recur(char *path){
    int cnt, ret = 0;
    struct stat stbuf;
    struct dirent **namelist;
    char buf[PATHMAX];

    if(lstat(path, &stbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", path);
        exit(1);
    }
    if(S_ISDIR(stbuf.st_mode)) {
        ret += Remove_Node(path) + 1;
        if ((cnt = scandir(path, &namelist, NULL, alphasort)) == -1) {
            fprintf(stderr, "ERROR : scandir error for %s\n", path);
            exit(1);
        }
        for (int i = 0; i < cnt; i++) {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
                continue;
            sprintf(buf, "%s/%s", path, namelist[i]->d_name);
            ret += Remove_Recur(buf);
        }
    }
    else if(S_ISREG(stbuf.st_mode)) {
        ret = Remove_Node(path) + 1;
    }
    return ret;
}

//명령어에 따라 삽입, 삭제 실행
void Stag_Listing(){
    int fd;
    int command;

    if((fd=open(STAGPATH, O_RDONLY)) < 0){
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }
    while((command = Read_Line(fd, BUF)) > 0){
        switch (command){
            case ADD_CMD :
                if(!Insert_Recur(BUF)){
                    fprintf(stderr, "staging insert duplication error\n");
                    exit(1);
                }
                break;
            case REM_CMD :
                if(Remove_Recur(BUF) < 0){
                    fprintf(stderr, "staging remove duplication error\n");
                    exit(1);
                }
                break;
            default:
                exit(1);
        }
    }
}

//트리 구조로 리스트 만들기
void Tree_Listing(){

}