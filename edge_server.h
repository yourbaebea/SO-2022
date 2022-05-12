
#ifndef EDGE_SERVER_H
#define EDGE_SERVER_H

#include "main.h"


void * cpu(int parameters[2]){
    cpu_struct * cpu;
    int id= parameters[0];

    server_struct* server;
    server= shm->server;
    for(int i=1; i<id; i++){
        server = server->next;
    }

	print("%d inside cpu[%d], before server_mutex", id, parameters[1]);
    //pthread_mutex_lock(&server->server_mutex);
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
    //pthread_mutex_unlock(&server->server_mutex);
    print("%d inside cpu[%d], after server_mutex", id, parameters[1]);

    
    print("inside cpu after params");
    
    pthread_mutex_lock(&shm->simulationstarted_mutex);
    	shm->count_init++;
    	print("%d inside cpu[%d], count init current=%d",  id, parameters[1], shm->count_init);	
    pthread_mutex_unlock(&shm->simulationstarted_mutex);
    
    pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
    print("%d inside cpu[%d], after simulation started",  id, parameters[1]);
   
   
    pthread_mutex_lock(&server->server_mutex); 
    if(cpu->active==true){
    	pthread_mutex_lock(&shm->dispacher_mutex);
	shm->count_dispacher++; //add to the count
	print("count dispacher is currently %d", shm->count_dispacher);
	pthread_mutex_unlock(&shm->dispacher_mutex);
	pthread_cond_signal(&shm->dispacher);
	print("dispacher cond broadcasted");
    }
    pthread_mutex_unlock(&server->server_mutex);	
    
   free(parameters);
      
    while(simulation_status()>=0){
    	
    	print("inside cpu, inside simulation");

        //its fine if inside its -1 bc its supossed to make it still run, the next time is not gonna enter

        pthread_cond_wait(&cpu->task_available,&cpu->task_available_mutex);
        pthread_mutex_lock(&cpu->task_available_mutex);
        //waiting for a new task to be processed;

        pthread_mutex_lock(&server->server_mutex);
        cpu->busy=true;
        pthread_mutex_unlock(&server->server_mutex);

        //TODO stuff here
        cpu->task->time_acceptance= current_time();
        cpu->task->time_needed= cpu->task->instructions / cpu->mips;
        cpu->task->time_waiting= current_time()- cpu->task->time_start;

        pthread_mutex_unlock(&cpu->task_available_mutex);

        sleep(cpu->task->instructions / cpu->mips);
        
        
        //add to stats AFTER BEING DONE
        pthread_mutex_lock(&shm->stats_mutex);
        shm->stats->tasks_done++;
        shm->stats->total_time_response+=cpu->task->time_waiting;
        shm->stats->tasks_by_server[id]++;
        pthread_mutex_unlock(&shm->stats_mutex);
        cpu->task=NULL;


        pthread_mutex_lock(&server->server_mutex);
        cpu->busy=false;
        
        if(server->stopped==false){
		pthread_mutex_lock(&shm->dispacher_mutex);
		shm->count_dispacher++; //add to the count
		pthread_mutex_unlock(&shm->dispacher_mutex);
		pthread_cond_broadcast(&shm->dispacher);
        }
        else{
        	cpu->active=false;
        }
        
        pthread_mutex_unlock(&server->server_mutex);

    }


    pthread_exit(NULL);
    return NULL;
}

task_struct * copy(task_struct * old){
    task_struct * new= (task_struct*) malloc(sizeof(task_struct));
    new->id=old->id;
    new->instructions=old->instructions;
    new->time_max= old->time_max;
    new->time_start= old->time_start;
    new->time_waiting= old->time_waiting;
    return new;

}

void * read_unnamed_pipe(server_struct * server){
    task_struct * temp= (task_struct*) malloc(sizeof(task_struct));

    while(read(server->p[0],&temp,sizeof(task_struct)) > 0)
    {
        print("UNNAMED PIPE NEW TASK: %d\n", temp->id);
        
        pthread_mutex_lock(&server->server_mutex);
        if(!server->cpu1->busy && server->cpu1->active){
            server->cpu1->task= copy(temp);

            //do i need this?
            //pthread_mutex_lock(&server->cpu1->task_available_mutex);
            pthread_cond_signal(&server->cpu1->task_available);
            //pthread_mutex_unlock(&server->cpu1->task_available_mutex);
            
        }

        pthread_mutex_unlock(&server->server_mutex);

    }
    pthread_exit(NULL);
    return NULL;

    //free(temp);
    
}

