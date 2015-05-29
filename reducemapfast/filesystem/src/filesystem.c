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

	//Variable del nombre de los nodos
	 nombre = string_new();

	//Lista de nodos
	lista_nodos = list_create();

	//Lista de archivos
	lista_archivos = list_create();

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
	g_MensajeError = malloc(1 * sizeof(char));

	//Este hilo es el que maneja la consola
	int iThreadConsola = pthread_create(&hConsola, NULL, (void*) Comenzar_Consola,
				NULL );
	if (iThreadConsola) {
		fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadConsola);
		exit(EXIT_FAILURE);
	}

	//Hilo orquestador conexiones para escuchar a Marta o a Nodos
		int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
				(void*) HiloOrquestadorDeConexiones, NULL );
		if (iThreadOrquestador) {
			fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadOrquestador);
			exit(EXIT_FAILURE);
		}

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );

	return EXIT_SUCCESS; 	

}

int eliminarMongo(){
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	mongoc_init ();

	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "archivos");

	doc = bson_new ();
	bson_oid_init (&oid, NULL);
	BSON_APPEND_OID (doc, "_id", &oid);
	BSON_APPEND_UTF8 (doc, "hello", "world");

	if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
		printf ("Insert failed: %s\n", error.message);
	}

	bson_destroy (doc);

	doc = bson_new ();
	BSON_APPEND_OID (doc, "_id", &oid);

	if (!mongoc_collection_remove (collection, MONGOC_DELETE_SINGLE_REMOVE, doc, NULL, &error)) {
		printf ("Delete failed: %s\n", error.message);
	}

	bson_destroy (doc);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);

	return 0;
}

int leerMongo(){
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;

	mongoc_init ();

	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "archivos");

	query = bson_new ();
	cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
	    printf ("ACA:%s\n", str);
	    bson_free (str);
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);

	return 0;
}

int iniciarMongo(){

	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	mongoc_init ();

	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "archivos");

	doc = bson_new ();
	bson_oid_init (&oid, NULL);
	BSON_APPEND_OID (doc, "_id", &oid);
	BSON_APPEND_UTF8 (doc, "hello", "world");

	if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
		printf ("%s\n", error.message);
	}

	bson_destroy (doc);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);



    return 0;

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

char* DigitosNombreArchivo(char *buffer,int *posicion){

	char *nombreArch;
	int digito=0,i=0,j=0,algo=0,aux=0,x=0;

	digito=PosicionDeBufferAInt(buffer,*posicion);
	for(i=1;i<=digito;i++){
		algo=PosicionDeBufferAInt(buffer,*posicion+i);
		aux=aux*10+algo;
	}
	nombreArch = malloc(aux+1);
	for(j=*posicion+i;j<*posicion+i+aux;j++){
		nombreArch[x]=buffer[j];
		x++;
	}
	nombreArch[x]='\0';
	*posicion=*posicion+i+aux;
	return nombreArch;
}

int AtiendeNodo(char* buffer,int*cantRafaga){

	char *la_Ip,*el_Puerto;
	int digitosCantNumIp=0,tamanioDeIp;
	int posActual=0;
	t_nodo * el_nodo;

	//BUFFER RECIBIDO = 3220 (EJEMPLO)
	//BUFFER RECIBIDO = 3119127.0.0.1246000
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 411

	digitosCantNumIp=PosicionDeBufferAInt(buffer,2);
	printf("Cantidad de digitos de Tamanio de Ip:%d\n",digitosCantNumIp);
	tamanioDeIp=ObtenerTamanio(buffer,3,digitosCantNumIp);
	printf("Tamaño de IP:%d\n",tamanioDeIp);
	posActual=1+digitosCantNumIp;
	la_Ip=DigitosNombreArchivo(buffer,&posActual);
	printf("Ip:%s\n",la_Ip);
	el_Puerto=DigitosNombreArchivo(buffer,&posActual);
	printf("Puerto:%s\n",el_Puerto);
	free(nombre);
	nombre = string_new();
	if(letra>'B'){
		letra = 'A';
		string_append(&nombre,"NodoA");
	} else {
		string_append(&nombre,"Nodo");
	}
	string_append(&nombre,&letra);
	letra++;
	el_nodo = nodo_create(nombre,la_Ip,el_Puerto,0);
	list_add(lista_nodos,el_nodo);

	return 1;
}


