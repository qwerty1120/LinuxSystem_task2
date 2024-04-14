#include "repo_header.h"

int main(void){
    strcpy(STAGPATH, "test.txt");
    stagList_Init();
    char buf[4096];

//    //write
//    int fd2 = open(STAGPATH, O_RDWR);
//    strcpy(buf, "/home/lsnov/task2/a.txt")
//    write_line(fd2, buf, 1);
//    write_line(fd2, buf, 2);
//    close(fd2);
//    //write

    int fd = open(STAGPATH, O_RDONLY);
    int tmp;
    while((tmp = Read_Line(fd, buf)) > 0){
        printf("%d %s\n", tmp, buf);
    }

}

void write_line(int fd, char *buf, int command){
    char cmd[4096];
    if(command == ADD_CMD) strcpy(cmd, "add \"");
    else if(command ==REM_CMD) strcpy(cmd, "remove \"");
    strcat(buf, "\"\n");
    write(fd, cmd, strlen(cmd));
    write(fd, buf, strlen(buf));
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

//한 글자 버리는 데에 사용
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
    staglist->tail->prev = new;
    new->next = staglist->tail;
    return 0;
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

void Stag_Listing(){
    int fd;
    int command;

    if((fd=open(STAGPATH, O_RDONLY)) > 0){
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }
    while((command = Read_Line(fd, BUF)) > 0){
        switch (command){
            case ADD_CMD :
                if(Insert_Node(BUF) < 0){
                    fprintf(stderr, "staging insert duplication error\n");
                    exit(1);
                }
                break;
            case REM_CMD :
                if(Remove_Node(BUF) < 0){
                    fprintf(stderr, "staging remove duplication error\n");
                    exit(1);
                }
                break;
        }
    }
}

void stagList_Init(){
    staglist = (struct stagList*)malloc(sizeof(struct stagList));
    staglist->head = (struct stagNode*)malloc(sizeof(struct stagNode));
    staglist->tail = (struct stagNode*)malloc(sizeof(struct stagNode));
    staglist->tail->next = NULL;
    staglist->head->prev = NULL;
    staglist->head->next = staglist->tail;
    staglist->tail->prev = staglist->head;
}