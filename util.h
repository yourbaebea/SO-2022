#ifndef UTIL_H
#define UTIL_H
#include "main.h"

//functions used in the whole project but arent specific


int simulation_status(){
    //do i need the mutex here?
    //return shm->status;
    //TODO
    return 1; //running
}


void print(char * message, ...){
    if(debug){
    va_list args;
    va_start(args,message);
    printf("DEBUG: ");
    vprintf(message,args);
    printf("\n");
    }
}

void write_log(char * message){

	char timeformat [80];
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeformat, 80,"[%I:%M:%S]",timeinfo);
    
    pthread_mutex_lock(&shm->log_mutex);

    fprintf(log_file, "%s %s \n", timeformat, message);
    fflush(log_file);
    
    // write in the terminal
    printf("%s %s\n",timeformat,message);

    pthread_mutex_unlock(&shm->log_mutex);

}


// called by the signal or by the named pipe to end the simulation
void terminate(){
	//update shm status to ending, this should end all threads EXCEPT the time, monitor, task manager, etc etc
	/* 
	wait for it to end:
	- search the servers status, when all are not running its done
	- finish time thread
	- print_stats();
	- end(EXIT_SUCCESS);
	
	*/
	

}

void print_stats(){
	char stats[10000];
	char buffer[BUF_SIZE];
	strcpy(stats, "\tSTATISTICS\n");
	/*
	pthread_mutex_lock(&shm->stats_mutex); // not sure if its absolutely needed here


	float wait_time= shm->stats.total_time_response/shm->stats.tasks_done;
	
	sprintf(buffer, "\t\ttotal tasks: %d\n\t\tdone tasks: %d\n\t\trefused tasks: %d\n\t\taverage wait time: %.2f\n \nServer\t\tTasks\t\tMaintenance\n", shm->stats.tasks_total, shm->stats.tasks_done, shm->stats.tasks_refused, wait_time);
	strcat(stats, buffer);
	
	
	server_node * temp = config->server_info;
	int i;
	for(i=0; i< config->edge_server_number; i++){
	
		sprintf(buffer, "%s\t\t%d\t\t%d\n", temp->name, shm->stats->tasks_by_server[i], shm->stats->op_by_server[i]);
		
		strcat(stats,buffer);
		temp=temp->next;
	}
	free(temp);
	
	pthread_mutex_unlock(&shm->stats_mutex);
	

	*/
	
	strcat(stats, "----------------------------------\n");
	
	write_log(stats);
}



void end(int status){
    print("ENDING THE SIMULATION.......");
    
    fclose(fopen(LOG_FILE, "w"));
    //TODO
    
    
    print("ALL RESOURCES WERE CLEARED, EXITING");

    exit(status);
}



void * time_update() {
    //sigprocmask(SIG_BLOCK, block_sigint, NULL); //we need to block the sigint signal TODO
    print("thread updating time");
    
    //TODO this thread is still not working idk whats wrong
    
    pthread_mutex_unlock(&shm->time_mutex);

    while (simulation_status() > 0) {
    	print("thread updating time");
        pthread_mutex_lock(&shm->time_mutex);
        shm->time++;
        pthread_mutex_unlock(&shm->time_mutex);
        usleep(1000);
    }
    
    printf("\n\nHEREsimulation status is %d", shm->status);

    pthread_exit(NULL);
    return NULL;
}




#endif
