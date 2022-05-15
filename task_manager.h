#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include "main.h"

void print_list(){
	printf("- current task list: ");
	task_struct * p=tasklist;
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


    //wait
    sem_wait(&sem_tasks);
    if(tasklist==NULL){
        tasklist = task;
        print_list();
        sem_post(&sem_tasks);
        return;
    }
    
    task_struct * p= tasklist;
	while(p->next!=NULL){
	    p = p->next;
	}
    p->next= task;
    print_list();

    sem_post(&sem_tasks);

    //TODO broadcast the condvariable
    print("broadcasting cond var scheduler");
    pthread_cond_signal(&shm->scheduler);

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
    
    if(tasklist==NULL){
    	print("tasklist is empty");
    	return;
    }
    
    if(before->next==NULL){
    	print("task to remove is empty");
    	return;
    }
    
    
    task_struct * del;
	
    if(before==NULL){
        print("deleting first in list");
        del=tasklist;
        tasklist = tasklist->next;
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
            pthread_mutex_unlock(&temp->server_mutex);
            close(temp->p[0]);
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

        //if(simulation_status()<0) break;

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

    print("THREAD SCHEDULER CREATED");
    task_struct * p;
    //int score;

    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside scheduler");
        
        pthread_mutex_lock(&shm->scheduler_mutex);
        print("waiting cond var scheduler");
        pthread_cond_wait(&shm->scheduler,&shm->scheduler_mutex);
        print("cond var scheduler wait done!");
        pthread_mutex_unlock(&shm->scheduler_mutex);

        if(simulation_status()<0){
            break;
        }


        sem_wait(&sem_tasks);

        p= tasklist;

        while(p!=NULL && p->next!=NULL){
            
            p->next->priority= p->next->time_start + (current_time() - p->next->time_max + p->next->time_start);
            //lowest priority is the most urgent!
            // time start -> the oldest has a lower time
            // currenttime - maxwaittime -> how long before it expires, the lowest the more urgent

	    print_task(p);
            
            if(p->next->time_max + p->next->time_start <= current_time()){
            	print("%d <= %d line 220\n", p->next->time_max + p->next->time_start, current_time());
                remove_task(p, false);
            }
            else{
                p=p->next;
                //when we delete something and replace it we need to check it again, so we use this else
            }

        }

        p= tasklist;

        //checking the first task in list last to make sure if we need to delete we are replacing with an already verified tasks
        if(p!=NULL){
        
            p->priority= p->time_max - (current_time() - p->time_start) ;
            
            if(p->time_max + p->time_start <= current_time()){
            print("%d <= %d line 238\n", p->time_max + p->time_start, current_time());
            remove_task(NULL,false);

        	}
        }
       

        sem_post(&sem_tasks);

        //dont post sem, we need the sem to only be released in the insert_task_list


        sleep(1);
        //THIS SHOULD NOT HAVE THIS SLEEP 1 IT ONLY SHOULD BE CALLED WHEN THE SEM IS RELEASED!!!!!!!!!!!
    }

    free(p);


    pthread_exit(NULL);
    return NULL;

}

void * dispacher(){
    
    print("THREAD DISPACHER CREATED");



    while(simulation_status()>=-1){  //while its running or waiting to stop
        //print("inside dispacher");

        //pthread_mutex_lock(&shm->dispacher_mutex);
        print("waiting cond var dispacher");
        pthread_cond_wait(&shm->dispacher,&shm->dispacher_mutex);
        print("cond var dispacher wait done!");
        //pthread_mutex_unlock(&shm->dispacher_mutex);
	bool okay=true;
	int count;
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
			sem_wait(&sem_tasks);
			bool check= dispatch_task();
			sem_post(&sem_tasks);
			
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

        p= tasklist;
        next=INT_MAX;
        
        if(p==NULL){
        	print("dispatch is empty, no tasks!");
        	return false;
        }
        
        while(p!=NULL && p->next!=NULL){

            if(p->next->time_max + p->next->time_start <= current_time()){
            print("%d <= %d line 294\n", p->time_max + p->time_start, current_time());
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

        p= tasklist;

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
        
        if(tasklist==NULL || next==INT_MAX ){
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
