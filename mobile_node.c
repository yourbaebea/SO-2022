/* SO 2021/22 Ana Beatriz Marques 2018274233 */
#ifndef MOBILE_NODE_C
#define MOBILE_NODE_C


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <regex.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>

#define BUF_SIZE 2048

int debug;

void print(char * message,...){
    if(debug){
    va_list args;
    va_start(args,message);
    printf("DEBUG: %s\n", message);
    }
}

int main(int argc, char *argv[]){
    if(argc <= 4){
        print("not enough args");
        printf("WRONG FORMAT, ./mobile_node {total_tasks} {interval} {instructions} {max_time_task} [debug]\n");
        return 0;
    }
    if((argc == 6) && strcmp(argv[5], "debug") == 0) debug = true;
    
    int total_tasks, interval, instructions, max_time_task;

    total_tasks = atoi(argv[1]) ;
    interval= atoi(argv[2]) ;
    instructions= atoi(argv[3]) ;
    max_time_task= atoi(argv[4]) ;

    if(total_tasks ==0 || interval==0 || instructions==0 || max_time_task==0){
        print("values given may be 0 or string, this isnt allowed");
        printf("WRONG FORMAT, ./mobile_node {total_tasks} {interval} {instructions} {max_time_task} [debug]\n");
        return 0;
    }

    int fd, i;
    char buffer[1024];

    print("STARTED MOBILE NODE");
    fd = open("TASK_PIPE", O_WRONLY);

    if (fd < 0) {
        printf("CANNOT OPEN PIPE FOR WRITING\n");
        exit(EXIT_FAILURE);
    }
    
    int pid= getpid();
    
    //printf("mobile node with pid: %d", getpid());
    
    

    for(i=0; i<total_tasks; i++){
        //ID tarefa:Nº de instruções (em milhares):Tempo máximo para execução

        sprintf(buffer, "%d%d:%d:%d\n", pid, i, instructions, max_time_task);
        write(fd, buffer, strlen(buffer)+1);
        print(buffer);


        usleep(interval);
    }
    
    close(fd);
    print("ended the program");
    //exit(EXIT_SUCCESS);
    return 0;
}

#endif
