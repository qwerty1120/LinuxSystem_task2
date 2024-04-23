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
//#include <openssl/md5>
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
extern char BUF[PATHMAX];

extern char *COMMAND_SET[];

//struct Node;
//struct List;

struct Node{
    int mode;
    int status;//status가 true 이면 mode 와 상관없이 child를 볼 필요가 있다. dir가 remove 된 후 안의 파일이 add 된 경우 status가 true가 된다.
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
extern struct List *Q;

void Get_Path();

void List_Init();

void Stag_Listing();

//read staging log function
char Read_One (int fd);
int Read_Delim(int fd, char *buf, char delim);
int Read_Line(int fd, char *buf);

//linked list atomic function
struct Node * Find_Node(char *path, struct Node* start);
void Insert_Node(struct Node* curr, char *path);
void Insert_Recur(struct Node *curr, char *path);
void List_Setting();
int Cmd_File_Switch(int command, struct Node *start);
int Cmd_Recur_Switch(int command, struct Node *start);
void Stag_Setting();

#endif //REPO_HEADER_H
