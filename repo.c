#include "repo_header.h"

void Init();
int Command_Check(char *command);
int Input_Line();
int Prompt();

int main(int argc, char *argv[]){
    Init();
    while(Prompt());
}

int Command_Check(char *command){
    for(int i=0;i<COMMAND_CNT;i++){
        if(!strcmp(command,COMMAND_SET[i])) return i;
    }
    return -1;
}

int Prompt(){
    printf("20232898> ");

    int cnt = Input_Line();
    pid_t pid;
    char *command=strtok(inputBuf," ");

    if(Command_Check(command)<0){
        fprintf(stderr, "%s is not command\n", command);
        return -1;
    }

    if(!strcmp(COMMAND_SET[7],command)){
        return 0;
    }

    char **argv=(char**)malloc(sizeof(char*)*(cnt+1)), *argu;
    argv[0] = (char *) malloc(sizeof(char) * (strlen(command)+1));

    strcpy(argv[0], command);

    for(int i=1;(argu = strtok(NULL, " ")) != NULL;i++) {
        argv[i] = (char *) malloc(sizeof(char) * (strlen(argu)+1));
        strcpy(argv[i], argu);
    }
    argv[cnt]=NULL;

    if((pid=fork())==0){
        sprintf(BUF, "./%s", argv[0]);
        BUF[strlen(argv[0])+2]=0;
        execv(BUF,argv);
    }
    else if(pid>0){
        wait(NULL);
    }
    else{
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    for(int i=0;i<cnt;i++){
        free(argv[i]);
    }
    free(argv);
    return 1;
}

//라인을 입력 받아서 인자 개수를 리턴하는 함수
int Input_Line(){
    int cnt=0;
    for(int i=0;;i++){
        inputBuf[i]=getchar();
        if(inputBuf[i]==' ')cnt++;
        if(inputBuf[i]=='\n') {
            cnt++;
            inputBuf[i]='\0';
            break;
        }
    }
    return cnt;
}


void Init(){
    int fd;

    getcwd(EXEPATH,PATHMAX);
    strcpy(REPOPATH,EXEPATH);
    strcat(REPOPATH,"/.repo");

    snprintf(COMMITPATH, strlen(REPOPATH)+13, "%s/.commit.log", REPOPATH);
    snprintf(STAGPATH, strlen(REPOPATH)+14, "%s/.staging.log", REPOPATH);

    if (access(REPOPATH, F_OK))// 백업 디렉토리가 존재하지 않을 경우 생성
        mkdir(REPOPATH, 0777);

    if ((fd = open(COMMITPATH, O_RDWR | O_CREAT, 0777)) < 0) {// 백업 로그 파일 열기
        fprintf(stderr, "open error for %s\n", COMMITPATH);
        exit(1);
    }
    close(fd);
    if ((fd = open(STAGPATH, O_RDWR | O_CREAT, 0777)) < 0) {// 백업 로그 파일 열기
        fprintf(stderr, "open error for %s\n", STAGPATH);
        exit(1);
    }
    close(fd);
}