/* SO 2021/22 Ana Beatriz Marques 2018274233 */
/*
things to see in the future
do i need to close the un pipes in each side
how to i stop the cond var wait errors?

*/
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
#include <stdarg.h>
#include <limits.h>

//---------------------- include other files ---------------------------------

//#include "main.c"

#define LOG_FILE "log.txt"
#define TASK_PIPE "TASK_PIPE"
#define BUF_SIZE 1024
#define MAINTENANCE_MINIMUM 1 // value defined by project but not in the config file
#define MAINTENANCE_MAXIMUM 5 // value defined by project but not in the config file
#define PATH "C:\\Users\\Ana\\Desktop\\SO\\project\\"

//---------------------- structs ---------------------------------

typedef int pipe_struct[2];

// Message struct
typedef struct
{
    long mtype;
    int maintenance_time;
} msg_struct;

// Config struct
typedef struct server_node_aux server_node_next;
typedef struct server_node_aux
{
    char *name;
    int cpu1;
    int cpu2;
    server_node_next *next;
} server_node;

typedef struct
{
    int queue_pos;
    int max_wait;
    int edge_server_number;
    server_node *server_info;
} config_struct;

// Task struct
// typedef struct task_struct_aux task_struct_next;
typedef struct /*task_struct_aux*/
{
    int id;
    int instructions;
    int time_max;
    int time_start;
    int time_acceptance;
    int time_needed;
    int time_waiting; // when accepted in cpu, do currenttime- creation time
    int priority;
    // task_struct_next * next;

} task_struct;

/*
// Task struct
typedef struct tasklist_struct_aux tasklist_struct_next;
typedef struct tasklist_struct_aux{
    pthread_mutex_t mutex;
    int count;
    int queue_max;
    task_struct_next * first;

}tasklist_struct;
*/

// cpu struct
typedef struct
{
    int mips;
    bool active;
    bool busy;
    pthread_cond_t task_available;
    pthread_mutex_t task_available_mutex; // for both status!!!!
    task_struct *task;                    // pointer to current task being done, no need to malloc space its only a pointer
    // missing stuff?
    // TODO
} cpu_struct;

// servers structs
typedef struct server_struct_aux server_struct_next;
typedef struct server_struct_aux
{
    pthread_mutex_t server_mutex;
    cpu_struct *cpu1;
    cpu_struct *cpu2;
    int p[2]; // pipe
    int id;
    char *name;
    int active_cpus; //=1; //default value for number of cpu is 1
    int tasks_done;  //=0;
    int maintenance; //=0;
    bool stopped;    //=false

    server_struct_next *next;

} server_struct;

// stats
typedef struct
{
    int tasks_total;         //=0;
    int tasks_done;          //=0;
    int total_time_response; //=0;
    int tasks_refused;       //=0;
    int *tasks_by_server;    //(int*) malloc(sizeof(int) * size)
    int *op_by_server;       //(int*) malloc(sizeof(int) * size)

} stats_struct;

// SHM struct
typedef struct
{
    int time;
    int status;        // 1 ready/running,0 hasnt started yet, -1 needs to end/waiting to end, -2 end
    int server_status; // 2 high, 1 normal, 0 hasnt started yet, -1 stopped;
    int count_init;
    int count_dispatcher;
    stats_struct *stats;
    // servers array data pointer
    server_struct *server;

    task_struct *tasklist;
    // add var to check current num de tasks

    pthread_mutex_t status_mutex, simulationstarted_mutex; // for both status!!!!
    pthread_mutex_t time_mutex, log_mutex, stats_mutex, scheduler_mutex, dispatcher_mutex, tasklist_mutex;
    // whenever we update the struct we need to lock this mutex

    // Condition variable
    pthread_cond_t scheduler, dispatcher;
    pthread_cond_t simulationstarted;

} shm_struct;

//---------------------- global vars ---------------------------------
bool debug = false;

FILE *log_file; // Log file pointer
shm_struct *shm;
config_struct *config;
// tasklist_struct * tasklist;

// TODO
int mqid;
// int shmid;
shm_struct *shm;       // Shared memory shm_struct struct
config_struct *config; // Config struct

pthread_t thread_time; // Update time in shared memory

sigset_t block_sigint;
sem_t sem_tasks;

//---------------------- functions ---------------------------------

// main
bool read_config(char *config_file);
server_node *read_config_aux(char *line);
void start(char *config_file);
bool init_mutex();

// util
int simulation_status();
void print(char *message, ...);
void write_log(char *message, ...);
void end(int status);
void terminate();
void print_stats();
void *time_update();
int current_time();

// task manager
bool new_task(char *buffer);
void insert_task_list();
void remove_task(task_struct *before, bool success);
bool write_unnamed_pipe(task_struct *current);
void read_task_pipe();
void *scheduler();
void *dispatcher();
bool dispatch_task();
void task_manager();
int find_index_shm();
void remove_shm(task_struct *t);
void insert_shm(task_struct *t);
void print_list_shm();

// edge server
void *cpu(void *args);
void *read_unnamed_pipe(void *args);
void edge_server(int id);

// maintenance manager
// TODO void receives_messages();
int generate(int min, int max);
bool check(server_struct *s);
void maintenance_manager();

// monitor
void monitor();

#endif
