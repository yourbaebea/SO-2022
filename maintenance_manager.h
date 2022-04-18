
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

bool check(server_struct * s){
//TODO
	return true;
}




//TODO
void maintenance_manager() {
	write_log("PROCESS MAINTENANCE MANAGER CREATED");
    int maintenance_time;
    long maintenance;
    int i, sum;
    bool okay=true;
    server_struct * current;
    
    msg_struct msg;

    while(1){
    	printf("inside maintenance manager");
        if(simulation_status()==false) break;
        
        maintenance=generate(0, config->edge_server_number-1);
        current= shm->server;
        okay=true;

        //try to do maintenance in random server
        for(i=0, sum=0 ;i<config->edge_server_number;i++){
            
            if(check(current)==true){
            	sum++;
            	if(i==maintenance){ //when we are trying to do maintenance on an already on maintenance server
            	okay=false;
            }
              
        }
        
        //if all servers are in maintenance
        if(sum>= config->edge_server_number-1) okay=false;
        //if not all servers are in maintenance
                   
       if(okay==true){
       	maintenance_time=generate(MAINTENANCE_MINIMUM,MAINTENANCE_MAXIMUM);
       	msg = (msg_struct) {(long) maintenance, maintenance_time};
       	
       	if(msgsnd(mqid,&msg,sizeof(msg)- sizeof(long), 0)>0){
		    write_log("Error sending message to message queue\n");
		    end(EXIT_FAILURE);
		}
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
