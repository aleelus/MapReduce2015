CCC:= gcc -c
CC := gcc
PN := ./nodo/
PJ := ./job/
PF := ./filesystem/
PM := ./marta/
SC := src/
DBS:= Debug/src/
DB := Debug/
 


#NODO

nodo: nodo.o
	$(CC) -o "$(PN)$(DB)nodo" $(PN)$(DBS)nodo.o

nodo.o: ./nodo/src/nodo.h
	$(CCC) $(PN)$(SC)nodo.c -o $(PN)$(DBS)nodo.o



#JOB

job: job.o
	$(CC) -o "$(PJ)$(DB)job" $(PJ)$(DBS)job.o
	
job.o: ./job/src/job.h
	$(CCC) $(PJ)$(SC)job.c -o $(PJ)$(DBS)job.o


#FILESYSTEM

filesystem: filesystem.o
	$(CC) -o "$(PF)$(DB)filesystem" $(PF)$(DBS)filesystem.o

filesystem.o: ./filesystem/src/filesystem.h
	$(CCC) $(PF)$(SC)filesystem.c -o $(PF)$(DBS)filesystem.o


#MARTA

marta: marta.o
	$(CC) -o "$(PM)$(DB)marta" $(PM)$(DBS)marta.o

marta.o: ./marta/src/marta.h
	$(CCC) $(PM)$(SC)marta.c -o $(PM)$(DBS)marta.o


#ALL

all: nodo job filesystem marta


#CLEAN

clean:
	rm $(PN)$(DBS)nodo.o
	rm $(PJ)$(DBS)job.o
	rm $(PF)$(DBS)filesystem.o
	rm $(PM)$(DBS)marta.o

 	
