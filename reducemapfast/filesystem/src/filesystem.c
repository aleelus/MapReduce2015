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
#include "consola.h"



int main(int argv, char** argc) {

	/* Abrir Archivo de Configuracion */
	if ( (config = fopen(argv[1],"rb")) == NULL ) {
		mostrarError(NoSePudoAbrirConfig);
		return EXIT_FAILURE;
		}

	/* Obtener valores de configuracion */
	while(fgets(linea, MAXLINEA, config) != NULL){
		if(sscanf(linea, "%*[^\n#]%c", &ch) == 1)
			;	//Se ignoran las lineas en blanco y comentarios

		if(sscanf(linea, " PUERTO_LISTEN %u = %[^\n]", &i, &puerto_listen) != 1){
			mostrarError(NoSePuedeObtenerPuerto);
			return EXIT_FAILURE;
		} else {
			// Se obtiene el puerto
			configuracion.puerto_listen = malloc(strlen(puerto_listen) +1);
			strcpy(configuracion.puerto_listen, puerto_listen);
		}
		if(sscanf(linea, " LISTA_NODOS %u = %[^\n]", &lista_nodos) != 1){
			mostrarError(NoSePuedeObtenerNodos);
			return EXIT_FAILURE;
		} else {
			// Se obtienen los nodos
			configuracion.lista_nodos = malloc(strlen(lista_nodos) +1);
			strcpy(configuracion.lista_nodos);
		}

		}

	printf("Puerto: \n", configuracion.puerto_listen);
	printf("Nodos: \n", configuracion.lista_nodos);

	}



	//inicializamos los semaforos
	//sem_init(&semaforoAccesoMemoria, 0, 1);
	//sem_init(&semaforoMarcosLibres, 0, 0);

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	//g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	//g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	//logger = log_create(NOMBRE_ARCHIVO_LOG, "msp", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	//LevantarConfig();
	//printf("Ok\n");
	//return 0;


#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {



	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
//	if (config->properties->table_current_size != 0) {
//
		// Puerto de escucha
//		if (config_has_property(config, "PUERTO_LISTEN")) {
//			g_Puerto_Listen = config_get_int_value(config, "PUERTO_LISTEN");
//		} else
//			Error("No se pudo leer el parametro PUERTO_LISTEN");

		// Lista de nodos minima que deben estar conectados para que funcione
//		if (config_has_property(config, "LISTA_NODOS")) {
//			g_Lista_Nodos = config_get_string_value(config,"LISTA_NODOS");
//		} else
//			Error("No se pudo leer el parametro LISTA_NODOS");
//
//	} else {
//		//ErrorFatal("No se pudo abrir el archivo de configuracion");
//	}
//	if (config != NULL ) {
//		free(config);
//	}
}

#endif

/*#if 1 // METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);

	fprintf(stderr, "\nERROR: %s\n", nuevo);
	log_error(logger, "%s", nuevo);

	va_end(arguments);
	if (nuevo != NULL )
		free(nuevo);
}
#endif*/
