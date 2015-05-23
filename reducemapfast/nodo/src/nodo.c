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
int socket_fsystem;


// Crear el archivo de espacio de datos datos.bin con "truncate -s 1G datos.bin"
int main(int argv, char** argc) {
	pagina = sysconf(_SC_PAGE_SIZE);
	logger = log_create(NOMBRE_ARCHIVO_LOG, "nodo", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
		LevantarConfig();

	//Abro el archivo de espacio de datos donde voy a leer y escribir bloques
	if(( archivoEspacioDatos = fopen(g_Archivo_Bin, "r+b") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", g_Archivo_Bin, strerror(errno));
		abort();
			}

	//Prueba getFileContent
	//char* temp_file = getFileContent("temporalPrueba.tmp");


	//Socket para escuchar al Job, FS y otros Nodos
	//Hilo orquestador conexiones
	int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
			(void*) HiloOrquestadorDeConexiones, NULL );
	if (iThreadOrquestador) {
		fprintf(stderr,
			"Error al crear hilo - pthread_create() return code: %d\n",
			iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	pthread_join(hOrquestadorConexiones, NULL );

	//Prueba setBloque
	printf("Escriba el numero de bloque que quiere escribir.\n");
	int numero;
	scanf("%d", &numero);
	printf("Escriba lo que quiere grabar en el bloque.\n");
	char* datos = "Este es el bloque 2";
	setBloque(numero,datos);

	//Prueba getBloque
	printf("Escriba el numero de bloque que quiere ver.\n");
	scanf("%d", &numero);
	char * bloqueSolicitado;
		bloqueSolicitado = getBloque(numero);
		printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloqueSolicitado);
		munmap( bloqueSolicitado, TAMANIO_BLOQUE ); //Desmapeo el bloque obternido, IMPORTANTE!

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

char* getBloque(int numero){
	char* bloque;
	int fd= fileno(archivoEspacioDatos);
	int offset = numero*(TAMANIO_BLOQUE/pagina);
	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ, MAP_SHARED, fd, offset*pagina)) == MAP_FAILED){
			//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
			fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
			abort();
		}
	printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);

	return bloque;
}
/* char* getBloque(int numero){
	char* bloque = malloc(TAMANIO_BLOQUE);
	long int offset=numero*TAMANIO_BLOQUE;
	fseek(archivoEspacioDatos, offset, SEEK_SET);
	fread(bloque, sizeof(char), TAMANIO_BLOQUE, archivoEspacioDatos);
	return bloque;
}*/


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
	static char ruta[] = "/tmp";
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

