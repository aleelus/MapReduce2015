CCC:= gcc -c
CC := gcc
PN := ./nodo/
PJ := ./job/
PF := ./filesystem/
PM := ./marta/
 


#NODO

nodo: nodo.o
	$(CC) -o "$(PN)Debug/nodo" $(PN)Debug/src/nodo.o

nodo.o: ./nodo/src/nodo.h
	$(CCC) $(PN)src/nodo.c -o $(PN)Debug/src/nodo.o


#JOB

job: job.o
	$(CC) -o "$(PJ)Debug/job" $(PJ)Debug/src/job.o
	
job.o: ./job/src/job.h
	$(CCC) $(PJ)src/job.c -o $(PJ)Debug/src/job.o


#FILESYSTEM

filesystem: filesystem.o
	$(CC) -o "$(PF)Debug/filesystem" $(PF)Debug/src/filesystem.o

filesystem.o: ./filesystem/src/filesystem.h
	$(CCC) $(PF)src/filesystem.c -o $(PF)Debug/src/filesystem.o


#MARTA

marta: marta.o
	$(CC) -o "$(PM)Debug/marta" $(PM)Debug/src/marta.o

marta.o: ./marta/src/marta.h
	$(CCC) $(PM)src/marta.c -o $(PM)Debug/src/marta.o


#ALL

all: nodo job filesystem marta


#CLEAN

clean:
	rm $(PN)Debug/src/nodo.o
	rm $(PJ)Debug/src/job.o
	rm $(PF)Debug/src/filesystem.o
	rm $(PM)Debug/src/marta.o

 	
