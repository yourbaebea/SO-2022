#ifndef UTIL_H
#define UTIL_H
#include "main.h"

//functions used in the whole project but arent specific

//future function that checks the status of the simulation in the shm
//this probably should require a cond var TODO
int simulation_status(){
    //do i need the mutex here?
    //return shm->status;
    //TODO
    pthread_mutex_lock(&shm->status_mutex);
    int simulation= shm->status;
    int servers= shm->server_status;
    if(servers==status.STOPPED && simulation==-1){
        shm->status=-2;
        //TODO broadcast the condvariable
        print("broadcasting cond vars to end the scheduler and dispacher");
        pthread_cond_signal(&shm->scheduler);
        pthread_cond_signal(&shm->dispacher);
    }
    pthread_mutex_unlock(&shm->status_mutex);

    return simulation;

}

//just for debug of messages
void print(char * message, ...){
    if(debug){
    va_list args;
    va_start(args,message);
    printf("DEBUG: ");
    vprintf(message,args);
    printf("\n");
    va_end(args);
    }
}

//write info in log and terminal at the same time
void write_log(char * message, ...){
    char buffer[BUF_SIZE];
    va_list args2;
    va_start(args2,message);
    vsprintf(buffer, message,args2);
    va_end(args2);

	char timeformat [80];
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeformat, 80,"[%I:%M:%S]",timeinfo);
    
    pthread_mutex_lock(&shm->log_mutex);

    fprintf(log_file, "%s %s \n", timeformat, buffer);
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

//print the stats
void print_stats(){
	char stats[10000];
	
	strcpy(stats, "\tSTATISTICS\n");
	/*
	char buffer[BUF_SIZE];
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

//delete all and end
void end(int status){
	int i;
    print("ENDING THE SIMULATION.......");
    
    //tasks -> delete all tasks on linked list, doesnot need to update stats
    //TODO
    
    //log
    fclose(fopen(LOG_FILE, "w"));
    
    //pipe
    //TODO
    
    //shm -> servers, cpus, stats and shm
    server_struct * temp= shm->server;
    server_struct * p;
    
    for(i=0; i<config->edge_server_number; i++){
    	p=temp->next;
    	free(temp->cpu1);
    	free(temp->cpu2);
    	free(temp);
    	temp=p;
    }
    
    free(shm->stats->tasks_by_server);
    free(shm->stats->op_by_server);
    free(shm->stats);
    free(shm);
    
    
    //config
    //TODO
    
    
    
    
    print("ALL RESOURCES WERE CLEARED, EXITING");
    exit(status);
}

//used in thread time for update
void * time_update() {
    //sigprocmask(SIG_BLOCK, block_sigint, NULL); //we need to block the sigint signal TODO
    
    //TODO this thread is still not working idk whats wrong
    
    pthread_mutex_lock(&shm->time_mutex);
    print("thread updating time started");
    shm->time=0;
    pthread_mutex_unlock(&shm->time_mutex);

    pthread_mutex_lock(&shm->status_mutex);
    shm->status=1;
    shm->server_status=1;
    pthread_mutex_unlock(&shm->status_mutex);

    write_log("OFFLOAD SIMULATOR STARTING");

    while (simulation_status() > 0) {
    	print("time: %d", shm->time);
        pthread_mutex_lock(&shm->time_mutex);
        shm->time++;
        pthread_mutex_unlock(&shm->time_mutex);
        sleep(1);
    }
    
    print("\n\nHEREsimulation status is %d", shm->status);

    pthread_exit(NULL);
    return NULL;
}


//get the current time
int current_time(){
    pthread_mutex_lock(&shm->time_mutex);
    int current= shm->time;
    pthread_mutex_unlock(&shm->time_mutex);
    return current;
}



#endif