int enviarDatos(int socket, void *buffer) {
	int bytecount;

	if ((bytecount = send(socket, buffer, BUFFERSIZE, 0)) == -1) {
		printf("ERROR: no se pudo enviar información. \n");
		log_info(logger, "ERROR: no se pudo enviar información. \n");
	}
	log_info(logger, "ENVIO datos. socket: %d. buffer: %s", socket,
			(char*) buffer);

	return (bytecount);
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
	log_info(logger, "Archivo de configuración: %s", PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Preguntamos y obtenemos el puerto donde esta escuchando el filesystem
		if (config_has_property(config, "PUERTO_FS")) {
			g_Puerto_Fs = config_get_int_value(config, "PUERTO_FS");
		} else {
			Error("No se pudo leer el parametro PUERTO_FS");
		}
		// Preguntamos y obtenemos la ip donde esta ejecutandose el filesystem
		if (config_has_property(config, "IP_FS")) {
			g_Ip_Fs = config_get_string_value(config,"IP_FS");
		} else{
			Error("No se pudo leer el parametro IP_FS");
		}


		// Obtenemos el nombre del archivo con los bloques
		if (config_has_property(config, "ARCHIVO_BIN")) {
			g_Archivo_Bin = config_get_string_value(config, "ARCHIVO_BIN");
		} else{
			Error("No se pudo leer el parametro ARCHIVO_BIN");
		}

		// Obtenemos el nombre del directorio temporal
		if (config_has_property(config, "DIR_TEMP")) {
			g_Dir_Temp = config_get_string_value(config, "DIR_TEMP");
		} else{
			Error("No se pudo leer el parametro DIR_TEMP");
		}

		// Obtenemos si es nodo nuevo
		if (config_has_property(config, "NODO_NUEVO")) {
			g_Nodo_Nuevo = config_get_string_value(config, "NODO_NUEVO");
		} else{
			Error("No se pudo leer el parametro NODO_NUEVO");
		}

		// Obtenemos la ip del nodo
		if (config_has_property(config, "IP_NODO")) {
			g_Ip_Nodo = config_get_string_value(config, "IP_NODO");
		} else{
			Error("No se pudo leer el parametro IP_NODO");
		}

		// Obtenemos el puerto de escucha del nodo
		if (config_has_property(config, "PUERTO_NODO")) {
			g_Puerto_Nodo = config_get_int_value(config, "PUERTO_NODO");
		} else{
			Error("No se pudo leer el parametro PUERTO_NODO");
		}

	} else {
		Error("No se pudo abrir el archivo de configuracion");
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

void SetearErrorGlobal(const char* mensaje, ...) {
	va_list arguments;
	va_start(arguments, mensaje);
	if (g_MensajeError != NULL )
		// NMR COMENTADO POR ERROR A ULTIMO MOMENTO	free(g_MensajeError);
		g_MensajeError = string_from_vformat(mensaje, arguments);
	va_end(arguments);
}

void ErrorFatal(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);
	printf("\nERROR FATAL--> %s \n", nuevo);
	log_error(logger, "\nERROR FATAL--> %s \n", nuevo);
	char fin;

	printf(
			"El programa se cerrara. Presione ENTER para finalizar la ejecución.");
	fin = scanf("%c", &fin);

	va_end(arguments);
	if (nuevo != NULL )
		free(nuevo);
	exit(EXIT_FAILURE);
}

int ChartToInt(char x) {
	int numero = 0;
	char * aux = string_new();
	string_append_with_format(&aux, "%c", x);
	//char* aux = malloc(1 * sizeof(char));
	//sprintf(aux, "%c", x);
	numero = strtol(aux, (char **) NULL, 10);

	if (aux != NULL )
		free(aux);
	return numero;
}

int PosicionDeBufferAInt(char* buffer, int posicion) {
	int logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return ChartToInt(buffer[posicion]);
}

int ObtenerTamanio (char *buffer , int posicion, int dig_tamanio){
	int x,digito,aux=0;
	for(x=0;x<dig_tamanio;x++){
		digito=PosicionDeBufferAInt(buffer,posicion+x);
		aux=aux*10+digito;
	}
	return aux;
}

char* RecibirDatos(int socket, char *buffer, int *bytesRecibidos,int *cantRafaga,int *tamanio) {
	*bytesRecibidos = 0;
	char *bufferAux= malloc(1);
	int digTamanio;
	if (buffer != NULL ) {
		free(buffer);
	}

	if(*cantRafaga==1){
		bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char));
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}

		digTamanio=PosicionDeBufferAInt(bufferAux,1);
		*tamanio=ObtenerTamanio(bufferAux,2,digTamanio);


	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char));
		memset(bufferAux, 0, *tamanio * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket,
			(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}

int EnviarDatos(int socket, char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a al clientes. Socket: %d", socket);

	//Traza("ENVIO datos. socket: %d. buffer: %s", socket, (char*) buffer);

	//char * bufferLogueo = malloc(5);
	//bufferLogueo[cantidadDeBytesAEnviar] = '\0';

	//memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			(char*) buffer);

	return bytecount;
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return PosicionDeBufferAInt(buffer, 0);
}

void implementoJob(int *id,char * buffer,int * cantRafaga,char ** mensaje){



	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	printf("RAFAGA:%d\n",tipo_mensaje);
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case RECIBIR_ARCHIVO:
			//AtiendeJob(id,buffer,cantRafaga);
			//*cantRafaga=0;
			//ObtenerInfoDeNodos(*id);
			//Planificar(*id);
			//EnviarPlanificacionAJob(id);
			*cantRafaga=1;
			break;
		case NOTIFICACION_NODO:
			break;
		case RECIBIDO_OK:
			break;
		default:
			break;
		}
		*mensaje = "Ok";
	} else {
		if (*cantRafaga==1) {
			*mensaje = "Ok";
			*cantRafaga = 2;
		} else {
			*mensaje = "No";
		}
	}
}



