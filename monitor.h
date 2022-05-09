
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

    while(simulation_status()>=0){ //if ending we dont need this anymore
        //update cpu status shm depending on lvl
        /*
        Processo responsável por controlar o número de vCPUs ativos dentro dos processos Edge Server.
        Para poupar energia cada servidor apenas tem por omissão 1 vCPU ativo.
        Sempre que a fila no processo Task Manager esteja mais de 80% preenchida e o tempo mínimo
        de espera para que uma nova tarefa seja executada for superior a MAX_WAIT segundos (ver
        ficheiro de configurações), o processo Monitor ativará o modo High performance de todos os Edge
        Server. Quando a fila descer a ocupação para 20% volta a ativar o modo de performance Normal.
        A troca de modo de performance será assinalada através de uma flag existente em memória
        partilhada.
        */

        count_tasks=1;
        lower_time=INT_MAX;
        temp= tasklist;
        task_struct * task;
        server=shm->server;
        float aux_tasks;
        int aux_time;


        sem_wait(&sem_tasks);
        while(temp->next!=NULL){
            count_tasks++;
            temp=temp->next;
        }
        sem_wait(&sem_tasks);

        while(server->next!=NULL){
            pthread_mutex_lock(&server->cpu1->task_available_mutex);
            task= server->cpu1->task;
            
            if(task!=NULL){
                aux_time= task->time_needed - ( current_time() - task->time_acceptance);
                //time still left to finish!
                if(aux_time<lower_time) lower_time = aux_time;
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

        if(aux_time > config->max_wait && aux_tasks > 0.8){
            //CHANGE TO HIGH
            //IM HERE
            //DOING THIS
        }




       //stop all things
        pthread_mutex_lock(&server->server_mutex);
        if(server->stopped==false){ 
            server->cpu2->active= true;
        }
        pthread_mutex_unlock(&server->server_mutex);

    }

}

#endif
