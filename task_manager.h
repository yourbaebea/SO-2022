#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include "main.h"


void print_list(){
	printf("- current task list: ");
	task_struct * p=tasklist->first;
	if(p==NULL){
		print("EMPTY\n");
		return;
	}
	
	while(p!=NULL){
		printf("%d ->", p->id);
		p=p->next;
	}
	printf("end\n");
}

void print_list_shm(){
    int i;
    //task_struct *t;
	
	for(i=0;i<config->queue_pos; i++){
        if(shm->tasklist[i].id!=0){
            printf("%d ->", shm->tasklist[i].id);
        }    
    }
	printf("end\n");
}




void print_task(task_struct * t){
    if(t==NULL) return;
    if(debug) printf("Task: %d, final time: %d, init: %d max: %d priority: %d\n", t->id, t->time_start+ t->time_max, t->time_start, t->time_max, t->priority);
}

void insert_shm(task_struct * t){
    int index= find_index_shm();

    if(index==-1) return; //its filled

    task_struct *new = &shm->tasklist[index];

    pthread_mutex_lock(&shm->tasklist_mutex);
    new->id=t->id;
    new->instructions=t->instructions;
    new->time_max=t->time_max;
    new->time_start=t->time_start;
    new->time_acceptance=t->time_acceptance;
    //new->time_needed=t->time_needed;
    //new->time_waiting=t->time_waiting;
    //new->priority=t->priority;
    pthread_mutex_lock(&shm->tasklist_mutex);
    return new;
}

void remove_shm(task_struct * t){
    pthread_mutex_lock(&shm->tasklist_mutex);
    t->id=0;
    t->priority=-1;
    pthread_mutex_unlock(&shm->tasklist_mutex);
}

int find_index_shm(){
    pthread_mutex_lock(&shm->tasklist_mutex);
    int i, index=-1;
    for(i=0; i< config->queue_pos;i++){
        if(shm->tasklist[i].id==0){
            index=i;
            break;
        }
    }
    pthread_mutex_unlock(&shm->tasklist_mutex);
    return index;

}



//transform from pipe to task
bool new_task(char * buffer){
    char temp [BUF_SIZE];
    strcpy(temp,buffer);
    int id,instructions,max_time;
    //ID tarefa:Nº de instruções (em milhares):Tempo máximo para execução
    if(sscanf(temp, "%d:%d:%d", &id, &instructions, &max_time)==3){
        //bool value = create_task(id,instructions,max_time);

        task_struct * task= (task_struct *) malloc(sizeof(task_struct));
    
        task->id = id;
        task->instructions= instructions;
        task->time_max= max_time;
        task->time_start = current_time();

        insert_task_list(task);
        free(task);
        return true;
    }
    return false;

}

//task_struct *new = &shm->tasklist[index];

void insert_task_list(task_struct * task){
	print_task(task);

    pthread_mutex_lock(&shm->stats_mutex);
    shm->stats->tasks_total++;
    pthread_mutex_unlock(&shm->stats_mutex);

    print("insert in tasklist");

    sem_wait(&sem_tasks);
    insert_shm(task);
    sem_post(&sem_tasks);

   
    print_list_shm();
    
    pthread_mutex_lock(&shm->scheduler_mutex);
    pthread_cond_broadcast(&shm->scheduler);
    print("broadcasting cond var scheduler");
    pthread_mutex_unlock(&shm->scheduler_mutex);
}

void remove_task(task_struct * t, bool success){

    if(success){
        print("Task %d is being sent to the cpu", t->id);
    }
    else{
        write_log("Task %d expired and was never executed", t->id);
    }

    sem_wait(&sem_tasks);
    remove_shm(t);
    sem_post(&sem_tasks);
     
}

bool write_unnamed_pipe(task_struct * current){
    int i;
    server_struct * temp;

    for(i=0; i< config->edge_server_number; i++){
        if(i==0){
            temp=shm->server;
        }
        else{
            temp = temp->next;
        }
	    print("task manager bf server mutex");
        pthread_mutex_lock(&temp->server_mutex);
        if(!temp->stopped){
        if( (!temp->cpu1->busy && temp->cpu1->active) || (!temp->cpu2->busy && temp->cpu2->active)){
            write(temp->p[1], current, sizeof(task_struct));
            /*
            struct dirent data;
            close(fileStatusPipe[1]);
            while(read(fileStatusPipe[0],&data,sizeof(data)) > 0)
            {
                printf("%ld: %s\n", data.d_ino, data.d_name);
            }
            close(fileStatusPipe[0]);
            exit(0);
            */
           pthread_mutex_unlock(&temp->server_mutex);
           print("task manager af server mutex");
           return true;
        }
        }
        pthread_mutex_unlock(&temp->server_mutex);
        print("task manager af server mutex");


    }

    print("there was a free cpu but the server probably went to maintenance, we are just ignoring and retrying when another becomes available!");
    return false;
}

