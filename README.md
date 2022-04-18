# Simulador para Offloading de Tarefas no Edge
Projeto SO 2021/22

| METAS | data |
| --- | --- |
| Entrega intermédia | 18/04/2021- 22h00|
| Defesa | Aulas PL da semana de	18-24 |
| Entrega final | 13/05/2021-22h00 |
| Defesa | 16/05/2021 a	03/06/2021 |

### Links
[Relatório](https://docs.google.com/document/d/1tC55xaVY_QENM3Livz2rrTaNydmLdTHq6Gej8J2ZANE/edit)
[Esquema](https://docs.google.com/presentation/d/190iKgZj00AvN4xIBbyooPCvG89EpxFZdq5zakP03UZo/edit#slide=id.g11b2811ab0c_0_9)

## run
- make
- ./offload_simulator {configfile} [debug]
- ./mobile_node {total_tasks} {interval} {instructions} {max_time_task} [debug]


## Checklist

### Mobile nodes
- [ ] Criação do mobile node S
- [ ] Leitura correta dos parâmetros da linha de comando
- [ ] Geração e escrita das tarefas no named pipe
- [ ] System Manager
- [ ] Arranque do simulador de offloading, leitura do ficheiro de configurações, validação dos dados do ficheiro e aplicação das configurações lidas. S
- [ ] Criação da memória partilhada S
- [ ] Criação do named pipe
- [ ] Criação dos processos Task Manager, Monitor e Maintenance Manager S
- [ ] Criação da fila de mensagens
- [ ] Escrever a informação estatística no ecrã como resposta ao sinal SIGTSTP
- [ ] Capturar o sinal SIGINT, terminar a corrida e liberta os recursos

### Task Manager

- [ ] Criar os processos Edge Server de acordo com as configurações S 
- [ ] Ler e validar comandos lidos do named pipe
- [ ] Criação da thread scheduler e gestão do escalonamento das tarefas S (preliminar)
- [ ] Criação da thread dispatcher para distribuição das tarefas

### Edge Server

- [ ] Criação das threads que simulam os vCPUs S
- [ ] Executar as tarefas
### Monitor
- [ ] Controla o nível de performance dos Edge Server de acordo com as regras estabelecidas

### Maintenance Manager
- [ ] Gerar mensagens de manutenção, receber resposta e gerir a manutenção
### Ficheiro log
- [ ] Envio sincronizado do output para ficheiro de log e ecrã. S
### Geral
- [ ] Criar um makefile S
- [ ] Diagrama com a arquitetura e mecanismos de sincronização S (preliminar)
- [ ] Suporte de concorrência no tratamento de pedidos
- [ ] Deteção e tratamento de erros.
- [ ] Atualização da shm por todos os processos e threads que necessitem Sincronização com mecanismos adequados (semáforos, mutexes ou variáveis de condição) S (preliminar)
- [ ] Prevenção de interrupções indesejadas por sinais não especificados no enunciado; fornecer a resposta adequada aos vários sinais especificados no enunciado
- [ ] Após receção de SIGINT, terminação controlada de todos os processos e threads, e libertação de todos os recursos.
