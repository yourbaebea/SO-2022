
#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include "main.h"

// wait_to_exit(); update shm status to -1 /waiting for things to end
// print_stats();

bool check_status(){
    int temp;
    temp=shm->status;
    //TODO missing mutex

    if (temp>0) return true;
    return false;
}

bool task_format(char * buffer){
    char temp [BUF_SIZE];
    strcpy(temp,buffer);
    int id,instructions,max_time;
    //ID tarefa:Nº de instruções (em milhares):Tempo máximo para execução
    if(sscanf(temp, "%d:%d:%d", &id, &instructions, &max_time)==3){
        bool value = create_task(id,instructions,max_time);
        return value;
    }
    return false;

}

//TODO
bool create_task(int id,int instructions, int max_time){
    //create and send to the linked list
    //if valid return true

    return true;
}


void read_pipe(){

    /*
    Task Manager
Processo responsável por gerir a recepção de tarefas através do named pipe e por as enviar para
execução num dos processos Edge Server. Cria os processos Edge Server (cada processo
representa um computador no Edge). Aceita comandos pelo named pipe com o nome TASK_PIPE.
Os comandos podem ser:
○ Nova tarefa para offloading, enviada pelos processos Mobile Node
○ Comando “EXIT” ou “STATS” que provocará respectivamente o fim do sistema de
offloading ou a escrita de estatísticas
○ Esses comandos serão escritos directamente pelo utilizador na linha de comando:
■ e.g., echo “EXIT” > TASK_PIPE
Quando uma nova tarefa de offloading chega através do named pipe são efetuadas as seguintes
operações:
● O pedido é colocado numa fila com tamanho máximo de QUEUE_POS (este parâmetro é
fornecido pelo ficheiro de configurações). Se a fila já estiver cheia, o pedido é eliminado e
essa informação é escrita no ecrã e no ficheiro de log.
● Uma thread scheduler analisa as várias tarefas na fila e atribui-lhes prioridades (1=
máxima prioridade). Sempre que um pedido novo chega e é colocado na fila, estas
prioridades são reavaliadas. Se durante a avaliação forem detetadas tarefas cujo prazo
máximo de execução já tenha passado, essas tarefas são eliminadas e essa informação
será escrita no log. O critério para a prioridade é baseado no tempo máximo para
execução que a tarefa tem e no seu tempo de chegada à fila. I.e., tarefas com limites
temporais de execução menor têm prioridade. O tempo máximo para execução é o tempo
máximo que a tarefa pode demorar até ser completamente executada, desde a altura em
que chegou ao Task Manager.
● Uma thread dispatcher pega na tarefa mais prioritária e verifica se é possível executá-la
no tempo disponível. I.e., verifica se o tempo que leva a executar em qualquer um dos
vCPU disponível dos Edge Server cumpre o tempo máximo limite indicado - tem de ter
em conta o tempo que ainda resta até ao limite temporal indicado e o tempo que levaria
a processar no vCPU disponível. Se não cumprir o tempo limite, não vale a pena executar
a tarefa, ela já não terá validade, pelo que a tarefa é eliminada e isso é escrito no log. A
thread dispatcher
    */
    
    print("task manager started reading from pipe");

    int fd;
    char buffer[BUF_SIZE];
    char message[BUF_SIZE];

    if ((fd = open(TASK_PIPE, O_RDWR)) < 0){
        print("Error when opening pipe");
        end(EXIT_FAILURE);
    }
        

    while(1){

        if(check_status()==false) break;

        for (int n = 0; read(fd, buffer, BUF_SIZE) && n > 0 && buffer[n] != '\0' && buffer[n] != '\n'; n = read(fd, buffer, sizeof(char)));
        if (strcmp(buffer, "EXIT") == 0) {
            //wait_to_exit();
            return;
        }
        else{
            if (strcmp(buffer, "STATS") == 0) {
            //print_stats();
            }
            else{
                if(task_format(buffer)==false){
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
    /*
        Uma thread scheduler analisa as várias tarefas na fila e atribui-lhes prioridades (1=
máxima prioridade). Sempre que um pedido novo chega e é colocado na fila, estas
prioridades são reavaliadas. Se durante a avaliação forem detetadas tarefas cujo prazo
máximo de execução já tenha passado, essas tarefas são eliminadas e essa informação
será escrita no log. O critério para a prioridade é baseado no tempo máximo para
execução que a tarefa tem e no seu tempo de chegada à fila. I.e., tarefas com limites
temporais de execução menor têm prioridade. O tempo máximo para execução é o tempo
máximo que a tarefa pode demorar até ser completamente executada, desde a altura em
que chegou ao Task Manager.
    */

    while(1){
        print("inside scheduler");


        //TODO





        usleep(1000);
    }



    pthread_exit(NULL);
    return NULL;

}

void * dispacher(){
    /*
    Uma thread dispatcher pega na tarefa mais prioritária e verifica se é possível executá-la
no tempo disponível. I.e., verifica se o tempo que leva a executar em qualquer um dos
vCPU disponível dos Edge Server cumpre o tempo máximo limite indicado - tem de ter
em conta o tempo que ainda resta até ao limite temporal indicado e o tempo que levaria
a processar no vCPU disponível. Se não cumprir o tempo limite, não vale a pena executar
a tarefa, ela já não terá validade, pelo que a tarefa é eliminada e isso é escrito no log. A
thread dispatcher é ativada sempre que 1 vCPU fica livre e desde que existam tarefas por
realizar.
    */

   while(1){
        print("inside dispacher");


        //TODO





        usleep(1000);
    }



    pthread_exit(NULL);
    return NULL;

}

//TODO
void task_manager() {
    write_log("PROCESS TASK_MANAGER CREATED");
    printf("TASKMANAGER PID: %d", getpid());

    //signal(SIGUSR1, print_stats);

    //create PROCESS of Servers, check shm for number and name
    //fork()
    int i;
    for(i=0; i< config->edge_server_number; i++){
        if(fork()){
            edge_server(i);
        }

    }

    
    read_pipe();

    print("creating threads: scheduler and dispacher");

    pthread_t thread_scheduler, thread_dispacher;
    

    pthread_create(&thread_scheduler, NULL, scheduler, NULL);
    pthread_create(&thread_dispacher, NULL, dispacher, NULL);





    wait(NULL);
    //keep the process alive untill the threads end

}

#endif
