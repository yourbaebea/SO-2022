
#ifndef MONITOR_H
#define MONITOR_H
#include "main.h"

//TODO
void monitor() {
    write_log("INSIDE MONITOR");

    while(1){
        //update cpu status shm depending on lvl
        /*
        Processo responsável por controlar o número de vCPUs ativos dentro dos processos Edge Server.
        Para poupar energia cada servidor apenas tem por omissão 1 vCPU ativo.
        Sempre que a fila no processo Task Manager esteja mais de 80% preenchida e o tempo mínimo
        de espera para que uma nova tarefa seja executada for superior a MAX_WAIT segundos (ver
        ficheiro de configurações), o processo Monitor ativará o modo High performance de todos os Edge
        Server. Quando a fila descer a ocupação para 20% volta a ativar o modo de performance Normal.
        A troca de modo de performance será assinalada através de uma flag existente em memória
        partilhada.
        */
    }

}

#endif