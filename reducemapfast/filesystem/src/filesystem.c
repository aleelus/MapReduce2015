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

int main(int argv, char** argc) {

	// Abrir Archivo de Configuracion
	leer_config();

	// Conectar a Marta
	conectar_marta();

	// Abrir conexiones de nodos
	conectar_nodos();

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
//	LevantarConfig();
//	printf("Ok\n");
//	return 0;



//#if 1 // METODOS CONFIGURACION //


//#endif

/*
#if 1 // METODOS MANEJO DE ERRORES //
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
#endif
*/



