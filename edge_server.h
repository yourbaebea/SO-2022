
#ifndef EDGE_SERVER_H
#define EDGE_SERVER_H

#include "main.h"


void * cpu(void* cpu_shm){
    cpu_struct * cpu;

    cpu = (cpu_struct *) cpu_shm;

    //do stuff with cpu;
    while(1){
        //if(cpu->active!=true) break;




        //usleep(time*1000);

    }



    pthread_exit(NULL);
    return NULL;
}


//TODO
void edge_server(int id) {
    write_log("EDGE SERVER");

    //inform Maintenance Manager by mq????


    pthread_t thread_cpu1, thread_cpu2;
    server_struct* server= (server_struct*) malloc(sizeof(server_struct*));
    server= shm->server;
    for(int i=1; i<id; i++){
        server = server->next;
    }

    print("creating threads of cpus");

    pthread_create(&thread_cpu1, NULL, cpu, (void *) &server->cpu1);
    pthread_create(&thread_cpu2, NULL, cpu, (void *) &server->cpu2);



    while(1){
        //update shm values for each cpu
    }



    wait(NULL);
    //keep the process alive untill the threads end

}


#endif