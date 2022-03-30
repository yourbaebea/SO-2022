/* SO 2021/22 Ana Beatriz Marques 2018274233 */
#ifndef MAIN_C
#define MAIN_C
#include "main.h"
#include "maintenance_manager.h"
#include "task_manager.h"
#include "edge_server.h"
#include "monitor.h"

void print(char * message){
    if(debug) printf("DEBUG: %s\n", message);
}

void write_log(char * message){
    char buffer[80];
  	char timeformat [80];
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeformat, 80,"[%I:%M:%S]",timeinfo);
    
    pthread_mutex_lock(&shm->log_mutex);

    fprintf(log_file, "%s %s \n", timeformat, message);
    fflush(log_file);
    
    // write in the terminal
    printf("%s %s\n",timeformat,message);

    pthread_mutex_unlock(&shm->log_mutex);


}

void clear_log(){
    fclose(fopen(LOG_FILE, "w"));
}

bool read_config(char * config_file) {
    
    /*
    char dir[BUF_SIZE];

    strcpy(dir, PATH);
    strcat(dir, config_file); 
    print(PATH);
    print(config_file);
    print(dir);
    
    print("READING CONFIG FILE");
    FILE * fi = fopen(dir, "r");
    */



    print("CHECK THIS\n\n\n error \n\n\n READING CONFIG FILE");
    FILE * fi = fopen(config_file, "r");

    if (fi == NULL){
        return false;
    }
    /*
    QUEUE_POS - número de slots na fila interna do Task Manager
    MAX_WAIT - tempo máximo para que o processo Monitor eleve o nível de performance dos
    Edge Servers (em segundos)
    EDGE_SERVER_NUMBER - número de edge servers (>=2)
    Nome do servidor de edge e capacidade de processamento de cada vCPU em MIPS
    Exemplo do ficheiro de configurações:
    50
    2
    3
    SERVER_1,100,200
    SERVER_2,150,200
    SERVER_3,180,200

    */

    print("1");
    int temp,i;

    //config
    config_struct * config_temp;

    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    fscanf(fi, "%d\n%d\n%d\n", config_temp->queue_pos,config_temp->max_wait, temp);

    printf("received: %d %d %d\n", config_temp->queue_pos, config_temp->max_wait, temp);

    if(temp<2) return false;

    server_node * server_temp;
    server_node * save;
    save =config_temp->server_info ;

    print("2");

    char name [BUF_SIZE];

    for(i=0; i<temp; i++){
        read = getline(&line, &len, fi);
        //if(fscanf(fi, "%s,%d,%d\n", name, &server_temp->cpu1, &server_temp->cpu2)!=3) return false;
        fscanf(line, "%s,%d,%d\n", name, &server_temp->cpu1, &server_temp->cpu2);
        printf("received: %s ; %d  ; %d\n", name, server_temp->cpu1, server_temp->cpu2 );
        server_temp->name=name;
        save = server_temp;
        save->next=save;
        server_temp=NULL;
        save=NULL;
    }

    print("3");

    config_temp->edge_server_number=temp;

    if(debug){
        printf("just to check reading config");
        for(i=0; i<temp; i++) printf(" server name %s, cpu1 %d, cpu2 %d",server_temp->name, server_temp->cpu1, server_temp->cpu2 );
    }


    fclose(fi);

    config = config_temp;

    return true;
}


void start(char * config_file){

    if(!read_config(config_file)){
        print("ERROR READING CONFIG FILE, LEAVING");
        end(EXIT_FAILURE);
    }

    print("CONFIG FILE DONE");


    //ppid = getpid();
    ////TODO

    // receive signals
    //TODO

    //open log file
    print("OPENING LOG FILE");
    clear_log(); //clear if the previous run ended and log wasnt cleared
    log_file = fopen(LOG_FILE, "a");
    if(log_file == NULL){
        print("ERROR OPENING LOG FILE");
        end(EXIT_FAILURE);
    }

    // Create pipe
    print("CREATE NAMED PIPE");
    unlink(TASK_PIPE);
    if(mkfifo(TASK_PIPE, O_CREAT|O_EXCL|0600) < 0) {
        print("ERROR CREATING FIFO TASK_PIPE");
        end(EXIT_FAILURE);
    }


    // Create Shared memory
    print("CREATE SHM");
    if((shmid = shmget(IPC_PRIVATE, sizeof(shm_struct), IPC_CREAT|0700)) == -1) {
        print("ERROR IN CREATE SHM");
        end(EXIT_FAILURE);
    }
    shm = shmat(shmid, NULL, 0);

    //Create Shared memory for servers
    //TODO
    //Create shm for list of tasks
    //TODO


    if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0700)) == -1) {
        printf ("Error on MQ creation\n");
        end(1);
    }

   
}


void end(int status){
    //remove all used

    exit(status);
}


int main(int argc, char *argv[]){
    if(argc >=4 || argc <= 1){
        printf("WRONG FORMAT, ./offload_simulator {configfile} [debug]\n");
        return 0;
    }
    if(argc == 3 && strcmp(argv[2], "debug") == 0) debug = true;


    print("SYSTEM MANAGER");

    start(argv[1]);

    print("START DONE");
    write_log("OFFLOAD SIMULATOR STARTING");

    if(fork()) {
        task_manager();
    }
    if(fork()) {
        monitor();
    }
    if(fork()) {
        maintenance_manager();
    }
    else {
        print("SYSTEM MANAGER AFTER FORKS");
        //this continues to be the SYSTEM MANAGER

        //update time and wait for the signals
        //signal(SIGINT, terminate);
        //signal (SIGTSTP,print_stats);
        end(EXIT_SUCCESS);
    }
}

#endif