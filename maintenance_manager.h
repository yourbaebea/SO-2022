
#ifndef MAINTENANCE_MANAGER_H
#define MAINTENANCE_MANAGER_H
#include "main.h"

/*
// Receive messages from MQ and answer back with SHM slot id
void receives_messages() {
    print()
    
    int valid = 0;
    Node node;
    msg_struct buffer, *temp;
    msg_reply_struct reply;

    int average_gap = (config->arrival_delay + config->arrival_duration + config->departure_delay + config->departure_duration) / 4;

    while (1) {
        // -2 argument makes prioritary messages first to read and doesn't read messages with mtype > 2 (meaning it will not read replies to flight threads)
        // Type TERMINATE (-3) ends thread
        msgrcv(mqid, &buffer, sizeof(msg_struct), -3, 0);
        if (buffer.mtype == TERMINATE)
            break;

        if ((buffer.flight_type == DEPARTURE && get_num_flights(DEPARTURE) < config->max_departures) || (buffer.flight_type == ARRIVAL && get_num_flights(ARRIVAL) < config->max_arrivals)) {
            temp = (msg_struct *) malloc(sizeof(msg_struct));
            if(temp == NULL){
                printf("Error on malloc\n");
                exit(-1);
            }
            buffer.slot_id = find_flight_slot();
            memcpy(temp, &buffer, sizeof(msg_struct));

            if (temp->flight_type == ARRIVAL)
                temp->time_takeoff = buffer.mtype == PRIORITARY ? 0 : get_value(&shm->current_time, &shm->time_mutex) + temp->time_ETA - average_gap;

            node = linked_list_insert(header_heu, temp, &heuristic_q_mutex, HEURISTIC_Q);
            reply = (msg_reply_struct) {temp->uid, temp->slot_id};
            valid = 1;
        } else { // Rejects flight
            reply = (msg_reply_struct) {buffer.uid, REJECT};
            increment_stat(&shm->stats.n_rejected_flights, 1);
        }
        // Answers flight
        msgsnd(mqid, &reply, sizeof(msg_reply_struct), 0);

        if (valid == 1 && flights_ahead(node) > 5)
            give_holding_order(node, 1);
    }
    write_log(DEB, "[Control Tower] Terminating receives_messages");
}
*/

int generate(int min, int max){
    srand(time(NULL));
    return (rand() % max)+ min;
}


void * read_msg_queue(int value){
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

}


//TODO
void maintenance_manager() {
    //ignore signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
	write_log("PROCESS MAINTENANCE MANAGER CREATED");
    pthread_t thread_maintenance;
    pthread_create(&thread_maintenance, NULL, read_msg_queue, INT_MAX);


    // Receives message from control tower
    msgrcv(mqid, &reply, sizeof(msg_struct), msg.uid, 0);


    /*
    int maintenance_time;
    long maintenance;
    int i, sum;
    bool okay=true;
    server_struct * current;
    
    msg_struct msg;
    */

   server_struct * temp;
   int count, maintenance, maintenance_time;
   bool valid;
   msg_struct msg;

    while(simulation_status()>=0){ //if its ending dont do maintenance
        count=0;
        valid=true;
        maintenance=generate(0, config->edge_server_number-1);

        for(i=0; i< config->edge_server_number; i++){
            if(i==0){
                temp=shm->server;
            }
            else{
                temp = temp->next;
            }

            pthread_mutex_lock(&temp->server_mutex);
            if(temp->stopped==true) count++;
            pthread_mutex_unlock(&temp->server_mutex);

            if(i==maintenance) valid=false; //when we are trying to do maintenance on an already on maintenance server


        }


        //if all servers are in maintenance
        if(count >= config->edge_server_number-1) valid=false;
        
                   
        if(valid==true){
            maintenance_time=generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
            msg = (msg_struct) {(long) maintenance, maintenance_time};
            msgsnd(mqid, &msg, sizeof(msg_struct), 0);
        }
        else{
            print("maintenance tried server %d, couldnt", maintenance);
        }
 
        sleep(generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM)*1); //interval between maintenance

    }
    
    //free(current);
    //free(msg);

    print("leaving maintenance");

}

#endif
