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


server_node * read_server_info(char * line){
    char name[BUF_SIZE];
    char buffer_line[BUF_SIZE];
    int cpu1,cpu2;

    strcpy(buffer_line,line);
    buffer_line[strcspn(buffer_line, "\n")] = 0;
    print(buffer_line);

    if(sscanf(buffer_line, "%[^,],%d,%d", name, &cpu1, &cpu2)==-1){
        print("error in reading sscanf");
        return false; 
    }

    printf("nova linha lida: s %s cpu %d cpu %d\n", name, cpu1, cpu2);


    //assign the config file
    server_node * aux= (server_node *) malloc(sizeof(server_node));
    aux->name=name;
    aux->cpu1=cpu1;
    aux->cpu2=cpu2;
    aux->next=NULL;

    return aux;
}


bool read_config_by_line(char * config_file){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char buffer_line[BUF_SIZE];


    //config
    config = (config_struct *) malloc(sizeof(config_struct));
    config->server_info = (server_node *) malloc(sizeof(server_node));



    fp = fopen(config_file, "r");
    if (fp == NULL){
        print("not opening");
        return false;
    }

    int i;
    int v1,v2,v3;
    if((read = getline(&line, &len, fp)) != -1){
        strcpy(buffer_line,line);
        buffer_line[strcspn(buffer_line, "\n")] = 0;
        v1=atoi(buffer_line);
    }
    else return false;

    if((read = getline(&line, &len, fp)) != -1){
        strcpy(buffer_line,line);
        buffer_line[strcspn(buffer_line, "\n")] = 0;
        v2=atoi(buffer_line);
    }
    else return false;

    if((read = getline(&line, &len, fp)) != -1){
        strcpy(buffer_line,line);
        buffer_line[strcspn(buffer_line, "\n")] = 0;
        v3=atoi(buffer_line);
    }
    else return false;

    //printf("v1: %d v2: %d v3: %d\n", v1,v2,v3);

    if(v3<=2){
        print("number of servers <=2");
        return false;
    }


    print("reading the server info now");
    server_node * temp = (server_node *) malloc(sizeof(server_node));
    server_node * aux = (server_node *) malloc(sizeof(server_node));

    if((read = getline(&line, &len, fp)) == -1){
            print("error reading line");
            return false; 
        }

    aux= read_server_info(line);
    config->server_info= aux;
    i=0;

    printf("%d  temp: %s, aux: %s, config server: %s\n",i, temp->name, aux->name, config->server_info->name);


    for(i=1; i< v3; i++){
        if((read = getline(&line, &len, fp)) == -1){
            print("error reading line");
            return false; 
        }

        aux= read_server_info(line);
        temp=aux;
        temp->next= aux;

        printf("%d  temp: %s, aux: %s, config server: %s\n",i, temp->name, aux->name, config->server_info->name);


    }

    temp->next=NULL;

    print("printing struct server node");
    print(config->server_info->name);
    server_node * check= config->server_info;

    print(config->server_info->name);
    print(check->name);
    /*
    i=0;

    while(check!=NULL && i<5){
        print(check->name);
        check=check->next;
        i++;
    }
    */

    print("done reading success");

    fclose(fp);
    free(temp);
    free(check);

    if (line)
        free(line);


    print("leaving config");
    return true;


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
    */

    return true;
}


void start(char * config_file){

    if(!read_config_by_line(config_file)){
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