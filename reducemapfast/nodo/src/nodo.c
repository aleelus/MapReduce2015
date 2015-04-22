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
	printf("el tamanio es %d \n",g_Tamanio_Bin);
	printf("el archivo es %s \n",g_Archivo_Bin);
	crearArchivoBin();
	mapeo();
	printf("Ok\n");
	fclose(archivoEspacioDatos);
	return 0;
}

void crearArchivoBin() {
	archivoEspacioDatos= fopen(g_Archivo_Bin, "wb");
	if(archivoEspacioDatos == NULL)
		Error("No se pudo abrir el archivo");
	int i=g_Tamanio_Bin;
	while(i>20){
		grabarBloque();
		i=i-20;
	}
	fclose(archivoEspacioDatos);
}
void grabarBloque(){
		int tamanio = TAMANIO_BLOQUE; //*1024*1024; //Tamanio del bloque 20mb
		char*txtBloq = malloc(TAMANIO_BLOQUE);//*1024*1024);
		memset(txtBloq, '0', tamanio * sizeof(char));
		//Rellena de 0 el txtBloq que se va a grabar
		fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
		//Grabo en el archivo el bloque
		free(txtBloq);
		//Libero el puntero
}

int tamanio_archivo(int fd){
	struct stat buf;
	fstat(fd, &buf);
	return buf.st_size;
}

void mapeo(){

	char* mapeo;
	//int tamanio;

	//char* nombre_archivo = "arch.dat";

	if(( archivoEspacioDatos = fopen (g_Archivo_Bin, "rb") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", g_Archivo_Bin, strerror(errno));
		abort();
	}
	int mapper= fileno(archivoEspacioDatos);
	//tamanio = tamanio_archivo(mapper);
	if( (mapeo = mmap( NULL, TAMANIO_BLOQUE, PROT_READ, MAP_SHARED, mapper, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
		fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
		abort();
	}
	printf ("Tamaño leido: %d\nContenido:'%s'\n", TAMANIO_BLOQUE, mapeo);

	//Seamos prolijos
	munmap( mapeo, TAMANIO_BLOQUE );
	//fclose(mapper);
	//Libero lo mapeado y no cierro el archivo aca xq lo cierro en el main.
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
					g_Tamanio_Bin = config_get_int_value(config, "TAMANIO_BIN");
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
