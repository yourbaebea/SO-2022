/* SO 2021/22 Ana Beatriz Marques 2018274233 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int debug;

int print(char * message){
    if(debug) printf("DEBUG: %s\n", message);
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

    print("MOBILE NODE");
    fd = open("TASK_PIPE", O_WRONLY);

    if (fd < 0) {
        printf("CANNOT OPEN PIPE FOR WRITING\n");
        exit(EXIT_FAILURE);
    }

    for(i=0; i<total_tasks; i++){
        //ID tarefa:Nº de instruções (em milhares):Tempo máximo para execução
        fprintf(buffer, "%d:%d:%d\n", i, instructions, max_time_task);
        write(fd, buffer, strlen(buffer)+1);
        print(buffer);


        usleep(interval);
    }
    
    fclose(fd);
    print("ended the program");
    exit(EXIT_SUCCESS);
}
