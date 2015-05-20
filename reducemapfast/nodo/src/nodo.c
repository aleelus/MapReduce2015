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

	socket_fsystem = conectarFS();

	//Prueba getFileContent
	char* temp_file = getFileContent("temporalPrueba.tmp");

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

int conectarFS() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON MSP
	log_info(logger, "Intentando conectar a file system\n");

	//conectar con memoria
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if (getaddrinfo(g_Ip_Fs, g_Puerto_Fs, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_filesystem");
	}

	int socket_filesystem;
	if ((socket_filesystem = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_filesystem");
	}
	if (connect(socket_filesystem, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_filesystem");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return (socket_filesystem);
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

		if ((socket_client = accept(socket_host,
				(struct sockaddr *) &client_addr, &size_addr)) != -1) {
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


void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
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

int recibirDatos(int socket, char *buffer) {
	int bytecount;
	// memset se usa para llenar el buffer con 0s
	memset(buffer, 0, BUFFERSIZE);

	//Nos ponemos a la escucha de las peticiones que nos envie el file system
	//aca si recibo 0 bytes es que se desconecto el otro, cerrar el hilo.
	if ((bytecount = recv(socket, buffer, BUFFERSIZE, 0)) == -1)//1 pid del file system,
		log_info(logger, "ERROR: error al intentar recibir datos");

	log_info(logger, "RECIBO datos. socket: %d. buffer: %s\n", socket,
			(char*) buffer);
	return (bytecount);
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;

//Es el ID del programa con el que está trabajando actualmente el HILO.
//Nos es de gran utilidad para controlar los permisos de acceso (lectura/escritura) del programa.
//(en otras palabras que no se pase de vivo y quiera acceder a una posicion de memoria que no le corresponde.)
	int id_Programa = 0;
	int tipo_Cliente = 0;
	int longitudBuffer;

// Es el encabezado del mensaje. Nos dice que acción se le está solicitando a la msp
	int tipo_mensaje = 0;

// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(1000000 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

// Cantidad de bytes recibidos.
	int bytesRecibidos;

// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

// Código de salida por defecto
	int code = 0;

	while ((!desconexionCliente) & g_Ejecutando) {

		if (buffer != NULL )
			free(buffer);
		buffer = string_new();
		//Recibimos los datos del cliente
		buffer = RecibirDatos(socket, buffer, &bytesRecibidos);

		if (bytesRecibidos > 0) {
			//Analizamos que peticion nos está haciendo (obtenemos el comando)
			tipo_mensaje = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (tipo_mensaje) {

			case MSJ_LEER_BLOQUE:
				buffer = ComandoLeerBloque(buffer, &id_Programa, tipo_Cliente, &longitudBuffer);
				break;
			case MSJ_ESCRIBIR_BLOQUE:
				buffer = ComandoEscribirBLoque(buffer, &id_Programa,
						tipo_Cliente, socket,&longitudBuffer);
				break;
			case MSJ_GET_TEMP:
				buffer = ComandoObtenerTemporal(buffer, &id_Programa, tipo_Cliente, &longitudBuffer);
				break;
			default:
				buffer = RespuestaClienteError(buffer,
						"El ingresado no es un comando válido\n");
				longitudBuffer=strlen(buffer);
				break;
			}
			printf("\nRespuesta: %s\n",buffer);
			// Enviamos datos al cliente.
			EnviarDatos(socket, buffer,longitudBuffer);
		} else
			desconexionCliente = 1;

	}

	CerrarSocket(socket);

	return code;
}

char* ComandoLeerBloque(char *buffer, int *idProg, int tipoCliente, int *longitud){
// Lee la memoria
// Formato del mensaje: CABBBB...
// C = Codigo de mensaje ( = 1)
// A = Cantidad de digitos que tiene el bloque
// BBBB = Bloque del nodo (hasta 9999)


// Retorna: Lo solicitado en la lectura
//			0 + mensaje error si no se pudo leer

	int ok = 0;

	int desplazamiento = 0;
	int longitudBuffer = 0;
	int offset = 0;

	int cantidadDigitosBloque = 0;
	int cantidadDigitosDesplazamiento = 0;
	int cantidadDigitoslongitudBuffer = 0;
	int posicion = 1;
	// Me fijo cuantos digitos tiene el numero de Bloque

	cantidadDigitosBloque = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidad Digitos del Bloque:%d\n", cantidadDigitosBloque);
	// Grabo el Numero del Bloque
	posicion++;

	*idProg = subCadenaAInt(buffer, posicion, cantidadDigitosBloque);
	printf("Bloque:%d\n", *idProg);
	log_trace(logger, "COMANDO Leer Bloque. Num Bloque: %d", *idProg);

	char* lectura = malloc(TAMANIO_BLOQUE);

	printf(
			"PARAMETROS: Bloque:%d\n",
			*idProg);
	lectura = getBloque(*idProg);
	//printf("\nESTE ES EL POSTA POSTA:");

	//lectura[longitudBuffer] = '\0';

	//printf("LECTURA:%s \n", lectura);

	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(TAMANIO_BLOQUE);
		memcpy(buffer,lectura,TAMANIO_BLOQUE);
		/*int tamanio = (longitudBuffer + 1) * sizeof(char);
		 buffer = realloc(buffer, tamanio * sizeof(char));
		 memset(buffer, 0, tamanio * sizeof(char));
		 sprintf(buffer, "%s%s", "1", lectura);*/
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal(
				"ERROR LEER BLOQUE. %s. Id: %d",stringErrorAux, idProg);
		if (buffer != NULL )
					free(buffer);
		buffer = malloc(3);
		string_append(&buffer, "-1");
		*longitud= strlen(buffer);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}

	if (lectura != NULL )
		free(lectura);
	return buffer;
}

char* ComandoEscribirBloque(char *buffer, int *idBloq, int tipoCliente,
		int socket, int *longitud) {
	// Graba en la memoria
	// Formato del mensaje: CABBBBOOOOOOOOO....
	// C = Codigo de mensaje ( = 2)
	// A = Cantidad de digitos que tiene el bloque
	// BBBB = Bloque del nodo (hasta 9999)
	// OOOOOOOOO = Datos a escribir

	// Retorna: 1 + Bytes si se leyo ok
	//			0 + mensaje error si no se pudo leer

	int ok = 0;
	int bytesRecibidos;

	int longitudBuffer = 0;

	int cantidadDigitosBloque = 0;
	int posicion = 1;
	// Me fijo cuantos digitos tiene el numero de bloque
	cantidadDigitosBloque = posicionDeBufferAInt(buffer, posicion);
	// Grabo el Num del Bloque
	posicion++;

	*idBloq = subCadenaAInt(buffer, posicion, cantidadDigitosBloque);

	log_trace(logger, "COMANDO Escribir Bloque. Id Bloque: %d", *idBloq);

	buffer = RecibirDatos(socket, buffer, &bytesRecibidos);

	char * aux = buffer;
	char* escritura = malloc(TAMANIO_BLOQUE);
	memcpy(escritura, aux,TAMANIO_BLOQUE);
	ok = escribirMemoria(*idBloq, escritura);

	printf("Memoria Escrita: %d\n", longitudBuffer);
	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(longitudBuffer+1);
		memcpy(buffer,escritura,longitudBuffer+1);
		//buffer = string_new();
		//string_append(&buffer, escritura);

		/*int tamanio = (longitudBuffer + 1) * sizeof(char);
		 buffer = realloc(buffer, tamanio * sizeof(char));
		 memset(buffer, 0, tamanio * sizeof(char));
		 sprintf(buffer, "%s%s", "1", lectura);*/
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal("ERROR GRABAR MEMORIA. %s. Id Bloque: %d",stringErrorAux, idBloq);

		if (buffer != NULL )
			free(buffer);
		buffer = malloc(2);
		string_append(&buffer, string_itoa(-1));
		*longitud= strlen(buffer);
		printf("LA LONGITUD DE BUFFER: %i\n",longitud);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}

	if (escritura != NULL )
		free(escritura);
	return buffer;
}

char* ComandoObtenerTemporal(char *buffer, int *idBloq, int tipoCliente,
		int socket, int *longitud) {
// Obtiene y devuelve el contenido del archivo temporal pedido
// Formato del mensaje: CNNNN...
// C = Codigo de mensaje ( = 3)
// NNNNNN = Nombre del archivo temporal

// Retorna: Lo solicitado en la lectura
//			0 + mensaje error si no se pudo leer

	int ok = 0;
	int bytesRecibidos;

	int desplazamiento = 0;
	int longitudBuffer = 0;
	int offset = 0;


	int posicion = 1;
	// Me fijo cuantos digitos tiene el numero de Bloque

	log_trace(logger, "COMANDO Leer Archivo Temporal");

	buffer = RecibirDatos(socket, buffer, &bytesRecibidos);

	char * aux = buffer;
	char* archivoTemporal = malloc(strlen(buffer));
	memcpy(archivoTemporal, aux,strlen(buffer));
	char* lectura = getFileContent(archivoTemporal);
	ok = 1; //Resultado de operacion


	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(strlen(lectura));
		memcpy(buffer,lectura,strlen(lectura));
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal(
				"ERROR LEER ARCHIVO TEMPORAL. %s.",stringErrorAux);
		if (buffer != NULL )
					free(buffer);
		buffer = malloc(3);
		string_append(&buffer, "-1");
		*longitud= strlen(buffer);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}

	if (lectura != NULL )
		free(lectura);
	return buffer;
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