int AtiendeCliente(void * arg) {
	int socket = (int) arg;
	int id=-1;
	//cantHilos++;
	//printf("Hilo numero:%d\n",cantHilos);

//Es el ID del programa con el que está trabajando actualmente el HILO.
//Nos es de gran utilidad para controlar los permisos de acceso (lectura/escritura) del programa.
//(en otras palabras que no se pase de vivo y quiera acceder a una posicion de memoria que no le corresponde.)
//	int id_Programa = 0;
//	int tipo_Cliente = 0;
	int longitudBuffer;
	//printf("ENTRE");

// Es el encabezado del mensaje. Nos dice quien envia el mensaje
	int emisor = 0;

// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(BUFFERSIZE * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

// Cantidad de bytes recibidos.
	int bytesRecibidos;

// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

// Código de salida por defecto
	int code = 0;
	int cantRafaga=1,tamanio=0;
	char * mensaje;
	while ((!desconexionCliente) & g_Ejecutando) {
		//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();

		//Recibimos los datos del cliente
		buffer = RecibirDatos(socket, buffer, &bytesRecibidos,&cantRafaga,&tamanio);


		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_JOB:
				implementoJob(&id,buffer,&cantRafaga,&mensaje);
				//BUFFER RECIBIDO = 23100 (EJEMPLO PRIMERA RAFAGA)
				//BUFFER ENVIADO = Ok
				//BUFFER RECIBIDO = 2
				//BUFFER ENVIADO = 311

				break;
			case ES_FS:
				printf("implementar atiendeFS\n");
				//implementoFS(buffer,&cantRafaga,&mensaje);
				//Esto va en nodo.h y son define
				//PRIMERA_CONEXION 1
				//GET_BLOQUE 2
				//SET_BLOQUE 3
				//GET_FILE_CONTENT 4

				//BUFFER RECIBIDO = 11 --- 1:soy FS 1:estas disponible para conectarte?
				//BUFFER ENVIADO = 311250 --- 3: soy nodo 1: estoy vivo 1: soy nuevo 2: cant de dig de cant de
				// 							   de bloques 50: cantidad de bloques

				//SET_BLOQUE
				//BUFFER RECIBIDO = 13820971520  --- 1: soy FS 3: quiero un set bloque 8: cant de dig de tamanio
				//								    de lo que necesitamos que grabe 20971520: tamaño en bytes
				//BUFFER ENVIADO = Ok

				//GET_BLOQUE
				//BUFFER RECIBIDO = 12210 1: soy FS 2: quiero un get bloque 3:cant de dig de numero de bloque
				//						  10: numero de bloque que necesito recibir
				//BUFFER ENVIADO = 32820971520 3: soy nodo 2: va un get bloque 8: cant de dig de tamanio de
				//								bloque solicitado 20971520: tamanio de bloque

				//GET_FILE_CONTENT
				//BUFFER RECIBIDO = 14213resultado.txt 1: soy FS 4: solicito un get file content 2: cant de dig
				//					de tamanio de archivo solicitado 13:tamanio de archivo y luego el archivo
				//BUFFER ENVIADO = 34820971520 3: soy nodo 4: va un get file content solicitado 8: cant de dig
				//							   de tamanio de arch de result 20971520: tamanio de archivo

				break;
			case ES_NODO:
				printf("implementar atiendeNodo\n");
				//RecorrerArchivos();
				mensaje = "Ok";
				break;
			case COMANDOBLOQUES:
				printf("Despues vemos que hace esto\n");
				//RecorrerListaBloques();
				mensaje = "Ok";
				break;
			default:
				break;
			}
			longitudBuffer=strlen(mensaje);
			//printf("\nRespuesta: %s\n",buffer);
			// Enviamos datos al cliente.
			EnviarDatos(socket, mensaje,longitudBuffer);
		} else
			desconexionCliente = 1;

	}

	CerrarSocket(socket);

	return code;
}

void HiloOrquestadorDeConexiones() {

	int socket_host;
	struct sockaddr_in client_addr;
	struct sockaddr_in my_addr;
	int yes = 1;
	socklen_t size_addr = 0;

	socket_host = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_host == -1)
		ErrorFatal(
				"No se pudo inicializar el socket que escucha a los clientes");

	if (setsockopt(socket_host, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		ErrorFatal("Error al hacer el 'setsockopt'");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(g_Puerto_Nodo);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY );
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		ErrorFatal("Error al hacer el Bind. El puerto está en uso");

	if (listen(socket_host, 10) == -1) // el "10" es el tamaño de la cola de conexiones.
		ErrorFatal(
				"Error al hacer el Listen. No se pudo escuchar en el puerto especificado");

	//Traza("El socket está listo para recibir conexiones. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);
	log_trace(logger,
			"SOCKET LISTO PARA RECBIR CONEXIONES. Numero de socket: %d, puerto: %d",
			socket_host, g_Puerto_Nodo);

	while (g_Ejecutando) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,(struct sockaddr *) &client_addr, &size_addr)) != -1) {
			//Traza("Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);
			log_trace(logger,
					"NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d",
					inet_ntoa(client_addr.sin_addr), client_addr.sin_port,
					socket_client);
			// Aca hay que crear un nuevo hilo, que será el encargado de atender al cliente
			pthread_t hNuevoCliente;
			pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente,
					(void *) socket_client);
		} else {
			Error("ERROR AL ACEPTAR LA CONEXIÓN DE UN CLIENTE");
		}
	}
	CerrarSocket(socket_host);
}
