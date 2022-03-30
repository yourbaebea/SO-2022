# SO 2021/22 Ana Beatriz Marques 2018274233
#
#
# how to use:
# - make
# - ./offload_simulator {configfile} [debug]
# - ./mobile_node {total_tasks} {interval} {instructions} {max_time_task} [debug]


FLAGS   = -Wall -pthread -D_REENTRANT -g
CC      = gcc
OFFLOAD_SIMULATOR  = offload_simulator
OBJ_OS  = main.o
MOBILE_NODE  = mobile_node
OBJ_MN  = mobile_node.o

all:	${OFFLOAD_SIMULATOR} ${MOBILE_NODE}

clean:
		rm ${OBJ_OS} ${OBJ_MN}

${OFFLOAD_SIMULATOR}:	${OBJ_OS}
		 ${CC} ${FLAGS} ${OBJ_OS} -o $@

${MOBILE_NODE}:	${OBJ_MN}
		${CC} ${FLAGS} ${OBJ_MN} -o $@

.c.o:
		${CC} ${FLAGS} $< -c


#########################

main.o:					main.c main.h maintenance_manager.h task_manager.h edge_server.h monitor.h
commands.o:				mobile_node.c