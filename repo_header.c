#include "repo_header.h"

#define true 1
#define false 0

#define UNTRACKED 0
#define ADD_CMD 1
#define REM_CMD 2

#define PATHMAX 4096
#define STRMAX 255

typedef char bool;
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
//
//struct Node{
//    int mode;
//    int status;//status가 true 이면 mode 와 상관없이 child를 볼 필요가 있다. dir가 remove 된 후 안의 파일이 add 된 경우 status가 true가 된다.
//    bool isdir;
//    char realpath[PATHMAX];
//    struct Node *next;
//    struct Node *prev;
//    struct Node *child;
//    struct Node *parent;
//};
//
//struct List{
//    struct Node *head;
//    struct Node *tail;
//};

struct List *Q;
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
    Q->head->isdir = true;
}

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

struct Node * Find_Node(char *path, struct Node* start){
    struct Node * curr = start;
    while(1){
        if(strcmp(path, curr->realpath) && !strncmp(path, curr->realpath, strlen(curr->realpath))){
            curr=curr->child;
        }
        else if(strncmp(path, curr->realpath, strlen(curr->realpath))){
            if(curr->prev != NULL){
                curr = curr->prev;
            }
            else {
                printf("listing error\n");
                exit(1);
            }
        }
        else if(!strcmp(path, curr->realpath)){
            return curr;
        }
    }
}
int Cmd_File_Switch(int command, struct Node *start){
    struct Node * curr = start;
    if(curr->mode == command){
        return 0;
    }
    curr->mode = command;

    if(command == ADD_CMD){
        while(curr->prev != NULL) {
            curr->status = true;
            curr = curr->parent;
        }
    }
    else if(command == REM_CMD){
        curr->mode = REM_CMD;
        curr->status = false;
    }
    return 1;
}
int Cmd_Recur_Switch(int command, struct Node *start){
    struct Node * curr = start;
    int ret;//ret이 최종적으로 0이면 cmd가 변경된 게 없다는 뜻 그러면 already...

    ret = Cmd_File_Switch(command, curr);

    if(curr->isdir == true){
        curr=curr->child;
        while(curr != NULL){
            if(curr->isdir == true) ret += Cmd_Recur_Switch(command, curr);
            else ret += Cmd_File_Switch(command, curr);
            if(curr->prev != NULL) curr = curr->prev;
            else break;
        }
    }
    return ret;
}
void Stag_Setting(){
    int fd;
    int command;

    List_Setting();

    if((fd=open(STAGPATH, O_RDONLY)) < 0){
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }

    while((command = Read_Line(fd, BUF)) > 0){
        if(Cmd_Recur_Switch(command, Find_Node(BUF, Q->head)) < 0){
            printf("staging log error\n");
            exit(1);
        }
    }
}