void AtiendeMarta(char* buffer,int*cantRafaga){

	char *nArchivo;
	int digitosCantDeArchivos=0,cantDeArchivos=0;
	int x,posActual=0;
	t_archivo * el_archivo;
	t_bloque *bloque = malloc(sizeof(t_bloque));

	//BUFFER RECIBIDO = 4270 (EJEMPLO)
	//BUFFER RECIBIDO = 4112220temperatura-2012.txt220temperatura-2013.txt
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 411

	digitosCantDeArchivos=PosicionDeBufferAInt(buffer,2);
	printf("CANTIDAD DE DIGITOS:%d\n",digitosCantDeArchivos);
	cantDeArchivos=ObtenerTamanio(buffer,3,digitosCantDeArchivos);
	printf("Cantidad de Archivos:%d\n",cantDeArchivos);
	posActual=3+digitosCantDeArchivos;

		for(x=0;x<cantDeArchivos;x++){
			//printf("PosicionActual:%d",posActual);
			nArchivo=DigitosNombreArchivo(buffer,&posActual);
			//printf("NOMBRE:%s\n",nArchivo);
			el_archivo = archivo_create(nArchivo);
			bloque->array[0].nodo = "NodoA";
			bloque->array[0].bloque = "Bloque30";
			bloque->array[1].nodo = "NodoD";
			bloque->array[1].bloque = "Bloque22";
			bloque->array[2].nodo = "NodoE";
			bloque->array[2].bloque = "Bloque11";
			bloque->bloque = "Bloque0";
			list_add(el_archivo->listaBloques,bloque);
			list_add(lista_archivos,el_archivo);
			//free(nArchivo);
			//ObtenerBloquesDeArchivo(nArchivo,);
		}
		//nResultado=DigitosNombreArchivo(buffer,&posActual);

		//Muestro por pantalla los Archivos
		int i=0;
		while(i<list_size(lista_archivos)){
			el_archivo = list_get(lista_archivos, i);
			printf("El archivo:%s\n",el_archivo->nombreArchivo);
			i++;
		}
		//*cantRafaga=1;
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return PosicionDeBufferAInt(buffer, 0);
}

int cuentaDigitos(int valor){
	int cont = 0;
	float tamDigArch=valor;
	while(tamDigArch>1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	return cont;
}

int EnviarInfoMarta(int socket) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);
	int cont,cantidadDeBytesAEnviar;
	int bytecount,bytesRecibidos,cantRafaga=1,tamanio;
	char*buffer = string_new();
	char*bufferR = string_new();
	char*bufferE = string_new();

	//VIEJO PROTOCOLO
	//BUFFER RECIBIDO = 1212220temperatura-2012.txt121015NODOA23015NODOB22215NODOF18
	//												 1115NODOE23015NODOJ22215NODOW18
	//					    220temperatura-2013.txt121015NODOA23015NODOB22215NODOF23
	//												 1115NODOE23015NODOJ22215NODOW18
	//					15NODOA19127.0.0.114600015NODOB19127.0.0.1146000
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 411

	//NUEVO PROTOCOLO
	//BUFFER RECIBIDO = 1212237/user/juan/datos/temperatura-2012.txt1215NODOA18Bloque3015NODOB18Bloque3715NODOF17Bloque8
		//											 				  15NODOE18Bloque1315NODOA18Bloque3815NODOC17Bloque7
		//			        237/user/juan/datos/temperatura-2013.txt1215NODOP18Bloque3115NODOF18Bloque4215NODOH17Bloque9
		//										     			      15NODOK18Bloque1115NODOB18Bloque5515NODOF17Bloque3
		//				  1815NODOA19127.0.0.114600015NODOB19127.0.0.1146000
