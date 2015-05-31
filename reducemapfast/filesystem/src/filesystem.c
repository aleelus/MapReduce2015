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

	leer_config();						// Abrir Archivo de Configuracion

	nombre = string_new();				//Variable del nombre de los nodos

	lista_nodos    = list_create();		//Lista de nodos
	lista_archivos = list_create();		//Lista de archivos
	int iThreadConsola;					//Hilo de consola
	int iThreadOrquestador;				//Hilo orquestador

	// Abrir conexiones de nodos
	//conectar_nodos();

	//if (configuracion.cantidadNodos == 4){ 		//cantidad minima de nodos a conectarse

		//Iniciar Mongo
		//iniciar_mongo();

		// Conectar a Marta
		//conectar_marta();		//deberia ir un mutex.. creo..

		// Iniciar consola
		//while(eleccion != 10){
			// Solicitar opcion de comando
			//printf("Ejecute una opcion: ");

	//	if (fgets(input, CANTMAX, stdin) == 0)
	//		printf("Error al seleccionar comando\n");
		//	scanf("%d", &eleccion);
		//	ejecutarComando(eleccion);
		//}


	//}



	//Archivo de Log
	logger = log_create(NOMBRE_ARCHIVO_LOG, "fs", true, LOG_LEVEL_TRACE);

	//iniciarMongo();
	leerMongo();
	//eliminarMongo();

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

	return EXIT_SUCCESS; 	

}


