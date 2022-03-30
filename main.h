/* SO 2021/22 Ana Beatriz Marques 2018274233 */

#ifndef MAIN_H
#define MAIN_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <regex.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

//---------------------- include other files ---------------------------------
#include "maintenance_manager.h"
#include "task_manager.h"
#include "edge_server.h"
#include "monitor.h"


#define LOG_FILE "log.txt"
#define TASK_PIPE "TASK_PIPE"
#define BUF_SIZE 1024


//---------------------- global vars ---------------------------------
bool debug=false;
enum status{ NORMAL, HIGH, STOPPED}; //how to set the server status server.status= status.NORMAL

FILE* log_file;                 // Log file pointer
shm_struct* shm;
config_struct * config;

//TODO
int mqid, shmid, servers_shmid; // Message Queue | Shared memory
shm_struct* shm;                // Shared memory shm_struct struct
config_struct* config;          // Config struct
pthread_mutexattr_t attrmutex;  // Mutexes attributes
pthread_condattr_t cattr;       // Condition variables attributes
sigset_t block_sigint;          // Signal set
pid_t ppid;


//---------------------- structs ---------------------------------

// SHM struct
typedef struct{
    int time;
    int status=1; // 1 ready/running, -1 needs to end/waiting to end, -2 end

    int tasks_total=0;
    int tasks_done=0;
    int total_time_response=0;
    int tasks_refused=0;

    //servers array data pointer
    server_struct * server;

    // Mutexes ???????
    //pthread_mutex_t end_mutex, runways_mutex, time_mutex, log_mutex, stdout_mutex, stats_mutex, servers_array_mutex;

} shm_struct;

// Message struct
typedef struct{
    long mtype; //strtol(server.name) 
    int maintenance_time;
} msg_struct;

// Config struct
typedef struct{
    int queue_pos;
    int max_wait;
    int edge_server_number;
    server_node * server_info;
} config_struct;


//Server details node only temporary
typedef struct{
    char[BUF_SIZE] name;
    int cpu1;
    int cpu2;
    server_node * next;
} server_node;


// servers structs
typedef struct{
    //pthread_cond_t cond_var; // can we still use this???
    int state= status.NORMAL;
    cpu_struct[2] cpu;
    int performance; //lvl of performance
    int active_cpus=1; //default value for number of cpu is 1

    int tasks_done=0;
    int maintenance=0;

    server_struc * next;

} server_struct;


typedef struct{
    pthread_t thread;
    pthread_cond_t cond_var;
    int mips;
    bool active;
    //missing stuff?
} cpu_struct;


//---------------------- functions ---------------------------------

//main
int print(char * message);
void write_log(char * message);
void clear_log();
bool read_config(char * config_file);
void start(char * config_file);
void end(int status);


#endif