//							15NODOC19127.0.0.114600015NODOE19127.0.0.1146000
//							15NODOF19127.0.0.114600015NODOH19127.0.0.1146000
//							15NODOK19127.0.0.114600015NODOP19127.0.0.1146000

	//Bloque0 :: NODOA:Bloque30 NODOB:Bloque37 NODOF:Bloque8
	//Bloque1 :: NODOE:Bloque13 NODOA:Bloque38 NODOC:Bloque7

	//Bloque0 :: NODOP:Bloque21 NODOF:Bloque42 NODOH:Bloque9
	//Bloque1 :: NODOK:Bloque11 NODOB:Bloque55 NODOF:Bloque3


	buffer=string_new();
	string_append(&buffer,"1212237/user/juan/datos/temperatura-2012.txt1215NodoA18Bloque3015NodoB18Bloque3715NodoF17Bloque815NodoE18Bloque1315NodoA18Bloque3815NodoC17Bloque7237/user/juan/datos/temperatura-2013.txt1215NodoP18Bloque3115NodoF18Bloque4215NodoH17Bloque915NodoK18Bloque1115NodoB18Bloque5515NodoF17Bloque31815NodoA19127.0.0.114600015NodoB19127.0.0.114600015NodoC19127.0.0.114600015NodoE19127.0.0.114600015NodoF19127.0.0.114600015NodoH19127.0.0.114600015NodoK19127.0.0.114600015NodoP19127.0.0.1146000");
	cantidadDeBytesAEnviar = strlen(buffer);
	cont = cuentaDigitos(cantidadDeBytesAEnviar);
	string_append(&bufferE,"1");
	string_append(&bufferE,string_itoa(cont));
	string_append(&bufferE,string_itoa(cantidadDeBytesAEnviar));

	//Primera Rafaga para Marta
	if ((bytecount = send(socket, bufferE, strlen(bufferE), 0)) == -1)
		Error("No puedo enviar información al cliente. Socket: %d", socket);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
		(char*) bufferE);

	bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);

	if(strcmp(bufferR,"Ok")==0){
		//Segunda Rafaga para Marta
		if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1)
			Error("No puedo enviar información al cliente. Socket: %d", socket);
		log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			(char*) buffer);
		bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);

	} else {
		Error("Marta tuvo algun problema con la rafaga 1. Socket: %d", socket);
		return 0;
	}
	return bytecount;
}


void implementoMarta(int *id,char * buffer,int * cantRafaga,char ** mensaje, int socket){

	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	printf("RAFAGA:%d\n",tipo_mensaje);
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case CONSULTA_ARCHIVO:
			AtiendeMarta(buffer,cantRafaga);
			EnviarInfoMarta(socket);
			*cantRafaga=1;
			break;
		default:
			break;
		}
		//*mensaje = "Ok";
	} else {
		if (*cantRafaga==1) {
			*mensaje = "Ok";
			*cantRafaga = 2;
		} else {
			*mensaje = "No";
		}
	}
}

void implementoNodo(char * buffer,int * cantRafaga,char ** mensaje, int socket){

	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	printf("RAFAGA:%d\n",tipo_mensaje);
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case CONEXION:
			if(AtiendeNodo(buffer,cantRafaga)){
				*mensaje = "Ok";
			} else {
				*mensaje = "No";
			}
			*cantRafaga=1;
			break;
		default:
			break;
		}
		//*mensaje = "Ok";
	} else {
		if (*cantRafaga==1) {
			*mensaje = "Ok";
			*cantRafaga = 2;
		} else {
			*mensaje = "No";
		}
	}
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

