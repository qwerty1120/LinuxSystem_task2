#define OPENSSL_API_COMPAT 0x10100000L

#ifndef REPO_HEADER_H
#define REPO_HEADER_H
//#pragma once
#include <stdio.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <wait.h>
#define true 1

#define false 0

#define UNTRACKTED 0
#define ADD_CMD 1
#define REM_CMD 2

#define PATHMAX 4096
#define STRMAX 255

typedef char bool;
extern int COMMAND_CNT;

extern char EXEPATH[PATHMAX];
extern char REPOPATH[PATHMAX];
extern char COMMITPATH[PATHMAX];
extern char STAGPATH[PATHMAX];
extern char FILEPATH[PATHMAX];
extern char inputBuf[PATHMAX*4];
extern char BUF[PATHMAX*2];
extern char BUF1[PATHMAX*2];

extern char *COMMAND_SET[];

struct Node{
    int mode;
    int status;//status가 true 이면 mode 와 상관없이 child를 볼 필요가 있다. dir가 remove 된 후 안의 파일이 add 된 경우 status가 true가 된다.
    bool isdir;
    char realpath[PATHMAX];
    char backupname[STRMAX];
    struct Node *next;
    struct Node *prev;
    struct Node *child;
    struct Node *parent;
};

struct List{
    struct Node *head;
    struct Node *tail;
};

struct node{
    char path[PATHMAX];
    struct node* next;
    struct node* prev;
};

struct list{
    struct node* head;
    struct node* tail;
};

extern struct list *NEW;
extern struct list *MDF;
extern struct list *REM;
extern struct list *UNT;

extern struct List *Q;

int md5(char *target_path, char *hash_result);

void Get_Path();

struct list * node_Init(struct list *new);
struct List * List_Init(struct List * Q);
void Status_Init();

void Stag_Setting();

//read staging log function
char Read_One (int fd);
int Read_Delim(int fd, char *buf, char delim);

int Read_Line(int fd, char *buf, int mode);
//linked list atomic function
struct Node * Find_Node(char *path, struct Node* start);
void Insert_Node(struct Node* curr, char *path);
void Insert_Recur(struct Node *curr, char *path);
void List_Setting();
int Cmd_File_Switch(int command, struct Node *start);
int Cmd_Recur_Switch(int command, struct Node *start);
int Check_Status(struct Node *start, int command);


char * Commit_Path(char * name, char * path);
void Make_Commit(struct Node *start, char *name);
void Print_Commit(char *name);
void Commit(char *name);

void File_Status(struct Node *file);
void Status_Check(struct Node *start);
void Print_Status();

void Print_Log(char *name);
#endif //REPO_HEADER_H