/* SO 2021/22 Ana Beatriz Marques 2018274233 */
#ifndef MAIN_C
#define MAIN_C
#include "main.h"
#include "maintenance_manager.h"
#include "task_manager.h"
#include "edge_server.h"
#include "monitor.h"
#include "util.h"


server_node * read_server_info(char * line){
    char name[BUF_SIZE];
    char buffer_line[BUF_SIZE];
    int cpu1,cpu2;

    strcpy(buffer_line,line);
    buffer_line[strcspn(buffer_line, "\n")] = 0;
    //print(buffer_line);
    
    memset(name,0, strlen(name));

    if(sscanf(buffer_line, "%[^,],%d,%d", name, &cpu1, &cpu2)==-1){
        print("error in reading sscanf");
        return false; 
    }

    //assign the config file
    server_node * aux= (server_node *) malloc(sizeof(server_node));
    aux->name=strdup(name);
    aux->cpu1=cpu1;
    aux->cpu2=cpu2;
    aux->next=NULL;

    return aux;
}


void print_node(server_node * temp){
    if(temp==NULL) printf("temp node is null something is wrong");
    else printf("node: %s %d %d\n", temp->name, temp->cpu1, temp->cpu2);
}



bool read_config(char * config_file){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char buffer_line[BUF_SIZE];


    //config
    config = (config_struct *) malloc(sizeof(config_struct));
    
    fp = fopen(config_file, "r");
    if (fp == NULL){
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
        if(v3<3){
      	print("Number of Servers is not permited");
        return false;
    	}
    }
    else return false;
    
    config->queue_pos=v1;
    config->max_wait=v2;
    config->edge_server_number=v3;
    
    print("Number of Servers= %d", v3);
    config->server_info=NULL;

    for(i=0; i< v3; i++){
        if((read = getline(&line, &len, fp)) == -1){
            print("error reading line");
            return false; 
        }

        server_node * temp = read_server_info(line);


        if(config->server_info == NULL){
            config->server_info = (server_node *) malloc(sizeof(server_node));
            config->server_info = temp;
           
        }
        else{
            server_node * p;
            p  = config->server_info;
            while(p->next != NULL){
                p = p->next;
            }
            p->next = temp;
            p = p->next;
        }

    }
      
    fclose(fp);
    if (line) free(line);
       

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
    fclose(fopen(LOG_FILE, "w")); //clear if the previous run ended and log wasnt cleared
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
   
    shmid = shmget(IPC_PRIVATE, sizeof(shm_struct), IPC_CREAT | 0600);
    if (shmid < 0) {
        print("ERROR IN CREATE SHMID");
        end(EXIT_FAILURE);
    }

    shm = (shm_struct*) shmat(shmid, NULL, 0);
    if (shm == (shm_struct*)-1) {
        print("ERROR IN CREATE SHM");
        end(EXIT_FAILURE);
    }
    

    //this should allocate memory for all needed structs without creating a new shared memory
	/*
    shm->teams = (Team *) malloc(config.teams *sizeof(Team));
    for(int i=0; i<config.teams;i++){
     shm->teams[i].list_of_cars = (Car *) malloc(config.max_cars_per_team *sizeof(Car));
     }

*/





    //Create Shared memory for servers
    //TODO
    //Create shm for list of tasks
    //TODO


    if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0700)) == -1) {
        printf ("Error on MQ creation\n");
        end(EXIT_FAILURE);
    }

   
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


        
        print("before time");

        pthread_create(&thread_time, NULL, time_update, NULL);
        
        
        print("after time");
        
        wait(NULL);
        
        print("after waiting");




        //update time and wait for the signals
        //signal(SIGINT, terminate);
        //signal (SIGTSTP,print_stats);

    }

    return 0;
}


#endif
