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

//#include "main.c"


#define LOG_FILE "log.txt"
#define TASK_PIPE "TASK_PIPE"
#define BUF_SIZE 1024
#define PATH "C:\\Users\\Ana\\Desktop\\SO\\project\\"

//---------------------- structs ---------------------------------


// Message struct
typedef struct{
    long mtype; //strtol(server.name) 
    int maintenance_time;
} msg_struct;

typedef struct server_node_aux server_node_next;
typedef struct server_node_aux {
    char * name;
    int cpu1;
    int cpu2;
    server_node_next * next;
}server_node;


// Config struct
typedef struct{
    int queue_pos;
    int max_wait;
    int edge_server_number;
    server_node * server_info;
} config_struct;


typedef struct{
    pthread_mutex_t cpu_mutex;
    int mips;
    bool active;
    bool busy;
    //missing stuff?
} cpu_struct;



// servers structs
typedef struct server_struct_aux server_struct_next;
typedef struct server_struct_aux{
    int state;//= status.NORMAL;
    cpu_struct cpu1;
    cpu_struct cpu2;
    int performance; //lvl of performance
    int active_cpus;//=1; //default value for number of cpu is 1

    int tasks_done;//=0;
    int maintenance;//=0;

    server_struct_next * next;

} server_struct;


// SHM struct
typedef struct{
    int time;
    int status;//=1; // 1 ready/running, -1 needs to end/waiting to end, -2 end

    int tasks_total;//=0;
    int tasks_done;//=0;
    int total_time_response;//=0;
    int tasks_refused;//=0;

    //servers array data pointer
    server_struct * server;

    pthread_mutex_t time_mutex, log_mutex, stats_mutex, servers_array_mutex;

} shm_struct;


//---------------------- global vars ---------------------------------
bool debug=false;
enum status{ NORMAL, HIGH, STOPPED}; //how to set the server status server.status= status.NORMAL

FILE* log_file;                 // Log file pointer
shm_struct * shm;
config_struct * config;

//TODO
int mqid, shmid, servers_shmid; // Message Queue | Shared memory
shm_struct * shm;                // Shared memory shm_struct struct
config_struct * config;          // Config struct
pthread_mutexattr_t attrmutex;  // Mutexes attributes
pthread_condattr_t cattr;       // Condition variables attributes
sigset_t block_sigint;          // Signal set
pid_t ppid;

pthread_t thread_time;          // Update time in shared memory




//---------------------- functions ---------------------------------

//main
void print(char * message);
void write_log(char * message);
void clear_log();
bool read_config(char * config_file);
server_node * read_server_info(char * line);
bool read_config_by_line(char * config_file);
void add_node(server_node * temp);
void start(char * config_file);
void end(int status);

int generate(int min, int max);
bool check(server_struct * s);
void maintenance_manager();
void edge_server(int id);
void monitor();
bool check_status();
bool task_format(char * buffer);
bool create_task(int id,int instructions, int max_time);
void read_pipe();
void task_manager();
void * cpu(void* cpu_shm);

#endif
