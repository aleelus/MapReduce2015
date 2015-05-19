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
// Crear el archivo de espacio de datos datos.bin con "truncate -s 1G datos.bin"
int main(int argv, char** argc) {
	//inicializamos los semaforos
	//sem_init(&semaforoAccesoMemoria, 0, 1);
	//sem_init(&semaforoMarcosLibres, 0, 0);
	int pagina = sysconf(_SC_PAGE_SIZE);
	printf("el tamanio de pag es %d",pagina);

	//char* temp_file = tmpnam(NULL);
	char* temp_file = getFileContent("temporalPrueba.tmp");
	logger = log_create(NOMBRE_ARCHIVO_LOG, "nodo", true, LOG_LEVEL_TRACE);
	// Levantamos el archivo de configuracion.
	LevantarConfig();
	printf("el archivo es %s \n",g_Archivo_Bin);
	if(( archivoEspacioDatos = fopen(g_Archivo_Bin, "r+b") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", g_Archivo_Bin, strerror(errno));
		abort();
	}
	//mapeo();
	printf("Escriba el numero de bloque que quiere escribir.\n");
	int numero;
	scanf("%d", &numero);
	printf("Escriba lo que quiere grabar en el bloque.\n");
	char* datos = "Este es el bloque 2";
	setBloque(numero,datos);
	printf("Escriba el numero de bloque que quiere ver.\n");
	scanf("%d", &numero);
	char * bloqueSolicitado;
		bloqueSolicitado = getBloque(numero);
		printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloqueSolicitado);
		free(bloqueSolicitado);
		//munmap( bloqueSolicitado, TAMANIO_BLOQUE );


	printf("Ok\n");
	fclose(archivoEspacioDatos);
	return 0;
}


void grabarBloque(){
		int tamanio = TAMANIO_BLOQUE; //*1024*1024; //Tamanio del bloque 20mb
		char*txtBloq = malloc(tamanio);//*1024*1024);
		memset(txtBloq, '0', tamanio * sizeof(char));
		//Rellena de 0 el txtBloq que se va a grabar
		fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
		//Grabo en el archivo el bloque
		free(txtBloq);
		//Libero el puntero
}

int tamanio_archivo(char* nomArch){
	struct stat buf;
	stat(nomArch, &buf);
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

/* char* getBloque(int numero){
	char* bloque;
	int fd= fileno(archivoEspacioDatos);
	int offset = numero*5120;
	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ, MAP_SHARED, fd, offset)) == MAP_FAILED){
			//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
			fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
			abort();
		}
	printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);

	return bloque;
}*/
char* getBloque(int numero){
	char* bloque = malloc(TAMANIO_BLOQUE);
	long int offset=numero*TAMANIO_BLOQUE;
	fseek(archivoEspacioDatos, offset, SEEK_SET);
	fread(bloque, sizeof(char), TAMANIO_BLOQUE, archivoEspacioDatos);
	return bloque;
}


void setBloque(int numero, char*datos){
	if(( tamanio_archivo(g_Archivo_Bin) <= (numero*TAMANIO_BLOQUE) )){
			//Si no se pudo abrir, imprimir el error y abortar;
			printf("El bloque no existe en el archivo. \n");
			abort();
		}
	long int tamanio = TAMANIO_BLOQUE; //Tamanio del bloque 20mb
	char*txtBloq = malloc(TAMANIO_BLOQUE);
	memset(txtBloq, '0', tamanio * sizeof(char));
	//Rellena de 0 el txtBloq que se va a grabar
	memcpy(txtBloq, datos, strlen(datos)+1);
	//Copia los datos a grabar en el bloque auxiliar
	long int offset=numero*TAMANIO_BLOQUE;

	fseek(archivoEspacioDatos, offset, SEEK_SET);
	//Posicion el puntero en el bloque pedido
	fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
	//Grabo en el archivo el bloque
	free(txtBloq);
	//Libero el puntero


}


char * getFileContent(char* nombre){
	FILE* archivoTemporal;
	static char ruta[] = "DIR_TEMP";
	char* nombreT = string_new();
	string_append(&nombreT,ruta);
	string_append(&nombreT,"/");
	string_append(&nombreT,nombre);
	char fname[PATH_MAX];
	strcpy(fname,nombreT);
	printf("%s \n",fname);
	char* contenido= malloc(tamanio_archivo(fname));
	//Busca el archivo temporal nombre y devuelve su contenido
	if(( archivoTemporal = fopen(fname, "r+b") ) == NULL){
			//Si no se pudo abrir, imprimir el error y abortar;
			fprintf(stderr, "Error al abrir el archivo '%s': %s\n", fname, strerror(errno));
			abort();
		}
	fread(contenido, sizeof(char),tamanio_archivo(fname),archivoTemporal);
	printf("Filename is %s\n", fname);
	printf("Received string: %s", contenido);
	return contenido;
}

int pipesPrueba(void)
{
        int fd[2], nbytes, temp;
        pid_t   childpid;
        char*    string = "Hello, world!\nBomba la loca \nAloha manola\n";
        char    readbuffer[80];
        static char template[] = "/tmp/mytemporalXXXXXX";
        char fname[PATH_MAX];

        pipe(fd);

        if((childpid = fork()) == -1)
        {
                perror("fork");
                exit(1);
        }

        if(childpid == 0)
        {
                /* Proceso hijo cierra la entrada del pipe */
                close(fd[0]);

                /* Manda 'string' por la salida del pipe*/
                write(fd[1], string, (strlen(string)+1));



                close(fd);				/* Cierro el temporal */
        }
        else
        {
                /* Proceso padre cierra la salida del pipe */
                close(fd[1]);
                /* Leo un string por el pipe */
                nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
                /* Creo un archivo temporal a donde voy a hacer sort*/
                strcpy(fname,template);
                temp =  mkstemp(fname);
                write(temp, string, (strlen(string)+1));
                printf("Filename is %s\n", fname);
                printf("Received string: %s", readbuffer);
                /* Ejecuto sort sobre el archivo temporal*/
                execlp("sort","sort",fname,NULL);
        }
        return(0);
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
