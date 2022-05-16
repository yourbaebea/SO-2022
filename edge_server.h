
#ifndef EDGE_SERVER_H
#define EDGE_SERVER_H

#include "main.h"


void * cpu(void * args){
    int * parameters=(int *) args;
    cpu_struct * cpu;
    
    int id= parameters[0];

    server_struct* server;
    server= shm->server;
    for(int i=0; i<id; i++){
        server = server->next;
    }
    
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
    
    pthread_mutex_lock(&shm->simulationstarted_mutex);
    	shm->count_init++;
    	//print("%d inside cpu[%d], count init current=%d",  id, parameters[1], shm->count_init);	
    pthread_mutex_unlock(&shm->simulationstarted_mutex);
    
    //pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
    //print("%d inside cpu[%d], after simulation started",  id, parameters[1]);
   
   while(simulation_status()==0);
   //print("after simulation started");
   
   print("SERVER %s, CPU %d", server->name, parameters[1]);

    pthread_mutex_lock(&server->server_mutex); 
    bool check=cpu->active;
    pthread_mutex_unlock(&server->server_mutex); 

    if(check==true){
    	pthread_mutex_lock(&shm->dispatcher_mutex);
        shm->count_dispatcher++; //add to the count
        print("count dispatcher is currently %d", shm->count_dispatcher);
        pthread_cond_broadcast(&shm->dispatcher);
        print("dispatcher cond broadcasted");
        pthread_mutex_unlock(&shm->dispatcher_mutex);
    }
   //free(parameters);
      
    while(simulation_status()>=0){
    	
    	//print("inside cpu, inside simulation");

        //its fine if inside its -1 bc its supossed to make it still run, the next time is not gonna enter
	pthread_mutex_lock(&cpu->task_available_mutex);
        pthread_cond_wait(&cpu->task_available,&cpu->task_available_mutex);
        pthread_mutex_unlock(&cpu->task_available_mutex);
        
        print("task available wait done!");
        //pthread_mutex_lock(&cpu->task_available_mutex);
        //waiting for a new task to be processed;
        
        if(cpu->task==NULL){
        	print("task available but task is null, something went wrong");
        	
        }
        else{
		print("SERVER %s, CPU %d: NEW TASK %d", server->name, parameters[1],cpu->task->id );
		pthread_mutex_lock(&server->server_mutex);
		cpu->busy=true;
		pthread_mutex_unlock(&server->server_mutex);

		//TODO stuff here
		cpu->task->time_acceptance= current_time();
		cpu->task->time_needed= cpu->task->instructions / cpu->mips;
		cpu->task->time_waiting= current_time()- cpu->task->time_start;

		pthread_mutex_unlock(&cpu->task_available_mutex);


		print("cpu doing task for %.2f", cpu->task->instructions /(float) cpu->mips);
		sleep(cpu->task->instructions / (float) cpu->mips);
		
		
		//add to stats AFTER BEING DONE
		pthread_mutex_lock(&shm->stats_mutex);
		shm->stats->tasks_done++;
		shm->stats->total_time_response+=cpu->task->time_waiting;
		shm->stats->tasks_by_server[id]++;
		pthread_mutex_unlock(&shm->stats_mutex);
		
		
		write_log("SERVER %s, CPU %d: TASK %d DONE, CPU IS FREE", server->name, parameters[1], cpu->task->id);
		cpu->task=NULL;

		pthread_mutex_lock(&server->server_mutex);
		cpu->busy=false;
		
		if(server->stopped==false){
			pthread_mutex_lock(&shm->dispatcher_mutex);
			shm->count_dispatcher++; //add to the count
			pthread_cond_broadcast(&shm->dispatcher);
			pthread_mutex_unlock(&shm->dispatcher_mutex);
		}
		else{
			cpu->active=false;
		}
		
		pthread_mutex_unlock(&server->server_mutex);
        }
        
        
        
        

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

void * read_unnamed_pipe(void * args){
	server_struct * server = (server_struct *) args;
    task_struct * temp= (task_struct*) malloc(sizeof(task_struct));
    
    while(read(server->p[0],temp,sizeof(task_struct)) > 0)
    {
    	print("pipe reading");
    	
        print("UNNAMED PIPE NEW TASK: %d\n", temp->id);
        
        pthread_mutex_lock(&server->server_mutex);
        if(!server->cpu1->busy && server->cpu1->active){
            server->cpu1->task= copy(temp);

            //do i need this?
            pthread_mutex_lock(&server->cpu1->task_available_mutex);
            print("pipe inside cond signal task available");
            
            pthread_cond_broadcast(&server->cpu1->task_available);
            pthread_mutex_unlock(&server->cpu1->task_available_mutex);
            
        }

        pthread_mutex_unlock(&server->server_mutex);

    }
    pthread_exit(NULL);
    return NULL;

    //free(temp);
    
}

//TODO
void edge_server(int id) {
    if(simulation_status()<0) return;
    //ignore signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    write_log("PROCESS EDGE SERVER CREATED");
    //print("edge server %d", id);

    //inform Maintenance Manager by mq????


    pthread_t thread_cpu1, thread_cpu2, thread_read_pipe;
    server_struct* server; //= (server_struct*) malloc(sizeof(server_struct*));
    //its just a pointer we dont need to alloc memory
    server= shm->server;
    for(int i=0; i<id; i++){
        server = server->next;
    }
    
    print("HERE in edge %s", server->name);

    //print("creating threads of cpus");

    int parameters[2];
    parameters[0]=id;
    parameters[1]=1;
    
    pthread_create(&thread_cpu1, NULL, cpu,(void *) parameters);

    int parameters2[2];
    parameters2[0]=id;
    parameters2[1]=2;
    pthread_create(&thread_cpu2, NULL, cpu,(void *) parameters2);

    //print("creating thread to read unnamed pipe");
    pthread_create(&thread_read_pipe, NULL, read_unnamed_pipe,(void *) server);


    msg_struct msg;
    msg_struct reply;
    msg_struct confirmation;
    
    pthread_mutex_lock(&shm->simulationstarted_mutex);
    	shm->count_init++;
    	//print("server: count init %d", shm->count_init);	
    pthread_mutex_unlock(&shm->simulationstarted_mutex);
    
    //pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
    
    
    
    while(simulation_status()==0);
    write_log("SERVER %s READY", server->name);
    
    print("server %d OR %d: reading msgs",  server->id, id);
    while(simulation_status()>=0){
    
        if(msgrcv(mqid, &msg, sizeof(msg_struct)-sizeof(long), id, 0)== -1){
        	/*
        	if(errno == EINTR){
        		print("signal was detected in msgrcv");
        		break;
        	}
        	else{
        	print("some other error msgrcv");
        	}*/
        }
        else{
        
        print("edge server, msgrcv");

        //stop all things
        pthread_mutex_lock(&server->server_mutex);
        server->stopped=true;
        server->maintenance++;
        server->cpu1->active=false;
        server->cpu2->active=false;
        pthread_mutex_unlock(&server->server_mutex);

        if(server->cpu1->busy==true || server->cpu2->busy==true){
            pthread_mutex_lock(&shm->dispatcher_mutex);
            pthread_cond_wait(&shm->dispatcher,&shm->dispatcher_mutex);
            pthread_mutex_unlock(&shm->dispatcher_mutex);
            print("edge server: maintenance used dispatcher cond var once");

            if(server->cpu1->busy==true || server->cpu2->busy==true){
                pthread_mutex_lock(&shm->dispatcher_mutex);
                pthread_cond_wait(&shm->dispatcher,&shm->dispatcher_mutex);
                pthread_mutex_unlock(&shm->dispatcher_mutex);
                print("edge server: maintenance used dispatcher cond var twice");
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
        
        pthread_mutex_lock(&shm->dispatcher_mutex);
	shm->count_dispatcher++; //add to the count
        if(s==2){
            shm->count_dispatcher++; //add to the count
        }
        pthread_cond_broadcast(&shm->dispatcher);
        pthread_mutex_unlock(&shm->dispatcher_mutex);
        }
    }
    
    bool okay=true;
    while(okay){
	    pthread_mutex_lock(&server->server_mutex);
	    if(server->cpu1->busy || server->cpu1->busy) okay=false;
	    pthread_mutex_unlock(&server->server_mutex);
	    if(okay) sleep(1);
   }
    
    pthread_cancel(thread_cpu1);
    pthread_cancel(thread_cpu2);
    pthread_cancel(thread_read_pipe);
    
    
    pthread_join(thread_cpu1,NULL);
    pthread_join(thread_cpu2,NULL);
    pthread_join(thread_read_pipe,NULL);

    print("server: exit");
    //keep the process alive untill the threads end

}


#endif