//TODO
void edge_server(int id) {
    //ignore signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    write_log("PROCESS EDGE SERVER CREATED");
    print("edge server %d", id);

    //inform Maintenance Manager by mq????


    pthread_t thread_cpu1, thread_cpu2, thread_read_pipe;
    server_struct* server; //= (server_struct*) malloc(sizeof(server_struct*));
    //its just a pointer we dont need to alloc memory
    server= shm->server;
    for(int i=1; i<id; i++){
        server = server->next;
    }

    print("creating threads of cpus");

    int parameters[2];
    parameters[0]=id;
    parameters[1]=1;
    
    pthread_create(&thread_cpu1, NULL, cpu,(void *) parameters);

    int parameters2[2];
    parameters2[0]=id;
    parameters2[1]=2;
    pthread_create(&thread_cpu2, NULL, cpu,(void *) parameters2);

    print("creating thread to read unnamed pipe");
    pthread_create(&thread_read_pipe, NULL, read_unnamed_pipe,(void *) server);


    msg_struct msg;
    msg_struct reply;
    msg_struct confirmation;
    
    pthread_mutex_lock(&shm->simulationstarted_mutex);
    	shm->count_init++;
    	print("count init current=%d", shm->count_init);	
    pthread_mutex_unlock(&shm->simulationstarted_mutex);
    
    pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
    print("%d after simulation started",  id);

    while(simulation_status()>=0){
    
        msgrcv(mqid, &msg, sizeof(msg_struct), id, 0);
        print("edge server, msgrcv");

        //stop all things
        pthread_mutex_lock(&server->server_mutex);
        server->stopped=true;
        server->maintenance++;
        server->cpu1->active=false;
        server->cpu2->active=false;
        pthread_mutex_unlock(&server->server_mutex);

        if(server->cpu1->busy==true || server->cpu2->busy==true){
            pthread_mutex_lock(&shm->dispacher_mutex);
            pthread_cond_wait(&shm->dispacher,&shm->dispacher_mutex);
            pthread_mutex_unlock(&shm->dispacher_mutex);
            print("edge server: maintenance used dispacher cond var once");

            if(server->cpu1->busy==true || server->cpu2->busy==true){
                pthread_mutex_lock(&shm->dispacher_mutex);
                pthread_cond_wait(&shm->dispacher,&shm->dispacher_mutex);
                pthread_mutex_unlock(&shm->dispacher_mutex);
                print("edge server: maintenance used dispacher cond var twice");
            }
            //we need to do this in case both cpus are being used
        }

    

        sleep(msg.maintenance_time);

        reply = (msg_struct) {(long) INT_MAX, msg.mtype};
        msgsnd(mqid, &reply, sizeof(msg_struct), 0);

        msgrcv(mqid, &confirmation, sizeof(msg_struct), id, 0);
        
        pthread_mutex_lock(&shm->status_mutex);
    	int s=shm->server_status;
    	pthread_mutex_unlock(&shm->status_mutex);

        //restart all things
        pthread_mutex_lock(&server->server_mutex);
        server->stopped=false;
        server->active_cpus=s;
        server->cpu1->active=true;
        if(server->active_cpus==2){
            server->cpu2->active= true;
        }
        pthread_mutex_unlock(&server->server_mutex);
        
        
        //send cond var
        
        pthread_mutex_lock(&shm->dispacher_mutex);
	shm->count_dispacher++; //add to the count
        if(s==2){
            shm->count_dispacher++; //add to the count
        }
        pthread_mutex_unlock(&shm->dispacher_mutex);
        pthread_cond_signal(&shm->dispacher);

    }

    //free(msg);
    //free(reply);
    //free(confirmation);


    wait(NULL);
    //keep the process alive untill the threads end

}


#endif
