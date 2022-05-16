/* SO 2021/22 Ana Beatriz Marques 2018274233 */
#ifndef MAIN_C
#define MAIN_C
#include "main.h"
#include "maintenance_manager.h"
#include "task_manager.h"
#include "edge_server.h"
#include "monitor.h"
#include "util.h"

// read individual server config from the config file
server_node * read_config_aux(char * line){
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

//read config file
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

        server_node * temp = read_config_aux(line);


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

//init simulation -> start log, config, shm, mq,...
void start(char * config_file){
	int i;

    if(!read_config(config_file)){
        print("ERROR READING CONFIG FILE, LEAVING");
        end(EXIT_FAILURE);
    }

    print("CONFIG FILE DONE");
    
    
    
    //open log file
    print("OPENING LOG FILE");
    fclose(fopen(LOG_FILE, "w")); //clear if the previous run ended and log wasnt cleared
    log_file = fopen(LOG_FILE, "a");
    if(log_file == NULL){
        print("ERROR OPENING LOG FILE");
        end(EXIT_FAILURE);
    }


    //ppid = getpid();
    ////TODO

    // receive signals
    //TODO

    // Create pipe
    print("CREATE NAMED PIPE");
    unlink(TASK_PIPE);
    if(mkfifo(TASK_PIPE, O_CREAT|O_EXCL|0600) < 0) {
        print("ERROR CREATING FIFO TASK_PIPE");
        end(EXIT_FAILURE);
    }


    // Create Shared memory
    print("CREATE SHM");
    int shmid, server_shmid, task_shmid;
   
    shmid = shmget(IPC_PRIVATE, sizeof(shm_struct), IPC_CREAT | 0600);
    if (shmid < 0) {
        print("ERROR IN CREATE SHMID");
        end(EXIT_FAILURE);
    }

    shm = (shm_struct*) shmat(shmid, NULL, 0);
    if (shm == (shm_struct*)(-1)) {
        print("ERROR IN CREATE SHM");
        end(EXIT_FAILURE);
    }
    
    //shm for servers
    
    
    
    server_shmid = shmget(IPC_PRIVATE, sizeof(server_struct) * config->edge_server_number,  IPC_CREAT | 0600);
    if (server_shmid < 0) {
        print("ERROR IN CREATE SHMID");
        end(EXIT_FAILURE);
    }

    shm->server = (server_struct*) shmat(server_shmid, NULL, 0);
    if (shm->server == (server_struct*) (-1)) {
        print("ERROR IN CREATE SHM");
        end(EXIT_FAILURE);
    }
    

    

    
    
    //shm for tasks!
    
    int size=sizeof(task_struct) * config->queue_pos;
    
    task_shmid = shmget(IPC_PRIVATE, size,  IPC_CREAT | 0600);
    if (shmid < 0) {
        print("ERROR IN CREATE SHMID");
        end(EXIT_FAILURE);
    }
    
    shm->tasklist = (task_struct*) shmat(task_shmid, NULL, 0);
    if (shm->tasklist == (task_struct*)(-1)) {
        print("ERROR IN CREATE SHM");
        end(EXIT_FAILURE);
    }
    
    //pthread_mutex_lock(&shm->tasklist_mutex);
    
    for(i=0; i< config->queue_pos;i++){
    	shm->tasklist[i].id=0;
    	shm->tasklist[i].priority=-1;
    }
    //pthread_mutex_unlock(&shm->tasklist_mutex);
    
    
    if(init_mutex()==false){
    	print("ERROR IN CREATE MUTEXES");
        end(EXIT_FAILURE);
    }
    
    server_struct * p;
    server_node * paux;
    pthread_mutexattr_t attrmutex;
    if(pthread_mutexattr_init(&attrmutex)!=0) end(EXIT_FAILURE);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    
    for(i=0; i<config->edge_server_number; i++){
    	server_struct * temp= (server_struct *) malloc(sizeof(server_struct));
    	
    	temp->server_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    	if( pthread_mutex_init(&temp->server_mutex,&attrmutex) !=0) end(EXIT_FAILURE);
        
    	temp->cpu1= (cpu_struct *) malloc(sizeof(cpu_struct));
    	temp->cpu2= (cpu_struct *) malloc(sizeof(cpu_struct));
    	
    	temp->cpu1->task_available_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    	if( pthread_mutex_init(&temp->cpu1->task_available_mutex,&attrmutex) !=0) end(EXIT_FAILURE);
    	
    	temp->cpu1->task_available = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    	if( pthread_cond_init(&temp->cpu1->task_available,NULL) !=0) end(EXIT_FAILURE);
    	
    	temp->cpu2->task_available_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    	if( pthread_mutex_init(&temp->cpu2->task_available_mutex,&attrmutex) !=0) end(EXIT_FAILURE);
    	
    	temp->cpu2->task_available = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    	if( pthread_cond_init(&temp->cpu2->task_available,NULL) !=0) end(EXIT_FAILURE);
    	
    	temp->cpu1->task=NULL;
    	temp->cpu2->task=NULL;
    	
        temp->id=i;     
        temp->maintenance=0;
        temp->tasks_done=0;
        temp->stopped=false;
        temp->active_cpus=1;

        
        if (pipe(temp->p) == -1) {
            print("ERROR IN CREATE UNNAMED PIPE");
            end(EXIT_FAILURE);
        }
    	
        if(i==0){
            shm->server = temp;
            p  = shm->server;
            paux= config->server_info;        
        }
        else{
            p->next = temp;
            p = p->next;
            paux=paux->next;
        }
        temp->name=paux->name;
        temp->cpu1->mips= paux->cpu1;
        temp->cpu2->mips= paux->cpu2;
        print("server in shm %s", temp->name);        
    }
    
    
    pthread_mutexattr_destroy(&attrmutex);
    
    
    
    
    
    pthread_mutex_lock(&shm->status_mutex);  
    shm->status=0;
    shm->server_status=0;
    pthread_mutex_unlock(&shm->status_mutex);
    shm->count_init=0;
    shm->count_dispatcher=0;
    
    //print("AFTER MUTEX");
    //tasklist
    //shm->tasklist= (task_struct *) malloc(sizeof(task_struct));
    
    
    //STATS
    shm->stats= (stats_struct *) malloc(sizeof(stats_struct));
    pthread_mutex_lock(&shm->stats_mutex);
    shm->stats->tasks_total=0;
    shm->stats->tasks_done=0;
    shm->stats->total_time_response=0;
    shm->stats->tasks_refused=0;
    shm->stats->tasks_by_server= (int*) malloc(sizeof(int) * config->edge_server_number);
    shm->stats->op_by_server=(int*) malloc(sizeof(int) * config->edge_server_number);
    
    for(i=0; i<config->edge_server_number; i++){
    	shm->stats->tasks_by_server[i]=0;
    	shm->stats->op_by_server[i]=0;
    }
    
    pthread_mutex_unlock(&shm->stats_mutex);
    
    //print("AFTER STATS");


    //sem tasks
    sem_init(&sem_tasks, 0, 1);
    
    //print("AFTER SEM");

    if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0700)) == -1) {
        printf ("Error on MQ creation\n");
        end(EXIT_FAILURE);
    }
    
   
}


