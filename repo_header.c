#include "repo_header.h"

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

struct stagNode{
    char realpath[PATHMAX];
    struct stagNode *next;
    struct stagNode *prev;
};

struct stagList{
    struct stagNode *head;
    struct stagNode *tail;
};

struct stagList *staglist;

//staglist 초기화 세팅
void stagList_Init(){
    staglist = (struct stagList*)malloc(sizeof(struct stagList));
    staglist->head = (struct stagNode*)malloc(sizeof(struct stagNode));
    staglist->tail = (struct stagNode*)malloc(sizeof(struct stagNode));
    staglist->tail->next = NULL;
    staglist->head->prev = NULL;
    staglist->head->next = staglist->tail;
    staglist->tail->prev = staglist->head;
}

//필요한 repo path들 가져오기
void Get_Path(){
    getcwd(EXEPATH,PATHMAX);
    strcpy(REPOPATH,EXEPATH);
    strcat(REPOPATH,"/.repo");

    snprintf(COMMITPATH, strlen(REPOPATH)+13, "%s/.commit.log", REPOPATH);
    snprintf(STAGPATH, strlen(REPOPATH)+14, "%s/.staging.log", REPOPATH);
}

/*

add, remove기준

노드 삽입 (char * str) => void  1/1/1
노드 탐색 (char * str) => node* 1/1/2/1
노드 삭제 (char * str) => void 1/1/2

fscanf(f, "%s%*c%s%*c", buf1, buf2);

특정 구분자가 있을 때 까지 읽기(구분자까지 읽는거)(int fd, char*buf, char delim) => void 1/2/1/1
파일 한줄읽기(이어서 읽기, 오프셋은 내부의 read로만 변경한다고 가정) (int fd, char *buf) => int 1/2/1
링크드 리스트 구성(int fd, node * head) => void 1

*/

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
struct stagNode* Find_Node(char *path){
    struct stagNode *curr = staglist->head->next;
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

    struct stagNode *new = (struct stagNode*)malloc(sizeof(struct stagNode));
    strcpy(new->realpath, path);

    new->prev= staglist->tail->prev;
    staglist->tail->prev->next = new;
    staglist->tail->prev=new;
    new->next = staglist->tail;
    return 0;
}

int Insert_Recur(char *path){
    int cnt=0, ret = 0;
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
    struct stagNode *rem = Find_Node(path);
    if(rem == NULL){
        return -1;
    }
    rem->prev->next=rem->next;
    rem->next->prev=rem->prev;
    free(rem);
    return 0;
}

int Remove_Recur(char *path){
    int cnt=0, ret = 0;
    struct stat stbuf;
    struct dirent **namelist;
    char buf[PATHMAX];

    if(lstat(path, &stbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", path);
        exit(1);
    }
    if(S_ISDIR(stbuf.st_mode)) {
        ret += Remove_Recur(path) + 1;
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

void Stag_Listing(){
    int fd;
    int command;
    struct stat stbuf;

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