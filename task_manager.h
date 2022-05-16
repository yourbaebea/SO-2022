#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include "main.h"


void print_list(){
	printf("- current task list: ");
	task_struct * p=tasklist->first;
	if(p==NULL){
		print("EMPTY\n");
		return;
	}
	
	while(p!=NULL){
		printf("%d ->", p->id);
		p=p->next;
	}
	printf("end\n");
}



void print_task(task_struct * t){
    if(t==NULL) return;
    if(debug) printf("Task: %d, final time: %d, init: %d max: %d priority: %d\n", t->id, t->time_start+ t->time_max, t->time_start, t->time_max, t->priority);
}

//transform from pipe to task
bool new_task(char * buffer){
    char temp [BUF_SIZE];
    strcpy(temp,buffer);
    int id,instructions,max_time;
    //ID tarefa:Nº de instruções (em milhares):Tempo máximo para execução
    if(sscanf(temp, "%d:%d:%d", &id, &instructions, &max_time)==3){
        //bool value = create_task(id,instructions,max_time);

        task_struct * task= (task_struct *) malloc(sizeof(task_struct));
    
        task->id = id;
        task->instructions= instructions;
        task->time_max= max_time;
        task->time_start = current_time();

        insert_task_list(task);
        return true;
    }
    return false;

}

void insert_task_list(task_struct * task){
	print_task(task);

    pthread_mutex_lock(&shm->stats_mutex);
    shm->stats->tasks_total++;
    pthread_mutex_unlock(&shm->stats_mutex);

    print("insert in tasklist");

    //wait
    pthread_mutex_lock(&tasklist->mutex);
    if(tasklist->count>=tasklist->queue_max){
    	pthread_mutex_unlock(&tasklist->mutex);
    	return;
    }
    print("inside tasklist");
    if(tasklist->first==NULL){
        tasklist->first = task;
        
    }
    else{
    	task_struct * p= tasklist->first;
	while(p->next!=NULL){
	    p = p->next;
	}
	    p->next= task;
    }
    print("after adding in tasklist");
    print_list();
    
    tasklist->count++;
    
    pthread_cond_broadcast(&shm->scheduler);
    print("leaving tasklist");
    pthread_mutex_unlock(&tasklist->mutex);

    //TODO broadcast the condvariable
    print("broadcasting cond var scheduler");
    //pthread_cond_signal(&shm->scheduler);

}

void remove_task(task_struct * before, bool success){

    if(success){
        print("Task %d is being sent to the cpu");
    }
    else{
        write_log("Task %d expired and was never executed", before->next->id);
        //print_task(current);
        pthread_mutex_lock(&shm->stats_mutex);
        shm->stats->tasks_refused++;
        pthread_mutex_unlock(&shm->stats_mutex);

    }
    
    if(tasklist->first==NULL){
    	print("tasklist is empty");
    	return;
    }
    
    if(before->next==NULL){
    	print("task to remove is empty");
    	return;
    }
    
    tasklist->count--;
    
    task_struct * del;
	
    if(before==NULL){
        print("deleting first in list");
        del=tasklist->first;
        tasklist->first = tasklist->first->next;
        del->next=NULL;
        free(del);
        print_list();
        
        return;
    }
    
    print("deleting inside list");
    del=before->next;
    before->next= before->next->next;
    del->next=NULL;
    free(del);
    print_list();
    return; 
}

bool write_unnamed_pipe(task_struct * current){
    int i;
    server_struct * temp;

    for(i=0; i< config->edge_server_number; i++){
        if(i==0){
            temp=shm->server;
        }
        else{
            temp = temp->next;
        }
	print("task manager bf server mutex");
        pthread_mutex_lock(&temp->server_mutex);
        if(!temp->stopped){
        if( (!temp->cpu1->busy && temp->cpu1->active) || (!temp->cpu2->busy && temp->cpu2->active)){
            write(temp->p[1], current, sizeof(task_struct));
            /*
            struct dirent data;
            close(fileStatusPipe[1]);
            while(read(fileStatusPipe[0],&data,sizeof(data)) > 0)
            {
                printf("%ld: %s\n", data.d_ino, data.d_name);
            }
            close(fileStatusPipe[0]);
            exit(0);
            */
           pthread_mutex_unlock(&temp->server_mutex);
           print("task manager af server mutex");
           return true;
        }
        }
        pthread_mutex_unlock(&temp->server_mutex);
        print("task manager af server mutex");


    }

    print("there was a free cpu but the server probably went to maintenance, we are just ignoring and retrying when another becomes available!");
    return false;
}