void read_task_pipe(){

    
    print("task manager started reading from pipe");

    int fd;
    char buffer[BUF_SIZE];
    char message[BUF_SIZE];

    if ((fd = open(TASK_PIPE, O_RDWR)) < 0){
        print("Error when opening pipe");
        end(EXIT_FAILURE);
    }
        

    while(simulation_status()>=0){  //if waiting to stop we stop reading from pipe!

        if(simulation_status()<0) break;

        for (int n = 0; read(fd, buffer, BUF_SIZE) && n > 0 && buffer[n] != '\0' && buffer[n] != '\n'; n = read(fd, buffer, sizeof(char)));
        if (strcmp(buffer, "EXIT") == 0) {
            terminate();
            return;
        }
        else{
            if (strcmp(buffer, "STATS") == 0) {
            print_stats();
            }
            else{
                if(new_task(buffer)==false){
                    strncpy(message, "WRONG COMMAND =>", BUF_SIZE);
                }
                else{
                    strncpy(message, "VALID COMMAND =>", BUF_SIZE);
                }
                
                strcat(message,buffer);
                write_log(message);
            }  
        }

    }

    //TODO stop reading from pipe
    close(fd);

    print("task manager stopped reading from pipe");

}

void * scheduler(){
    //shm_struct * shm = (shm_struct *) args;

    print("THREAD SCHEDULER CREATED");
    //int score;

    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside scheduler");
        
        pthread_mutex_lock(&shm->scheduler_mutex);
        print("waiting cond var scheduler");
        pthread_cond_wait(&shm->scheduler,&shm->scheduler_mutex);
        print("cond var scheduler wait done!");

        if(simulation_status()<0){
            break;
        }

        sem_wait(&sem_tasks);

        task_struct *t;
        int i;
        
        for(i=0;i<config->queue_pos; i++){
            t= &shm->tasklist[i];
            if(t->id!=0){
                
                //if(t->priority==-1); //first time in scheduler
                t->priority= t->time_start + (current_time() - t->time_max + t->time_start);

                if(t->time_max + t->time_start <= current_time()){
                    write_log("Task %d expired and was never executed", t->id);
                    remove_shm(t);
                }
            }    
        }

        sem_post(&sem_tasks);
	    pthread_mutex_unlock(&shm->scheduler_mutex);
    }

    //free(p);

    pthread_exit(NULL);
    return NULL;

}

void * dispacher(){
    //shm_struct * shm = (shm_struct *) args;
    
    print("THREAD DISPACHER CREATED");



    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside dispacher");

        pthread_mutex_lock(&shm->dispacher_mutex);
        print("waiting cond var dispacher");
        pthread_cond_wait(&shm->dispacher,&shm->dispacher_mutex);
        print("cond var dispacher wait done!");
        //pthread_mutex_unlock(&shm->dispacher_mutex);

        if(simulation_status()<0){
            break;
        }
        int i, count;

        for(count=0; count< shm->count_dispacher; count++){
            sem_wait(&sem_tasks);

            task_struct *t;
            task_struct *save;
            int save_value=INT_MAX;
            
            
            for(i=0;i<config->queue_pos; i++){
                t= &shm->tasklist[i];
                if(t->id!=0 && t->priority>0){
                    
                    if(t->time_max + t->time_start <= current_time()){
                        write_log("Task %d expired and was never executed", t->id);
                        remove_shm(t);
                    }
                    else{
                        if(t->priority< save_value){
                            save_value=t->priority;
                            save=t;
                        }
                    }
                }    
            }

            if(save_value<INT_MAX){
                if(write_unnamed_pipe(save)){
                    remove_shm(save);
                }
                else{
                    count--;
                }
            }

            sem_post(&sem_tasks);

        }

        
	    pthread_mutex_unlock(&shm->scheduler_mutex);
    }

    //free(p);
    
    pthread_exit(NULL);
    return NULL;
}


//TODO
void task_manager() {
    //ignore signal
    write_log("PROCESS TASK_MANAGER CREATED");
    
    //shm->tasklist=NULL;
    //print_task(shm->tasklist);
    
    //remove_task(NULL,false);
    
    //print("config nr: %d",config->edge_server_number);
    
    pthread_create(&thread_time, NULL, time_update, NULL);

    
    pthread_t thread_scheduler, thread_dispacher;
    
    pthread_create(&thread_scheduler, NULL, scheduler, NULL);
    pthread_create(&thread_dispacher, NULL, dispacher, NULL);

    int i;
    //j=0;
    for(i=0; i< config->edge_server_number; i++){
        if(fork()){
        	print("--- server %d of %d", i,config->edge_server_number);
            edge_server(i);
        }
    }
    
  
    read_task_pipe();
	

    wait(NULL);
    //keep the process alive untill the threads end

}

#endif
