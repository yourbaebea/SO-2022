
#include "main.h"

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
    writes_log(DEB, "[Control Tower] Terminating receives_messages");
}

int generate_time(){

}




//TODO
void maintenance_manager() {

    int maintenance_time;
    server * current=NULL;

    while(1){
        if(check_status()) break;
        shm->server
        while(i<config.edge_server_number){
            check(shm->server)
        }



        
        
        
        usleep(generate_time*1000); //interval between maintenance


    }
    
    sem_wait(mutex_shm);
    if(shm->started==true){
      time_t t;
      srand((unsigned) time(&t));
      for(int i=0; i<config.teams; i++){
		    if(shm->teams[i].name==NULL) break;
        for(int j=0; j<config.max_cars_per_team; j++){
         // if(shm->teams[i].list_of_cars[j]==NULL || shm->teams[i].list_of_cars[j].breakdown) break;
          if(shm->teams[i].list_of_cars[j].reliability>= (rand() % 100 + 1)){
            //ex: if reliability is 95%, only when random value is between 95 and 100 (5%) generates breakdown

            // new message to that car, we are sending the team pos and car pos in the team to make it easier to search,
            // im not sure this is needed bc this message is read in the thread so we can edit the thread itself
            //and not the shm list
            Message msg;
            msg.team= i;
            msg.car= j;
            if(msgsnd(mqid,&msg,sizeof(msg)- sizeof(long), 0)>0){
              write_log("Error sending message to message queue\n");
              end(EXIT_FAILURE);
            }

            if(DEBUG) printf(msg.mtype);
          }
        }
      }        
    }
	}
  sem_post(mutex_shm);
      
  usleep(config.time_unit*1000* config.breakdown_interval); //time in between T_Avaria
}

    if(msgsnd(mqid,&msg,sizeof(msg)- sizeof(long), 0)>0){
        write_log("Error sending message to message queue\n");
        end(EXIT_FAILURE);
    }



    writes_log("INSIDE MAINTENANCE");



}
