
#ifndef MAINTENANCE_MANAGER_H
#define MAINTENANCE_MANAGER_H
#include "main.h"

int generate(int min, int max){
    srand(time(NULL));
    int r= ( (rand() % (max+1-min))+ min);
    return r;
}


void * read_msg_queue(int value/*void * args*/){
	//int value=*((int *) args);
    msg_struct reply;
    msg_struct msg;

    while(simulation_status()>=0){
    
        if(msgrcv(mqid,&reply,sizeof(msg_struct)- sizeof(long),value, IPC_NOWAIT)==-1)
        {
            /*
            if(errno==ENOMSG){
            	print("msg queue is empty");
            	}
            else print("error msgrcv maintenance: %s", strerror(errno));
        	*/
        }
        else{
            //response to server
            msg = (msg_struct) {(long) reply.mtype, 0};
            msgsnd(mqid, &msg, sizeof(msg_struct)-sizeof(long), 0);
        }
    }
    return NULL;

}


//TODO
void maintenance_manager() {
	if(simulation_status()<0) return;
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
    //print("maintenance searching");
    //if its ending dont do maintenance
        count=0;
        valid=true;
        maintenance=generate(0, config->edge_server_number-1);
        //print("maintenance is %d, should be 0,1 or 2 config: %d", maintenance, config->edge_server_number);

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
            maintenance_time=generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
            msg = (msg_struct) {(long) maintenance, maintenance_time};
            msgsnd(mqid, &msg, sizeof(msg_struct)- sizeof(long), 0);
            print("maintenance:valid, we are sending a msg %d %d", msg.mtype, msg.maintenance_time);
        }
        else{
            print("maintenance: tried server %d, couldnt", maintenance);
        }

        maintenance_time= generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
        sleep(maintenance_time * 5); //interval between maintenance

    }
    
    //free(current);
    //free(msg);
    pthread_join(thread_maintenance,NULL);
    wait(NULL);
    print("maintenance: exit");
    exit(EXIT_SUCCESS);

}

#endif