void read_task_pipe(){

    
    print("task manager started reading from pipe");

    int fd;
    char buffer[BUF_SIZE];
    char message[BUF_SIZE];

    if ((fd = open(TASK_PIPE, O_RDWR)) < 0){
        print("Error when opening pipe");
        end(EXIT_FAILURE);
    }
        

    while(simulation_status()>=0){  //if waiting to stop we stop reading from pipe!

        if(simulation_status()<0) break;

        for (int n = 0; read(fd, buffer, BUF_SIZE) && n > 0 && buffer[n] != '\0' && buffer[n] != '\n'; n = read(fd, buffer, sizeof(char)));
        if (strcmp(buffer, "EXIT") == 0) {
            terminate();
            return;
        }
        else{
            if (strcmp(buffer, "STATS") == 0) {
            print_stats();
            }
            else{
                if(new_task(buffer)==false){
                    strncpy(message, "WRONG COMMAND =>", BUF_SIZE);
                }
                else{
                    strncpy(message, "VALID COMMAND =>", BUF_SIZE);
                }
                
                strcat(message,buffer);
                write_log(message);
            }  
        }

    }

    //TODO stop reading from pipe
    close(fd);

    print("task manager stopped reading from pipe");

}

void * scheduler(){
    //shm_struct * shm = (shm_struct *) args;

    print("THREAD SCHEDULER CREATED");
    //int score;

    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside scheduler");
        
        pthread_mutex_lock(&shm->scheduler_mutex);
        print("waiting cond var scheduler");
        pthread_cond_wait(&shm->scheduler,&shm->scheduler_mutex);
        print("cond var scheduler wait done!");
        //pthread_mutex_unlock(&shm->scheduler_mutex);

        if(simulation_status()<0){
            break;
        }

	print("SCHEDULER HERE");
        //sem_wait(&sem_tasks);
        print("SCHEDULER HERE INSIDE SEM");
        pthread_mutex_lock(&tasklist->mutex);
	task_struct * p=tasklist->first;
	print_list();

        while(p!=NULL && p->next!=NULL){
            
            p->next->priority= p->next->time_start + (current_time() - p->next->time_max + p->next->time_start);
            //lowest priority is the most urgent!
            // time start -> the oldest has a lower time
            // currenttime - maxwaittime -> how long before it expires, the lowest the more urgent
		print("inside scheduler");
		print_task(p->next);
            
            if(p->next->time_max + p->next->time_start <= current_time()){
            	print("%d <= %d line 220\n", p->next->time_max + p->next->time_start, current_time());
                remove_task(p, false);
            }
            else{
                p=p->next;
                //when we delete something and replace it we need to check it again, so we use this else
            }

        }

        p= tasklist->first;

        //checking the first task in list last to make sure if we need to delete we are replacing with an already verified tasks
        if(p!=NULL){
        
            p->priority= p->time_max - (current_time() - p->time_start) ;
            print("%d <= %d line 238\n", p->time_max + p->time_start, current_time());
            if(p->time_max + p->time_start <= current_time()){
            print("%d <= %d line 238\n", p->time_max + p->time_start, current_time());
            remove_task(NULL,false);

        	}
        }
        
        
        print("SCHEDULER HERE AFTER");
	print_list();
	pthread_mutex_unlock(&tasklist->mutex);
        //sem_post(&sem_tasks);
        

	pthread_mutex_unlock(&shm->scheduler_mutex);
        //dont post sem, we need the sem to only be released in the insert_task_list


        sleep(1);
        //THIS SHOULD NOT HAVE THIS SLEEP 1 IT ONLY SHOULD BE CALLED WHEN THE SEM IS RELEASED!!!!!!!!!!!
    }

    //free(p);


    pthread_exit(NULL);
    return NULL;

}

