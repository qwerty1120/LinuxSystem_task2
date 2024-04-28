#include "repo_header.h"

#define true 1
#define false 0

#define HASH_MD5  33

#define UNTRACKED 0
#define ADD_CMD 1
#define REM_CMD 2

#define STAG_MOD 1
#define CMT_MOD 2
#define LOG_MOD 3

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
char BUF[PATHMAX*2];
char BUF1[PATHMAX*2];

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

struct List *Q;

struct list *NEW;
struct list *MDF;
struct list *REM;
struct list *UNT;

struct List *Commit_Q;

int md5(char *target_path, char *hash_result) {
    FILE *fp;
    unsigned char hash[MD5_DIGEST_LENGTH];
    unsigned char buffer[SHRT_MAX];
    int bytes = 0;
    MD5_CTX md5;

    if ((fp = fopen(target_path, "rb")) == NULL) {
        printf("ERROR: fopen error for %s\n", target_path);
        return 1;
    }

    MD5_Init(&md5);

    while ((bytes = fread(buffer, 1, SHRT_MAX, fp)) != 0)
        MD5_Update(&md5, buffer, bytes);

    MD5_Final(hash, &md5);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(hash_result + (i * 2), "%02x", hash[i]);
    hash_result[HASH_MD5 - 1] = 0;
    fclose(fp);

    return 0;
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
int Read_Line(int fd, char *buf, int mode){
    int ret = -1;
    int check;
    if(mode == STAG_MOD) {
        if ((check = Read_Delim(fd, buf, ' ')) == 0) {
            return 0;
        } else if (check < 0) {
            fprintf(stderr, "read error for %s\n", STAGPATH);
            exit(1);
        }

        if (!strcmp(buf, COMMAND_SET[0])) {
            ret = ADD_CMD;
        } else if (!strcmp(buf, COMMAND_SET[1])) {
            ret = REM_CMD;
        }
        Read_One(fd);
        Read_Delim(fd, buf, '\"');
        Read_One(fd);
    }
    else if(mode == CMT_MOD || mode == LOG_MOD){
        if ((check = Read_Delim(fd, buf, ' ')) == 0) {
            return 0;
        } else if (check < 0) {
            fprintf(stderr, "read error for %s\n", COMMITPATH);
            exit(1);
        }
        ret = 1;
        Read_One(fd);

        Read_Delim(fd, buf, '\"');//commit name
        Read_One(fd);
        Read_Delim(fd, inputBuf, ' ');
        Read_Delim(fd, inputBuf, ':');//command
        if(mode == LOG_MOD) strcpy(BUF1, inputBuf);
        if(strcmp(inputBuf, "removed") != 0){
            Read_Delim(fd, inputBuf, '\"');
            Read_Delim(fd, inputBuf, '\"');//realpath
        }
        Read_One(fd);
    }
    return ret;
}
struct list * node_Init(struct list *new){
    new = (struct list*)malloc(sizeof(struct list));
    new->head = (struct node*)malloc(sizeof(struct node));
    new->tail = (struct node*)malloc(sizeof(struct node));
    new->head = new->tail;
    new->tail->prev = new->head;
    new->head->next=new->tail;
    new->tail->next = NULL;
    return new;
}
void Status_Init(){
    NEW = node_Init(NEW);
    MDF = node_Init(MDF);
    REM = node_Init(REM);
    UNT = node_Init(UNT);
}
//list 초기화 세팅
struct List * List_Init(struct List * Q){
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
    Q->head->status = true;
    return Q;
}

void Insert_Node(struct Node *curr, char *path){
    struct Node * new = (struct Node *)malloc(sizeof(struct Node));
    strcpy(new->realpath, path);
    new->mode = UNTRACKED;
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

    Q = List_Init(Q);

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
                Insert_Node(curr->parent, path);
                return curr->parent->child;
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
//staging list 세팅
void Stag_Setting(){
    int fd;
    int command;

    List_Setting();

    if((fd=open(STAGPATH, O_RDONLY)) < 0){
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }

    while((command = Read_Line(fd, BUF, STAG_MOD)) > 0){
        if(Cmd_Recur_Switch(command, Find_Node(BUF, Q->head)) < 0){
            printf("staging log error\n");
            exit(1);
        }
    }

    if((fd=open(COMMITPATH, O_RDONLY)) < 0){
        fprintf(stderr, "open error for %s\n", COMMITPATH);
        exit(1);
    }

    while((command = Read_Line(fd, BUF, CMT_MOD)) > 0){
        if(strcmp(inputBuf, "removed") != 0 ) {
            strcpy(Find_Node(inputBuf, Q->head)->backupname, BUF);
        }
        else continue;
    }
}
//add remove child와 parent 관계에 따른 예외 처리용
int Check_Status(struct Node *start, int command){
    int ret = 0;
    struct Node *curr = start;

    if(curr->mode != command){
        return 1;
    }
    if(curr->isdir == true){
        curr = curr->child;
        while(curr->prev != NULL){
            ret += Check_Status(curr, command);
            curr = curr->prev;
        }
    }
    else {
        return 0;
    }
    return ret;
}
//commit path로 변화
char * Commit_Path(char * name, char * path){
    sprintf(BUF, "%s/%s%s", REPOPATH, name, path+strlen(EXEPATH));
    return BUF;
}
//커밋 파일만들고 내용 복사
void File_Commit(char *realpath, char * commitpath){
    struct stat statbuf;
    int fd1, fd2;
    int len;
    char buf[PATHMAX*4];

    if((fd1=open(realpath,O_RDONLY))<0){
        fprintf(stderr, "commit error for %s\n", realpath);
    }
    if((fd2=open(commitpath,O_CREAT|O_RDWR, 0777))<0){
        fprintf(stderr, "commit error for %s\n", commitpath);
    }
    if (fstat(fd1, &statbuf) < 0) {
        fprintf(stderr, "stat error for %s\n", realpath);
        exit(1);
    }
    while ((len = read(fd1, buf, statbuf.st_size)) > 0) {//내용 복사
        if(write(fd2, buf, len) < 0){// 왜 루트권한으로 써지지...
            fprintf(stderr, "write error for %s\n", commitpath);
            exit(1);
        }
    }
}
void Make_Commit(struct Node *start, char *name){
    struct Node * curr = start;
    if(curr->isdir == true && (curr->status == true || curr->mode == ADD_CMD)){
        if(curr->child != NULL) {
            curr = curr->child;
            while (1) {
                Make_Commit(curr, name);
                if (curr->prev != NULL) {
                    curr = curr->prev;
                } else break;
            }
        }
    }
    else if(curr->isdir == false && curr->mode == ADD_CMD){
        if(!access(curr->realpath, F_OK)) {
            strcpy(BUF, Commit_Path(name, curr->realpath));
            for (size_t j = strlen(REPOPATH) + 1; BUF[j] != 0; j++) {// 디렉터리가 없으면 생성
                if (BUF[j] == '/') {
                    BUF[j] = 0;
                    if (access(BUF, F_OK)) {
                        mkdir(BUF, 0777);
                    }
                    BUF[j] = '/';
                }
            }
            File_Commit(curr->realpath, BUF);
        }
    }
}

void File_Status(struct Node *file){
    struct node * new = (struct node*)malloc(sizeof(struct node));
    strcpy(new->path, file->realpath);
    if(file->mode == UNTRACKED){
        UNT->tail->next = new;
        new->prev = UNT->tail;
        UNT->tail = UNT->tail->next;
        UNT->tail->next = NULL;
    }
    else if(file->mode == ADD_CMD){
        if(access(file->realpath, F_OK)) {
            printf("%s\n", file->realpath);
            REM->tail->next = new;
            new->prev = REM->tail;
            REM->tail = REM->tail->next;
            REM->tail->next = NULL;
        }
        else {
            if(!strcmp(file->backupname, "")){
                NEW->tail->next = new;
                new->prev = NEW->tail;
                NEW->tail = NEW->tail->next;
                NEW->tail->next = NULL;
            }
            else {
                md5(Commit_Path(file->backupname, file->realpath), inputBuf);
                md5(file->realpath, BUF);
                if(strncmp(BUF, inputBuf, MD5_DIGEST_LENGTH) != 0){
                    MDF->tail->next = new;
                    new->prev = MDF->tail;
                    MDF->tail = MDF->tail->next;
                    MDF->tail->next = NULL;
                }
            }
        }
    }
}
void Status_Check(struct Node *start){
    struct Node * curr = start;
    if(curr->isdir == true){
        if(curr->child != NULL) {
            curr = curr->child;
            while (1) {
                if(curr->isdir == false && curr->mode != REM_CMD){
                    File_Status(curr);
                }
                if (curr->prev != NULL) {
                    curr = curr->prev;
                } else break;
            }
            curr = start->child;
            while(1){
                if(curr->isdir == true){
                    Status_Check(curr);
                }
                if (curr->prev != NULL) {
                    curr = curr->prev;
                } else break;
            }
        }
    }
}

void Print_Status(){
    Status_Init();
    Status_Check(Q->head);
    if(NEW->head->next != NULL || MDF->head->next != NULL || REM->head->next != NULL){
        printf("Changes to be committed: \n");
        if(MDF->head->next != NULL){
            struct node *curr = MDF->head->next;
            printf("  Modified: \n");
            while(1){
                printf("\t\".%s\"\n", curr->path + strlen(EXEPATH));
                if(curr->next == NULL) break;
                curr=curr->next;
            }
            free(curr);
            printf("\n");
        }
        if(REM->head->next != NULL){
            struct node *curr = REM->head->next;
            printf("  Removed: \n");
            while(1){
                printf("\t\".%s\"\n", curr->path + strlen(EXEPATH));
                if(curr->next == NULL) break;
                curr=curr->next;
            }
            free(curr);
            printf("\n");
        }
        if(NEW->head->next != NULL){
            struct node *curr = NEW->head->next;
            printf("  New file: \n");
            while(1){
                printf("\t\".%s\"\n", curr->path + strlen(EXEPATH));
                if(curr->next == NULL) break;
                curr=curr->next;
            }
            free(curr);
            printf("\n");
        }
    }
    else {printf("Nothing to commit\n");}
    if(UNT->head->next != NULL){
        printf("Untraked files: \n");
        struct node *curr = UNT->head->next;
        printf("  New file: \n");
        while(1){
            printf("\t\".%s\"\n", curr->path + strlen(EXEPATH));
            if(curr->next == NULL) break;
            curr=curr->next;
        }
        free(curr);
        printf("\n");
    }
}
void Print_Commit(char *name){
    int fd;

    if((fd=open(COMMITPATH, O_RDWR|O_APPEND)) < 0){
        fprintf(stderr, "open error for %s\n", COMMITPATH);
        exit(1);
    }
    printf("commit to \"%s\"\n", name);
    if(MDF->head->next != NULL){
        struct node *curr = MDF->head->next;
        while(1){
            printf("\tmodified: \".%s\"\n", curr->path + strlen(EXEPATH));
            sprintf(BUF, "commit: \"%s\" - modified: \"%s\"\n", name, curr->path);
            write(fd, BUF, strlen(BUF));
            if(curr->next == NULL) break;
            curr=curr->next;
        }
        free(curr);
        printf("\n");
    }
    if(REM->head->next != NULL){
        struct node *curr = REM->head->next;
        while(1){
            printf("\tremoved: \".%s\"\n", curr->path + strlen(EXEPATH));
            sprintf(BUF, "commit: \"%s\" - removed: \"%s\"\n", name, curr->path);
            write(fd, BUF, strlen(BUF));
            if(curr->next == NULL) break;
            curr=curr->next;
        }
        free(curr);
        printf("\n");
    }
    if(NEW->head->next != NULL){
        struct node *curr = NEW->head->next;
        while(1){
            printf("\tnew file: \".%s\"\n", curr->path + strlen(EXEPATH));
            sprintf(BUF, "commit: \"%s\" - new file: \"%s\"\n", name, curr->path);
            write(fd, BUF, strlen(BUF));
            if(curr->next == NULL) break;
            curr=curr->next;
        }
        free(curr);
        printf("\n");
    }
}
void Commit(char *name){
    Status_Init();
    Status_Check(Q->head);
    if(NEW->head->next != NULL || MDF->head->next != NULL || REM->head->next != NULL){
        Print_Commit(name);
        Make_Commit(Q->head, name);
    }
    else {
        printf("Nothing to commit\n");
    }
}

void Print_Log(char *name){
    int fd;
    int command = 0, cnt;
    char backupname[STRMAX]={0};

    if((fd=open(COMMITPATH, O_RDONLY)) < 0){
        fprintf(stderr, "open error for %s\n", COMMITPATH);
        exit(1);
    }

    if(strcmp(name, "") != 0){
        strcpy(backupname, name);
        printf("commit: \"%s\"\n", name);
        command = true;
    }

    while((cnt = Read_Line(fd, BUF, LOG_MOD)) > 0){
        if(!strcmp(backupname, BUF)){
            printf("\t- %s: \"%s\"\n", BUF1, inputBuf);
        }
        else if(!command){
            printf("commit: \"%s\"\n", BUF);
            strcpy(backupname, BUF);
            printf("\t- %s: \"%s\"\n", BUF1, inputBuf);
        }
    }
}