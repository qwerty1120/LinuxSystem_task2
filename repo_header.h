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
#ifndef TASK2_REPO_HEADER_H
#define TASK2_REPO_HEADER_H

#define ADD_CMD 1
#define REM_CMD 2

#endif //TASK2_REPO_HEADER_H

#define PATHMAX 4096
#define STRMAX 255
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

void Get_Path();

void stagList_Init();

void Stag_Listing();

//read staging log function
char Read_One (int fd);
int Read_Delim(int fd, char *buf, char delim);
int Read_Line(int fd, char *buf);

//linked list atomic function
struct stagNode* Find_Node(char *path);
int Insert_Node(char *path);
int Insert_Recur(char *path);
int Remove_Node(char *path);
