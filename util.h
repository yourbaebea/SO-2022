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
    int simulation=shm->status;
    if(simulation==-1 && shm->server_status==-1){
        shm->status=-2;
        //TODO broadcast the condvariable
        print("broadcasting cond vars to end the scheduler and dispatcher");
        pthread_cond_signal(&shm->scheduler);
        pthread_cond_signal(&shm->dispatcher);
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
    //memset(buffer, 0, strlen(buffer));
    va_list args2;
    va_start(args2,message);
    vsprintf(buffer, message,args2);
    char timeformat [80];
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeformat, 80,"[%I:%M:%S] ",timeinfo);
    
    pthread_mutex_lock(&shm->log_mutex);
    
    fprintf(log_file, "%s %s \n", timeformat, buffer);
    fflush(log_file);
    
    va_start(args2, message);
    // write in the terminal
    printf("%s ",timeformat);
    vprintf(message,args2);
    printf("\n");
    fflush(stdout);
    va_end(args2);

    pthread_mutex_unlock(&shm->log_mutex);

}

// called by the signal or by the named pipe to end the simulation
void terminate(){
	write_log("SIMULATION ENDING...");
	signal(SIGINT, SIG_IGN);
	pthread_mutex_lock(&shm->status_mutex);
        shm->status=-1;
    	pthread_mutex_unlock(&shm->status_mutex);
    	//print("ending...");
}

//print the stats
void print_stats(){
	char stats[10000];
	
	strcpy(stats, "\n\tSTATISTICS\n");
	
	char buffer[BUF_SIZE];
	pthread_mutex_lock(&shm->stats_mutex); // not sure if its absolutely needed here
	float wait_time=0;
	if(shm->stats->tasks_done!=0){
		wait_time= shm->stats->total_time_response/shm->stats->tasks_done;
	}
	
	sprintf(buffer, "\t\ttotal tasks: %d\n\t\tdone tasks: %d\n\t\trefused tasks: %d\n\t\taverage wait time: %.2f\n \nServer\t\tTasks\t\tMaintenance\n", shm->stats->tasks_total, shm->stats->tasks_done, shm->stats->tasks_refused, wait_time);
	strcat(stats, buffer);
	
	
	server_node * temp = config->server_info;
	int i;
	for(i=0; i< config->edge_server_number; i++){
	
		sprintf(buffer, "%s\t\t%d\t\t%d\n", temp->name, shm->stats->tasks_by_server[i], shm->stats->op_by_server[i]);
		
		strcat(stats,buffer);
		temp=temp->next;
	}
	//free(temp);
	
	pthread_mutex_unlock(&shm->stats_mutex);
	
	
	strcat(stats, "------------------------------------------\n");
	
	write_log(stats);
}

//delete all and end
void end(int status){
	//called by task manager
	int i;
	
    write_log("CLEARING MEMORY TO END SIMULATION");
    
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

    sem_destroy(&sem_tasks);
    
    
    //config
    //TODO
    
    
    
    
    print("ALL RESOURCES WERE CLEARED, EXITING");
    exit(status);
}

//used in thread time for update
void * time_update() {
    //sigprocmask(SIG_BLOCK, block_sigint, NULL); //we need to block the sigint signal TODO
    
    //TODO this thread is still not working idk whats wrong
    int a=0, b=0,c=0;
    while(simulation_status()==0){
    	b=shm->count_init;
    	if(b!=a) a=b;
    	if(b==a){
    		print("init count= %d", shm->count_init);
    		c++;
    	}
    	pthread_mutex_lock(&shm->simulationstarted_mutex);
    	if(shm->count_init>=(config->edge_server_number*3)){
    		shm->status=1;
    		shm->server_status=1;
    		pthread_mutex_lock(&shm->time_mutex);
		    //print("thread updating time started");
		    shm->time=0;
		    pthread_mutex_unlock(&shm->time_mutex);
		    write_log("OFFLOAD SIMULATOR STARTING");
		    pthread_cond_broadcast(&shm->simulationstarted);
    		    //print("status broadcasted");	
    		   
    	}
    	if(c==3){
    		printf("something when wrong the last time and the memory was not cleared!");
    		shm->status=-1;
    		shm->server_status=-1;
    	}
    	
    	
    	pthread_mutex_unlock(&shm->simulationstarted_mutex);
    	sleep(1);
    }
      

   // write_log("OFFLOAD SIMULATOR STARTING");

    while (simulation_status() >=-1) {
    	print("time: %d", shm->time);
        pthread_mutex_lock(&shm->time_mutex);
        shm->time++;
        pthread_mutex_unlock(&shm->time_mutex);
        sleep(1);
    }
    
    print("time update is ending, status: %d", shm->status);

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
