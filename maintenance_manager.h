
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

}




//TODO
void maintenance_manager() {

    int maintenance_time, i;
    server_struct * current=NULL;

    while(1){
        if(check_status()==false) break;
        current= shm->server;

        //only try to do maintenance in random server
        i=0;
        while(i<config->edge_server_number){
            //check(current);
            //send message in mq
            /*
                if(msgsnd(mqid,&msg,sizeof(msg)- sizeof(long), 0)>0){
            write_log("Error sending message to message queue\n");
            end(EXIT_FAILURE);
        }
            */
            //TODO
        }

        usleep(generate(1,5)*1000); //interval between maintenance

    }

    print("leaving maintenance");

}

#endif