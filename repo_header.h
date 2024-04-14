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

#define ADD_CMD 1
#define REM_CMD 2

#define PATHMAX 4096
#define STRMAX 255

extern int COMMAND_CNT;

extern char EXEPATH[PATHMAX];
extern char REPOPATH[PATHMAX];
extern char COMMITPATH[PATHMAX];
extern char STAGPATH[PATHMAX];
extern char FILEPATH[PATHMAX];
extern char inputBuf[PATHMAX*4];
extern char BUF[PATHMAX];

extern char *COMMAND_SET[];

struct stagNode;
struct stagList;
extern struct stagList *staglist;

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
int Remove_Recur(char *path);

#endif //TASK2_REPO_HEADER_H