void * dispacher(){
    //shm_struct * shm = (shm_struct *) args;
    
    print("THREAD DISPACHER CREATED");



    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside dispacher");

        //pthread_mutex_lock(&shm->dispacher_mutex);
        print("waiting cond var dispacher");
        pthread_cond_wait(&shm->dispacher,&shm->dispacher_mutex);
        print("cond var dispacher wait done!");
        //pthread_mutex_unlock(&shm->dispacher_mutex);
	bool okay=true;
	//int count;
	while(okay==true){
		pthread_mutex_lock(&shm->dispacher_mutex);
		print("count dispacher %d", shm->count_dispacher);
		if(shm->count_dispacher>=1){
			okay=true;
			//count_dispacher--;
		}
		else{okay=false;}
		//this will be the last cycle of this while
		pthread_mutex_unlock(&shm->dispacher_mutex);
		
		if(simulation_status()<0){
		    okay=false; //just to be safe
		}
		
		if(okay==true){
			pthread_mutex_lock(&tasklist->mutex);
			bool check= dispatch_task();
			pthread_mutex_unlock(&tasklist->mutex);
			
			 if(check==true){
			 	pthread_mutex_lock(&shm->dispacher_mutex);
				print("success dispacher");
				shm->count_dispacher--;
				pthread_mutex_unlock(&shm->dispacher_mutex);
		
		 	}
		 	else{
		 		print("dispatch did not work probably bc tasklist is empty! We could add here a cond var scheduler to know when a new task is added but not worth it!");
		 		sleep(1);
		 	}
		}
		
        }

        //TODO
    }

    pthread_exit(NULL);
    return NULL;

}



bool dispatch_task(){
        task_struct * p;
        int next;
        task_struct * save_before;
        task_struct * save_current;

        p= tasklist->first;
        next=INT_MAX;
        
        if(p==NULL){
        	print("dispatch is empty, no tasks!");
        	return false;
        }
        
        while(p!=NULL && p->next!=NULL){

            if(p->next->time_max + p->next->time_start <= current_time()){
            print("%d <= %d line 294\n", p->next->time_max + p->next->time_start, current_time());
                remove_task(p,false);
            }
            else{
                if(p->next->priority< next){
                next=p->next->priority;
                save_before=p;
                save_current=p->next;
                print("new next tasked saved! value: %d", next);
                }

                p=p->next;

            }          

        }

        p= tasklist->first;

        //checking the first task in list last to make sure if we need to delete we are replacing with an already verified tasks
        if(p!=NULL){
            if(p->time_max + p->time_start <= current_time()){
            print("%d <= %d line 316\n", p->time_max + p->time_start, current_time());
                remove_task(NULL,false);
                
            }
            else{
                if(p->priority< next){
                    next=p->priority;
                    save_before=NULL;
                    save_current=p;
                    print("(first in list) new next tasked saved! value: %d", next);
                }
            }
        
        }
        
        if(tasklist->first==NULL || next==INT_MAX ){
        	print("after searching all, tasklist is empty, we deleted all");
        	return false;
        }

        if(write_unnamed_pipe(save_current)){
		print("%d <= %d line 332\n", p->time_max + p->time_start, current_time());
		remove_task(save_before, true);
		return true;
         }
         return false;
            //DISPACHER CAN BE USED WHEN IN MAINTENANCE DONT REMOVE IN CASE IT IS, already done!
            
}

//TODO
void task_manager() {
    //ignore signal
    write_log("PROCESS TASK_MANAGER CREATED");
    
    tasklist = (tasklist_struct *) malloc(sizeof(tasklist_struct));
    tasklist->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    if( pthread_mutex_init(&tasklist->mutex,NULL)!=0){
    	print("mutex tasklist error");
    	end(EXIT_FAILURE);
    }
    pthread_mutex_lock(&tasklist->mutex);
    tasklist->queue_max=config->queue_pos;
    tasklist->count=0;
    tasklist->first=NULL;
    pthread_mutex_unlock(&tasklist->mutex);
    
    //shm->tasklist=NULL;
    //print_task(shm->tasklist);
    
    //remove_task(NULL,false);
    
    //print("config nr: %d",config->edge_server_number);
    
    pthread_create(&thread_time, NULL, time_update, NULL);

    
    pthread_t thread_scheduler, thread_dispacher;
    
    pthread_create(&thread_scheduler, NULL, scheduler, NULL);
    pthread_create(&thread_dispacher, NULL, dispacher, NULL);

    int i;
    //j=0;
    for(i=0; i< config->edge_server_number; i++){
        if(fork()){
        	print("--- server %d of %d", i,config->edge_server_number);
            edge_server(i);
        }
    }
    
  
    read_task_pipe();
	

    wait(NULL);
    //keep the process alive untill the threads end

}

#endif