void RecorrerNodos(){
	t_nodo * el_nodo;

	int i=0;
	while(i<list_size(lista_nodos)){
		el_nodo = list_get(lista_nodos, i);
		printf("Nodo:"COLOR_VERDE "%s\n"DEFAULT,el_nodo->nombre);
		printf("La IP:"  COLOR_VERDE"%s\n"DEFAULT,el_nodo->ip);
		printf("El Puerto:"COLOR_VERDE"%s\n"DEFAULT,el_nodo->puerto);
		printf("Estado:"COLOR_VERDE "%d\n"DEFAULT,el_nodo->estado);
		i++;
	}
}


int AtiendeCliente(void * arg) {
	int socket = (int) arg;
	int id=-1;

	int longitudBuffer;

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
			case ES_MARTA:
				implementoMarta(&id,buffer,&cantRafaga,&mensaje,socket);
				break;
			case ES_NODO:
				printf("implementar atiendeNodo\n");
				implementoNodo(buffer,&cantRafaga,&mensaje,socket);
				break;
			case COMANDO:
				printf("Muestre toda la lista de Nodos:\n");
				RecorrerNodos();
				mensaje = "Ok";
				break;
			case COMANDOBLOQUES:
				printf("Muestre toda la lista de Bloques:\n");
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
	my_addr.sin_port = htons(configuracion.puerto_listen);
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
			socket_host, configuracion.puerto_listen);

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


