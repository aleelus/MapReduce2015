/*
 ============================================================================
 Name        : filesystem.c
 Author      : Los Barderos
 Version     : 1.0
 Copyright   : Los Barderos - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 1C 2015
 Testing	 :
 ============================================================================
 */

#include "filesystem.h"
#include "mongodev.h"



int main(int argv, char** argc) {
	mongo_db_open();
	levantarConfig();
	nroBloque=0;
	indexGlobal = 1;
	letra ='A';
	sem_init(&semLNodos,0,1);
	sem_init(&semHilos,0,1);
	sem_init(&semArchivos,0,1);
	sem_init(&semTamanioTotal,0,1);
	nombre = string_new();				//Variable del nombre de los nodos

	lista_nodos    = list_create();		//Lista de nodos
	lista_archivos = list_create();		//Lista de archivos
	lista_filesystem = list_create();	//Lista de estructura de filesystem
	cargar_listas_mongo();




	printf("\nLO HIZO!!!\n");


	int iThreadConsola;					//Hilo de consola
	int iThreadOrquestador;				//Hilo orquestador

	//Archivo de Log
	logger = log_create(NOMBRE_ARCHIVO_LOG, "fs", true, LOG_LEVEL_TRACE);

	//iniciarMongo();
	//leerMongo();
	//eliminarMongo();
//	leerJSON(SonArchivos);
//	leerJSON(SonDirectorios);
//	leerArchivoMongo();
//	eliminarArchivoMongo();

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	g_MensajeError   = malloc(1 * sizeof(char));

	//Este hilo es el que maneja la consola
	if ((iThreadConsola = pthread_create(&hConsola, NULL, (void*) Comenzar_Consola, NULL)) != 0){
		fprintf(stderr, (char *)NosePuedeCrearHilo, iThreadConsola);
		exit(EXIT_FAILURE);
	};

	//Hilo orquestador conexiones para escuchar a Marta o a Nodos
	if ((iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL, (void*) HiloOrquestadorDeConexiones, NULL )) != 0){
		fprintf(stderr, (char *)NosePuedeCrearHilo, iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );
	mongo_db_close();


	return EXIT_SUCCESS; 	

}