bool init_mutex(){
    pthread_mutexattr_t attrmutex;
    pthread_condattr_t cattr;
    if(pthread_mutexattr_init(&attrmutex)!=0) return false;
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    
    if(pthread_condattr_init(&cattr)!=0) return false;
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    
    shm->dispatcher_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->scheduler_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->status_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->time_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->log_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->stats_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    shm->tasklist_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    
    
    shm->dispatcher = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    shm->scheduler = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    
    if( pthread_cond_init(&shm->dispatcher,&cattr) !=0) return false;
    if( pthread_cond_init(&shm->scheduler,&cattr)!=0) return false;
    if( pthread_mutex_init(&shm->status_mutex,&attrmutex)!=0) return false;
    if( pthread_mutex_init(&shm->time_mutex,&attrmutex)!=0) return false;
    if( pthread_mutex_init(&shm->log_mutex,&attrmutex)!=0) return false;
    if( pthread_mutex_init(&shm->stats_mutex,&attrmutex)!=0) return false;
    if( pthread_mutex_init(&shm->tasklist_mutex,&attrmutex)!=0) return false;

    if( pthread_mutex_init(&shm->dispatcher_mutex,&attrmutex)!=0) return false;
    if( pthread_mutex_init(&shm->scheduler_mutex,&attrmutex)!=0) return false;
    
    pthread_condattr_destroy(&cattr);
    pthread_mutexattr_destroy(&attrmutex);
    
    return true;   

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
    
    signal(SIGINT, terminate);
    	signal (SIGTSTP,print_stats);
    	sigemptyset(&block_sigint);
    	sigaddset(&block_sigint, SIGINT);

    if(fork()) {
        task_manager();
    }
    else{
	    if(fork()) {
		monitor();
	    }
	    else{
		    if(fork()) {
			maintenance_manager();
		    }
		    else {
		       
			//print("SYSTEM MANAGER AFTER FORKS");
			//this continues to be the SYSTEM MANAGER


			while(simulation_status()>=-2){
			//not sure what to do here?
			sleep(1);
			}
			
			//idk what the main does after this?

     			print("main: exit");
		    }
	    }
    }
    
    wait(NULL);
    

    return 0;
}

#endif