int conectarNodo(int * socket_Nodo, char* ipNodo, char* puertoNodo) {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON NODO
	log_info(logger, "Intentando conectar a nodo\n");
	//conectar con Nodo
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int conexionOk = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(ipNodo, puertoNodo, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_nodo");
	}

	if ((*socket_Nodo = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_Marta");
	}
	if (connect(*socket_Nodo, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_Nodo");
	} else {
		conexionOk = 1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return conexionOk;
}


int corte_consola() {
	int corte;
	printf(
			"\n\n\ningrese 0 si desea terminar la consola del filesystem, sino ingrese otro numero\n\n\n");

	scanf("%d", &corte);
	return corte;
}

void Comenzar_Consola() {

	int corte_consola = -1;

	while (corte_consola != 0) {
		corte_consola = operaciones_consola();//menu de consola para elegir la opcion a realizar en filesystem
	}
	printf("Se termino la ejecucion de la consola del filesystem\n");
	//free(puntero_inicial);
}

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

int agregarNodo(){
	char * ipNodo = malloc(TAMANIO_IP);
	char * puertoNodo = malloc(sizeof(int));
	int socket_Nodo;
	t_nodo *nodo;
	free(nombre);
	nombre = string_new();
	if(letra>'B'){
		letra = 'A';
		string_append(&nombre,"NodoA");
	} else {
		string_append(&nombre,"Nodo");
	}
	printf("Ingrese la ip del nodo: ");
	scanf("%s",ipNodo);
	fflush(stdin);
	printf("Ingrese el puerto de escucha del nodo: ");
	scanf("%s",puertoNodo);
	fflush(stdin);
	if(conectarNodo(&socket_Nodo,ipNodo,puertoNodo)!=0){
		EnviarDatos(socket_Nodo,"110", strlen("110"));
		printf("\nNodo Conectado!");
		//Agregar Mutex
		string_append(&nombre,&letra);
		nodo = nodo_create(nombre,ipNodo,puertoNodo,1);
		list_add(lista_nodos,nodo);
		//MUTEX
		letra++;
		return 1;
	} else {
		printf("\nNo se pudo conectar el nodo porque no esta implementado");
		return 0;
	}
}

void RecorrerListaNodos(){
	t_nodo * el_nodo;

	int i=0;

	while(i<list_size(lista_nodos)){
		el_nodo = list_get(lista_nodos, i);
		printf("Nombre Nodo:%s\n",el_nodo->nombre);
		printf("IP:%s\n",el_nodo->ip);
		printf("Puerto:%s\n",el_nodo->puerto);
		printf("Activo:%d\n\n",el_nodo->estado);
		i++;
	}

}



int operaciones_consola() {
	//system("clear");
	printf("Comandos posibles: \n");
	printf("1 - Formatear MDFS\n");
	printf("2 - Eliminar/Renombrar/Mover Archivos\n");
	printf("3 - Crear/Eliminar/Renombrar/Mover Directorios\n");
	printf("4 - Copiar un archivo local al MDFS\n");
	printf("5 - Copiar un archivo del MDFS al filesystem local\n");
	printf("6 - Solicitar el MD5 de un archivo en MDFS\n");
	printf("7 - Ver/Borrar/Copiar los bloques que componen un archivo\n");
	printf("8 - Agregar un nodo de datos\n");
	printf("9 - Eliminar un nodo de datos\n");
	printf("0 - Salir\n");

	int variable_seleccion;
	scanf("%d", &variable_seleccion);

	system("clear");
	switch (variable_seleccion) {

	case 0:
		log_info(logger, "Terminando el programa");
		return 0;
		break;
	case 1:
		log_info(logger, "Ejecutando Formatear MDFS...\n");
		break;
	case 2:
		log_info(logger, "Se realizo Eliminar/Renombrar/Mover Archivos\n");
		break;
	case 3:
		log_info(logger, "Se realizo Crear/Eliminar/Renombrar/Mover Directorios\n");
		break;
	case 4:
		log_info(logger, "Se realizo Copiar un archivo local al MDFS\n");
		break;
	case 5:
		log_info(logger, "Se realizo Copiar un archivo del MDFS al filesystem local\n");
		break;
	case 6:
		log_info(logger, "Se realizo Solicitar el MD5 de un archivo en MDFS\n");
		break;
	case 7:
		log_info(logger, "Se realizo Ver/Borrar/Copiar los bloques que componen un archivo\n");
		break;
	case 8:
		if(agregarNodo()){
			log_info(logger, "Se realizo Agregar un nodo de datos\n");
		} else {
			log_info(logger, "No se pudo Agregar un nodo de datos\n");
		}
		break;
	case 9:
		log_info(logger, "Se realizo Eliminar un nodo de datos\n");
		break;
	case 10:
		printf("Muestre toda la lista de Nodos:\n");
		RecorrerListaNodos();
		break;
	default:
		operaciones_consola();
		break;
	return -1;
	}
	return -1;
}


//Mostrar Error
void mostrarError(error unError){
	switch(unError){
		case CantidadArgumentosIncorrecta: puts("CantidadArgumentosIncorrecta.");break;
		case NoSePudoAbrirConfig:		   puts("No se pudo abrir configuracion");break;
		case NoSePuedeObtenerPuerto:	   puts("No se pudo obtener puerto");break;
		case NoSePuedeObtenerNodos:		   puts("No se pudo obtener nodos");break;
		case OtroError:	 				   puts("OtroError.");break;
	};
}

int leer_config(){

	char *lista_nodos;								//Lista de Nodos necesarios para empezar
	char linea[MAXLINEA];							//Linea de configuracion
	FILE *config;									//Archivo de configuracion
	char ch;										//Exclusion de lineas

	if ( (config = fopen(PATH_CONFIG,"rt")) == NULL ) {
		mostrarError(NoSePudoAbrirConfig);
		return EXIT_FAILURE;
		}

	if ((lista_nodos = malloc(MAXLINEA+1)) == NULL)
		return EXIT_FAILURE;

	while(fgets(linea, MAXLINEA, config) != NULL){		// Obtener valores de configuracion

		if(sscanf(linea, "%*[^\n#]%c", &ch) != 1){
			;											//Se ignoran las lineas en blanco y comentarios
		}

		if(sscanf(linea, " PUERTO_LISTEN= %u", &configuracion.puerto_listen) == 1){
			printf("%u\n", configuracion.puerto_listen);
		}
	}
	fclose(config);
	return EXIT_SUCCESS;
}

