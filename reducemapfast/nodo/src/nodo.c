/*o
 ============================================================================
 Name        : nodo.c
 Author      : Los Barderos
 Version     : 1.0
 Copyright   : Los Barderos - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 1C 2015
 Testing	 :
 ============================================================================
 */

#include "nodo.h"

int main(int argv, char** argc) {
	//inicializamos los semaforos
	//sem_init(&semaforoAccesoMemoria, 0, 1);
	//sem_init(&semaforoMarcosLibres, 0, 0);

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	//g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	//g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	logger = log_create(NOMBRE_ARCHIVO_LOG, "nodo", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();
	crearArchivoBin();

	printf("Ok\n");
	fclose(archivoEspacioDatos);
	return 0;
}

void crearArchivoBin() {
	archivoEspacioDatos= fopen("data.bin", "wb");
	int i;
	grabarBloque();

}
void grabarBloque(){
	//char * memoria = g_BaseMemoria;
		int tamanio = TAMANIO_BLOQUE*1024*1024;
		//char* txtSwap =  malloc(tamanio);
		//memset(aux,'0', tamanio* sizeof(char));
		char*txtBloq = malloc(TAMANIO_BLOQUE*1024*1024);
		memset(txtBloq, '0', tamanio * sizeof(char));
		//memoria = memoria + ((nroMarco * TAMANIO_PAGINA));
		//while(tamanio--){
		//memcpy(txtSwap, memoria, tamanio);
		/*
		 char* aux =  malloc(tamanio);
		 memset(aux,'0', tamanio* sizeof(char));
		 memcpy(aux, texto, strlen(texto)+1);
		 char * memoria = g_BaseMemoria;
		 memoria = memoria + (nroMarco*TAMANIO_PAGINA) + posicion;
		 while(tamanio--){
		 *memoria++ = *(aux++); //VERIFICAR QUE ESTO FUNCIONE
		 printf("%c",*(memoria-1));
		 }
		 //NO HACE FREE DE AUX
		 return 1;
		 */

		//}

		fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
		free(txtBloq);
}

#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Preguntamos y obtenemos el puerto donde esta escuchando el filesystem
		if (config_has_property(config, "PUERTO_FS")) {
			g_Puerto_Fs = config_get_int_value(config, "PUERTO_FS");
		} else
			Error("No se pudo leer el parametro PUERTO_FS");

		// Preguntamos y obtenemos la ip donde esta ejecutandose el filesystem
		if (config_has_property(config, "IP_FS")) {
			g_Ip_Fs = config_get_string_value(config,"IP_FS");
		} else
			Error("No se pudo leer el parametro IP_FS");

		// Obtenemos el nombre del archivo con los bloques
		if (config_has_property(config, "ARCHIVO_BIN")) {
			g_Archivo_Bin = config_get_string_value(config, "ARCHIVO_BIN");
		} else
			Error("No se pudo leer el parametro ARCHIVO_BIN");

		// Obtenemos el tamanio del archivo con los bloques
				if (config_has_property(config, "ARCHIVO_BIN")) {
					g_Tamanio_Bin = config_get_string_value(config, "TAMANIO_BIN");
				} else
					Error("No se pudo leer el parametro TAMANIO_BIN");

		// Obtenemos el nombre del directorio temporal
		if (config_has_property(config, "DIR_TEMP")) {
			g_Dir_Temp = config_get_string_value(config, "DIR_TEMP");
		} else
			Error("No se pudo leer el parametro DIR_TEMP");

		// Obtenemos si es nodo nuevo
		if (config_has_property(config, "NODO_NUEVO")) {
			g_Nodo_Nuevo = config_get_string_value(config, "NODO_NUEVO");
		} else
			Error("No se pudo leer el parametro NODO_NUEVO");

		// Obtenemos la ip del nodo
		if (config_has_property(config, "IP_NODO")) {
			g_Ip_Nodo = config_get_string_value(config, "IP_NODO");
		} else
			Error("No se pudo leer el parametro IP_NODO");

		// Obtenemos el puerto de escucha del nodo
		if (config_has_property(config, "PUERTO_NODO")) {
			g_Puerto_Nodo = config_get_int_value(config, "PUERTO_NODO");
		} else
			Error("No se pudo leer el parametro PUERTO_NODO");

	} else {
		//ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif

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
