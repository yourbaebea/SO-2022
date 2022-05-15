
#ifndef MONITOR_H
#define MONITOR_H
#include "main.h"



bool check_change(int current){
    int count_tasks;
    int lower_time;
    task_struct * temp;
    server_struct * server;
    int change;
    bool cpu1, cpu2;
    bool okay;

        count_tasks=0;
        lower_time=INT_MAX;
        temp= tasklist;
        task_struct * task;
        
        float aux_tasks;
        int aux_time;

        sem_wait(&sem_tasks);
        while(temp!=NULL){
            count_tasks++;
            temp=temp->next;
        }
        sem_wait(&sem_tasks);
	aux_tasks= count_tasks/(float) config->queue_pos;
	if(current==2 && aux_tasks < 0.2) return true;
	if(current==2 || aux_tasks < 0.8) return false; // makes no diff either way above 0.2 and below 0.8 does not change anything!
	
	
	okay=false;
	server=shm->server;
	while(server!=NULL){
	    cpu1=false;
	    cpu2=false;
	    pthread_mutex_lock(&server->server_mutex);
	    if(server->cpu1->active==true){
	    	if(server->cpu1->busy==true) cpu1=true;
	    	else okay=true; //there is an active and empty cpu 
	    }
	    if(server->cpu2->active==true){
	    	if(server->cpu2->busy==true) cpu2=true;
	    	else okay=true; //there is an active and empty cpu 
	    }
	    pthread_mutex_unlock(&server->server_mutex);
	    
	    if(okay==true) return false;
	    //ignores the rest if there is an empty and active cpu
	    
	    if(cpu1==true){
	    	pthread_mutex_lock(&server->cpu1->task_available_mutex);
		    task= server->cpu1->task;
		    
		    if(task!=NULL){
			aux_time= task->time_needed - ( current_time() - task->time_acceptance);
			//time still left to finish!
			if(aux_time<lower_time) lower_time = aux_time;
		    }
		    else{
			aux_time=0;
			//this is never supposed to happen
		    }
	    	pthread_mutex_unlock(&server->cpu1->task_available_mutex);
	    }
	    
	    if(cpu2==true){
		    pthread_mutex_lock(&server->cpu2->task_available_mutex);
		    task= server->cpu2->task;
		    if(task!=NULL){
			aux_time= task->time_needed - ( current_time() - task->time_acceptance);
			//time still left to finish!
			if(aux_time<lower_time) lower_time = aux_time;
		    }
		    else{
			aux_time=0;
			//this is never supposed to happen
		    }
		    pthread_mutex_unlock(&server->cpu2->task_available_mutex);    
	    }
	
	    server=server->next;
	}
	
	if((aux_time > config->max_wait))  return true;
	
	return false;
}



//TODO
void monitor() {
    //ignore signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    write_log("PROCESS MONITOR CREATED");

    int count_tasks;
    int lower_time;
    task_struct * temp;
    server_struct * server;
    int change;
    
    //pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
    
    while(simulation_status()==0);
    print("after simulation started");

    while(simulation_status()>=0){
    	print("monitor");

        count_tasks=1;
        lower_time=INT_MAX;
        temp= tasklist;
        task_struct * task;
        
        float aux_tasks;
        int aux_time;


        /* THIS IS THE //TODO
        pthread_mutex_lock(&shm->status_mutex);
		print("MONITOR %d", shm->status);
		sim=shm->server_status;
    	pthread_mutex_unlock(&shm->status_mutex);
    	
    	if(check_change(sim)==true){
    		pthread_mutex_lock(&shm->status_mutex);
		if(shm->server_status==1) shm->server_status=2;
		if(shm->server_status==2) shm->server_status=1;
		pthread_mutex_unlock(&shm->status_mutex);
		
		
	    server=shm->server;
	    while(server->next!=NULL){
	        pthread_mutex_lock(&server->server_mutex);
	        if(server->stopped==false){ 
		   server->cpu2->active=!server->cpu2->active;
		}
	        pthread_mutex_unlock(&server->server_mutex);
	        server=server->next;
	    }	
    	}


        */

	/*
        sem_wait(&sem_tasks);
        while(temp->next!=NULL){
            count_tasks++;
            temp=temp->next;
        }
        sem_wait(&sem_tasks);

        server=shm->server;
        while(server!=NULL){
            pthread_mutex_lock(&server->cpu1->task_available_mutex);
            task= server->cpu1->task;
            
            if(task!=NULL){
                aux_time= task->time_needed - ( current_time() - task->time_acceptance);
                //time still left to finish!
                if(aux_time<lower_time) lower_time = aux_time;
            }
            else{
                aux_time=0;
                print("THERE IS AN AVAILABLE CPU, WE ARE IGNORING THE MONITOR");
                break;
            }
            pthread_mutex_unlock(&server->cpu1->task_available_mutex);

            pthread_mutex_lock(&server->cpu2->task_available_mutex);
            task= server->cpu2->task;
            if(task!=NULL){
                aux_time= task->time_needed - ( current_time() - task->time_acceptance);
                //time still left to finish!
                if(aux_time<lower_time) lower_time = aux_time;
            }
            pthread_mutex_unlock(&server->cpu2->task_available_mutex);
            server=server->next;
        }

        aux_tasks= count_tasks/(float) config->queue_pos;


        change=0;
        pthread_mutex_lock(&shm->status_mutex);
        if((shm->server_status==1) && (aux_time > config->max_wait) && (aux_tasks > 0.8)) change=1;
        if((shm->server_status==2) && (aux_tasks < 0.2)) change=-1;
        shm->server_status+=change;
        pthread_mutex_unlock(&shm->status_mutex);

        if(change!=0){
            print("MONITOR CHANGING STATUS IN SERVERS %d", change);
            server=shm->server;
            while(server->next!=NULL){
            	print("monitor bf server mutex");
                pthread_mutex_lock(&server->server_mutex);
                if(change==1) server->cpu2->active=true;
                if(change==-1) server->cpu2->active=false;
                pthread_mutex_unlock(&server->server_mutex);
                print("monitor af server mutex");
                server=server->next;
            }

        }
        */
        


      /* //stop all things
       print("monitor bf server mutex");
        pthread_mutex_lock(&server->server_mutex);
        if(server->stopped==false){ 
            server->cpu2->active= true;
        }
        pthread_mutex_unlock(&server->server_mutex);
        print("monitor af server mutex");
        */
        
        sleep(1);

    }

    print("monitor ended");
    sleep(1);

}

#endif
