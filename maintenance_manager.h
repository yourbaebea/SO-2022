
#ifndef MAINTENANCE_MANAGER_H
#define MAINTENANCE_MANAGER_H
#include "main.h"

int generate(int min, int max){
    srand(time(NULL));
    return (rand() % max)+ min;
}


void * read_msg_queue(void * args){
    int value=*((int *) args);
    msg_struct reply;
    msg_struct msg;

    while(simulation_status()>=0){
    
        if(msgrcv(mqid,&reply,sizeof(msg_struct)- sizeof(long),value, 0))
        {
            print("error while reading msg idk do we end?");
        }
        else{
            //response to server
            msg = (msg_struct) {(long) reply.mtype, 0};
            msgsnd(mqid, &msg, sizeof(msg_struct), 0);
        }
    }
    return NULL;

}


//TODO
void maintenance_manager() {
    //ignore signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    write_log("PROCESS MAINTENANCE MANAGER CREATED");
    pthread_t thread_maintenance;
    pthread_create(&thread_maintenance, NULL, read_msg_queue, (void *) INT_MAX);
    
    /*
    int sim=0;
    while(1){
    	sim =simulation_status();
    	if(sim<-2) break;
    	print("sfvnpiwsanev");
    }
    
    print("%d maintenance");
    */

  

   server_struct * temp;
   int count,i, maintenance, maintenance_time;
   bool valid;
   msg_struct msg;
   
   //pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
   
   
    while(simulation_status()>=0){
    	print("inside maintenance");
    	//if its ending dont do maintenance
        count=0;
        valid=true;
        maintenance=generate(0, config->edge_server_number-1);
	//print("maintenance searching");
        for(i=0; i< config->edge_server_number; i++){
            if(i==0){
                temp=shm->server;
            }
            else{
                temp = temp->next;
            }

            pthread_mutex_lock(&temp->server_mutex);
            if(temp->stopped==true){
            	count++;
            	if(i==maintenance) valid=false;
            }
            pthread_mutex_unlock(&temp->server_mutex);
        }


        //if all servers are in maintenance
        if(count >= config->edge_server_number-1) valid=false;
        
                   
        if(valid==true){
            print("maintenance is valid we are sending a msg");
            maintenance_time=generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
            msg = (msg_struct) {(long) maintenance, maintenance_time};
            msgsnd(mqid, &msg, sizeof(msg_struct), 0);
        }
        else{
            print("maintenance tried server %d, couldnt", maintenance);
        }
        
        maintenance_time= generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
 	print("SLEEP MAINTENANCE %d", maintenance_time);
        sleep(maintenance_time); //interval between maintenance
        print("SLEEP MAINTENANCE");

    }
    
    print("OUTSIDE MAINTENANCE");
    
    sleep(2);
    if(pthread_cancel(thread_maintenance)){
    	pthread_join(thread_maintenance,NULL);
    }
    else{
    	printf("error exiting the thread maintenance");
    }
    
    wait(NULL);
    
    print("leaving maintenance");
    return;

}

#endif
