
#ifndef MONITOR_H
#define MONITOR_H
#include "main.h"

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

}

#endif
