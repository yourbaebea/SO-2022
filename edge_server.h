
#ifndef EDGE_SERVER_H
#define EDGE_SERVER_H

#include "main.h"



void * cpu(int *parameters[2]){
    cpu_struct * cpu;
    int id= parameters[0];

    server_struct* server;
    server= shm->server;
    for(int i=1; i<id; i++){
        server = server->next;
    }

    pthread_mutex_lock(&server->cpu_mutex);

    if(parameters[1]==1){
        cpu= server->cpu1;
        cpu->active=true;
        cpu->busy=false;
    }
    else{
        cpu=server->cpu2;
        cpu->active=false; //default starts with normal status, so only one cpu active
        cpu->busy=false;
    }
    pthread_mutex_unlock(&server->cpu_mutex);

    free(parameters);
   
    while(simulation_status()>=0){
        //its fine if inside its -1 bc its supossed to make it still run, the next time is not gonna enter

        pthread_mutex_lock(&cpu->task_available_mutex);
        pthread_cond_wait(&cpu->task_available,&cpu->task_available_mutex);
        pthread_mutex_unlock(&cpu->task_available_mutex);
        //waiting for a new task to be processed;

        pthread_mutex_lock(&server->cpu_mutex);
        cpu->busy=true;
        pthread_mutex_unlock(&server->cpu_mutex);

        //TODO stuff here
        cpu->task->time_waiting= current_time()- cpu->task->time_start;



        sleep(cpu->task->instructions / cpu->mips);


        pthread_mutex_lock(&server->cpu_mutex);
        cpu->busy=false;
        pthread_mutex_unlock(&server->cpu_mutex);


        //add to stats AFTER BEING DONE
        pthread_mutex_lock(&shm->stats_mutex);
        shm->stats->tasks_done++;
        shm->stats->total_time_response+=cpu->task->time_waiting;
        shm->stats->tasks_by_server[id]++;
        pthread_mutex_unlock(&shm->stats_mutex);
        cpu->task=NULL;

    }


    pthread_exit(NULL);
    return NULL;
}

task_struct * copy(task_struct * old){
    task_struct * new= (task_struct*) malloc(sizeof(task_struct *));
    new->id=old->id;
    new->instructions=old->instructions;
    new->time_max= old->time_max;
    new->time_start= old->time_start;
    new->time_waiting= old->time_waiting;
    return new;

}
void read_unnamed_pipe(server_struct * server){
    task_struct temp= (task_struct*) malloc(sizeof(task_struct *));

    while(read(server->p[0],&temp,sizeof(task_struct)) > 0)
    {
        print("UNNAMED PIPE NEW TASK: %d\n", temp.id);
        
        pthread_mutex_lock(&server->cpu_mutex);
        if(!server->cpu1->busy && server->cpu1->active){
            server->cpu1->task= copy(temp);

            //do i need this?
            pthread_mutex_lock(&server->cpu1->task_available_mutex);
            pthread_cond_signal(&server->cpu1->task_available);
            pthread_mutex_unlock(&server->cpu1->task_available_mutex);
            
        }

        pthread_mutex_unlock(&server->cpu_mutex);

    }

    free(temp);
    
}


//TODO
void edge_server(int id) {
    write_log("PROCESS EDGE SERVER CREATED");
    print("edge server %d", id);

    //inform Maintenance Manager by mq????


    pthread_t thread_cpu1, thread_cpu2;
    server_struct* server; //= (server_struct*) malloc(sizeof(server_struct*));
    //its just a pointer we dont need to alloc memory
    server= shm->server;
    for(int i=1; i<id; i++){
        server = server->next;
    }

    print("creating threads of cpus");

    int *parameters[2];
    parameters[0]=id;


    parameters[1]=1;
    pthread_create(&thread_cpu1, NULL, cpu1, parameters);

    parameters[1]=2;
    pthread_create(&thread_cpu2, NULL, cpu2, parameters);



    while(1){
        //update shm values for each cpu
    }



    wait(NULL);
    //keep the process alive untill the threads end

}


#endif
