
#ifndef MAINTENANCE_MANAGER_H
#define MAINTENANCE_MANAGER_H
#include "main.h"

int generate(int min, int max){
    srand(time(NULL));
    return ( (rand() % (max+1-min))+ min);
}


void * read_msg_queue(int value/*void * args*/){
	//int value=*((int *) args);
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

  

   server_struct * temp;
   int count,i, maintenance, maintenance_time;
   bool valid;
   msg_struct msg;
   
   //pthread_cond_wait(&shm->simulationstarted,&shm->simulationstarted_mutex);
   while(simulation_status()==0);
   print("after simulation started");
   //print("after simulation started");
   
   
    while(simulation_status()>=0){
    print("maintenance searching");
    //if its ending dont do maintenance
        count=0;
        valid=true;
        maintenance=generate(0, config->edge_server_number-1);
        print("maintenance is %d, should be 0,1 or 2");

        temp=shm->server;
        for(i=0; i< config->edge_server_number; i++){
            
            pthread_mutex_lock(&temp->server_mutex);
            if(temp->stopped==true){
                count++;
                if(i==maintenance) valid=false;
                //when we are trying to do maintenance on an already on maintenance server
            }
            pthread_mutex_unlock(&temp->server_mutex);

            temp = temp->next;

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
        sleep(maintenance_time); //interval between maintenance

    }
    
    //free(current);
    //free(msg);

    print("leaving maintenance");

}

#endif
