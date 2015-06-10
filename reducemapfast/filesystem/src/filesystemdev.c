/*
 * filesystemdev.c
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#include "filesystem.h"

char letra = 'A';
int g_Ejecutando = 1;						// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int impre=0;

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

	char *la_Ip,*el_Puerto,*tamanioDatos;
	int digitosCantNumIp=0,tamanioDeIp;
	int posActual=0;
	t_nodo * el_nodo;

	//BUFFER RECIBIDO = 3220 (EJEMPLO)
	//BUFFER RECIBIDO = 3119127.0.0.12460002101000000000
	//3: es nodo 119127.0.0.1: la ip 246000: el puerto 2101000000000: el tamaño del espacio de datos del nodo
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
	tamanioDatos=DigitosNombreArchivo(buffer,&posActual);
	printf("Tamaño:%s\n",tamanioDatos);

	el_nodo = buscarNodo(la_Ip,el_Puerto);

	if(el_nodo==NULL){
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
		el_nodo = nodo_create(nombre,la_Ip,el_Puerto,tamanioDatos,0);
		list_add(lista_nodos,el_nodo);
	}

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
			el_archivo = archivo_create(nArchivo,1000,0,1);
			bloque->array[0].nombreNodo = "NodoA";
			bloque->array[0].nro_bloque = 30;
			bloque->array[1].nombreNodo = "NodoD";
			bloque->array[1].nro_bloque = 22;
			bloque->array[2].nombreNodo = "NodoE";
			bloque->array[2].nro_bloque = 11;
			bloque->bloque = 0;
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
	string_append(&buffer,"1212237/user/juan/datos/temperatura-2012.txt");
	string_append(&buffer,"13");
	string_append(&buffer,"15NodoA18Bloque3015NodoB18Bloque3715NodoF17Bloque8");
	string_append(&buffer,"15NodoE18Bloque1315NodoA18Bloque3815NodoC17Bloque7");
	string_append(&buffer,"15NodoA18Bloque4315NodoC18Bloque8815NodoB17Bloque2");
	string_append(&buffer,"237/user/juan/datos/temperatura-2013.txt");
	string_append(&buffer,"12");
	string_append(&buffer,"15NodoP18Bloque3115NodoF18Bloque4215NodoH17Bloque9");
	string_append(&buffer,"15NodoK18Bloque1115NodoB18Bloque5515NodoF17Bloque3");
	string_append(&buffer,"1815NodoA212192.168.1.2514600015NodoB212192.168.1.2514600015NodoC212192.168.1.2514600015NodoE212192.168.1.2514600015NodoF212192.168.1.2514600015NodoH212192.168.1.2514600015NodoK212192.168.1.2514600015NodoP212192.168.1.25146000");
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
		printf("El tamaño:"COLOR_VERDE"%s\n"DEFAULT,el_nodo->tamanio);
		printf("Estado:"COLOR_VERDE "%d\n"DEFAULT,el_nodo->estado);
		i++;
	}
}

void recursiva(int padre){
	t_list* lista;
	char caracter[1024];
	memset (caracter,' ',impre);
	int i=0;
	bool _true(void *elem){
		return ((t_filesystem*) elem)->padre==padre;
	}
	lista = list_filter(lista_filesystem,_true);
//	printf("Cantidad:%d\n",list_size(lista));
	if(lista!=NULL){
		impre++;
		while(i<list_size(lista)){
			t_filesystem * fs = list_get(lista,i);
			printf("%s/%s\n",caracter,fs->directorio);
			recursiva(fs->index);
			i++;
		}
	} else {
		impre--;
	}
}

void mostrarFilesystem(){
	printf("Raiz(/)\n");
	recursiva(0);
	impre=0;
}

void cargarFilesystem(){
	t_filesystem* el_fs;
	el_fs = filesystem_create(1,"etc",0);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(2,"home",0);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(3,"utnso",2);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(4,"tp",3);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(5,"tp-2015",4);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(6,"var",0);
	list_add(lista_filesystem,el_fs);
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
			case COMANDOFILESYSTEM:
				printf("Muestre la estructura de file system:\n");
				cargarFilesystem();
				mostrarFilesystem();
				mensaje ="Ok";
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
	my_addr.sin_port = htons(fs_Puerto);
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
			socket_host, fs_Puerto);

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

t_nodo * buscarNodo(char* ipNodo,char* puertoNodo){
	t_nodo* el_nodo = malloc(sizeof(t_nodo));
	bool _true(void *elem){
		return ((!strcmp(((t_nodo*) elem)->ip,ipNodo)) && (!strcmp(((t_nodo*) elem)->puerto,puertoNodo)));
	}
	el_nodo = list_find(lista_nodos, _true);
	return el_nodo;
}


int agregarNodo(){
	char * ipNodo = malloc(TAMANIO_IP);
	char * puertoNodo = malloc(sizeof(int));


	t_nodo *el_nodo;

	printf("Ingrese la ip del nodo: ");
	scanf("%s",ipNodo);
	fflush(stdin);
	printf("Ingrese el puerto de escucha del nodo: ");
	scanf("%s",puertoNodo);
	fflush(stdin);
	el_nodo = buscarNodo(ipNodo,puertoNodo);

	if(el_nodo!=NULL){
		printf("\nNodo %s Habilitado!\n",el_nodo->nombre);
		el_nodo->estado = 1;
		return 1;
	} else {
		printf("\nNo se pudo habilitar el nodo porque no existe\n");
		return 0;
	}
}

int validarDirectorio(char *directorio,int i){
	t_list * lista = list_create();
	t_filesystem* fs;
	bool _true(void *elem){
		return ((t_filesystem*) elem)->padre==i;
	}
	lista = list_filter(lista_filesystem,_true);
	if(lista!=NULL){
		bool _true2(void *elem){
			return !strcmp(((t_filesystem*) elem)->directorio,directorio);
		}
		fs = list_find(lista,_true2);
		if(fs==NULL){
			printf("Directorio:%s Invalido\n",directorio);
			return 0;
		} else {
			printf("Directorio:%s Valido\n",directorio);
			return fs->index;
		}
	}
	return 0;
}

int subirArchivo(long unsigned *tamanio,FILE ** fArchivo){
	char nombreArchivo[20];
	char directorio[50];
	int correcto = 0;
	char * path = string_new();

	printf("Ingrese el nombre del archivo para subir: ");
	scanf("%s",nombreArchivo);
	fflush(stdin);
	//tamanio = abrirArchivo(&fArchivo,&nombreArchivo);

	if(( *fArchivo = fopen(nombreArchivo, "r") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", nombreArchivo, strerror(errno));
		return 0;
	}

	fseek(*fArchivo, 0L, SEEK_END);
	*tamanio = ftell(*fArchivo);
	rewind(*fArchivo);

	printf("El Tamaño del archivo es:%lu\n",*tamanio);

	if(!tamanio){
		printf("No se pudo abrir correctamente el archivo:%s\n",nombreArchivo);
	}
	archivo = archivo_create(nombreArchivo,*tamanio,0,1);


	int j=0,padre=0,k;
	while(!correcto){
		mostrarFilesystem();
		printf("Ingrese de un directorio: ejemplo: home\n");
		printf("Path Ingresados:%s\n",path);
		printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
		scanf("%s",directorio);
		fflush(stdin);
		if(strcmp(directorio,"1")){
			if(!strcmp(directorio,"0")){
					j=0;
					path=string_new();
			} else {
				if(j==0){
					k=validarDirectorio(directorio,0);
				} else {
					k=validarDirectorio(directorio,padre);
				}
				if(k!=0){
					padre=k;
					string_append(&path,"/");
					string_append(&path,directorio);
					j++;
				}
			}
		} else {
			correcto = 1;
		}
		system("clear");
	}
	return 1;
}

int buscarNodoEnArray (){

    int i=0,k=0,bandera=0;
    int cantNodos=list_size(lista_nodos);
    t_array_nodo *el_array_nodo;

    for(k=0;k<cantNodos;k++){
        if(list_size(arrayNodos[k])>0){
            i=0;
            while(i<list_size(arrayNodos[k])){
                el_array_nodo=list_get(arrayNodos[k],i);
                if(strcmp(el_array_nodo->nombreArchivo,archivo->nombreArchivo)==0 && el_array_nodo->padre==archivo->padre){
                    bandera=1;
                }
                i++;
            }

            if(bandera==0){
                return k;
            }else{
                bandera=0;
            }
        }
    }

    return 0;

}

void llenarArrayDeNodos (){

    arrayNodos=(t_list**)malloc(list_size(lista_nodos)*sizeof(t_array_nodo));

    int i=0,k=0,posicion=0;
    t_nodo *el_nodo;
    t_array_nodo *el_array_nodo;


    for(i=0;i<list_size(lista_nodos);i++){
        arrayNodos[i] = list_create();
    }

    for(i=0;i<list_size(lista_nodos);i++){
        el_nodo=list_get(lista_nodos,i);

        for(k=0;k<list_size(lista_nodos);k++){

            bool _true(void *elem) {
                return ( !strcmp(((t_array_nodo*) elem)->nombre,el_nodo->nombre) );
            }


            el_array_nodo = list_find(arrayNodos[k], _true);

            if(el_array_nodo != NULL){
                posicion = k;
                k=list_size(lista_nodos);
            }

        }


        if(el_array_nodo == NULL){

            list_add(arrayNodos[posicion],array_nodo_create(el_nodo->nombre));

        }

    }
}

void ordenarArrayNodos(){
	int k;
	t_list *aux;
	int ordenado =0;

	while(ordenado==0){
		ordenado=1;
		for(k=1;k<list_size(lista_nodos);k++){
			if(list_size(arrayNodos[k])<list_size(arrayNodos[k-1])){
				aux = arrayNodos[k-1];
				arrayNodos[k-1]=arrayNodos[k];
				arrayNodos[k]=aux;
				ordenado=0;
			}
		}
	}
}

char* obtenerSubBuffer(char *nombre){
	// Esta funcion recibe un nombre y devuelve ese nombre de acuerdo al protocolo. Ej: carlos ------> 16carlos
	char *aux=string_new();
	int tamanioNombre=0;
	float tam=0;
	int cont=0;

	tamanioNombre=strlen(nombre);
	tam=tamanioNombre;
	while(tam>=1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioNombre));
	string_append(&aux,nombre);

	return aux;
}

void eliminarNodo (){

    t_nodo *el_nodo;
    int i=0,eleccion=0;

    while(i<list_size(lista_nodos)){
        el_nodo=list_get(lista_nodos,i);

        printf("Nro (%d)",i);
        printf("Nodo%s\n",el_nodo->nombre);
        printf("IP: %s\n",el_nodo->ip);
        printf("Puerto: %s\n",el_nodo->puerto);
        i++;
    }

    printf("Elija el Nro de nodo a eliminar: ");
    scanf("%d",&eleccion);
    if(eleccion>0 && eleccion<i){
        el_nodo=list_get(lista_nodos,eleccion);
        el_nodo->estado=0;
    }


}


void enviarBufferANodo(t_envio_nodo* envio_nodo){
	int socket,bytesRecibidos,cantRafaga=1,tamanio=10;
	char* buffer1,*buffer2,*bufferR;
	buffer1=string_new();
	buffer2=string_new();
	conectarNodo(&socket,envio_nodo->ip,envio_nodo->puerto);
	//Segunda Rafaga
	string_append(&buffer2,"13");
	string_append(&buffer2,obtenerSubBuffer(string_itoa(envio_nodo->bloque)));
	string_append(&buffer2,obtenerSubBuffer(envio_nodo->buffer));
	//Primer Rafaga
	string_append(&buffer1,"1");
	string_append(&buffer1,string_itoa(cuentaDigitos(strlen(envio_nodo->buffer))));
	string_append(&buffer1,string_itoa(strlen(envio_nodo->buffer)));
	EnviarDatos(socket,buffer1,strlen(buffer1));
	RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
	EnviarDatos(socket,buffer2,strlen(buffer2));
}

t_nodo* buscarNodoPorNombre(char* nombre){
	t_nodo* el_nodo = malloc(sizeof(t_nodo));
	bool _true(void *elem){
		return ((!strcmp(((t_nodo*) elem)->nombre,nombre)));
	}
	el_nodo = list_find(lista_nodos, _true);
	return el_nodo;
}

int buscarBloqueDisponible(t_nodo*nodo){
	t_bloque_disponible* bloque;
	bloque = list_remove(nodo->bloquesDisponibles,0);
	if(bloque!=NULL){
		return bloque->bloque;
	} else {
		return 0;
	}
}

t_array_copias *funcionLoca(char* buffer,t_bloque ** bloque,int j){
	int nroNodo,bloqueDisponible;
	t_nodo* el_nodo;
	t_array_nodo* nodo;
	t_envio_nodo* envio_nodo;
	ordenarArrayNodos();
	nroNodo = buscarNodoEnArray();
	nodo = list_get(arrayNodos[nroNodo],0);
	el_nodo=buscarNodoPorNombre(nodo->nombre);
	bloqueDisponible = buscarBloqueDisponible(el_nodo);
	envio_nodo = envio_nodo_create(buffer,el_nodo->ip,el_nodo->puerto,bloqueDisponible);
	int iThreadHilo = pthread_create(&hNodos, NULL,
			(void*) enviarBufferANodo, (void*) envio_nodo );
	if (iThreadHilo) {
		fprintf(stderr,
			"Error al crear hilo - pthread_create() return code: %d\n",
			iThreadHilo);
		exit(EXIT_FAILURE);
	}

	pthread_join(hNodos, NULL );
	return array_copias_create(nodo->nombre,bloqueDisponible);
}

void enviarCopias(char*bufferAux,t_bloque ** bloque){
	int j;
	nroBloque++;
	for(j=0;j<3;j++){
		funcionLoca(bufferAux,bloque,j);
	}
}

void enviarBloque(char *bufferAux){
	t_bloque * bloque;
	bloque = bloque_create(nroBloque++);
	enviarCopias(bufferAux,&bloque);
	list_add(archivo->listaBloques,bloque);
}

void recorrerArchivo(FILE *fArchivo){
	char * buffer,*bufferAux;
	buffer = malloc(TAMANIO_BLOQUE+1);
	//memset(&buffer,0,TAMANIO_BLOQUE+1);
	long unsigned tamanio,tamanioA;
	fseek(fArchivo,0L,SEEK_END);
	tamanioA=ftell(fArchivo);
	rewind(fArchivo);
	int i=0,j=0;
	llenarArrayDeNodos();
	while(!feof(fArchivo)){
		fread(buffer,1,TAMANIO_BLOQUE,fArchivo);
		for(j=0;j<TAMANIO_BLOQUE;j++){
			if(buffer[TAMANIO_BLOQUE-j]=='\n'){
				bufferAux = malloc(TAMANIO_BLOQUE-j+1);
				memcpy(bufferAux,buffer,TAMANIO_BLOQUE-j);
				enviarBloque(bufferAux);
				i++; //Contador de Bloques
				printf("LA I:%d\n",i);
				printf("LA J:%d\n",j);
				tamanio = ftell(fArchivo);
				if(tamanio!=tamanioA){
					fseek(fArchivo,-j,SEEK_CUR);
					free(bufferAux);
					printf("TAMANIO:%lu\n",tamanio);
				}
				j=TAMANIO_BLOQUE;
			}
		}

		/*if(c[0] == '\n'){
			tamanio = strlen(bufferP);
			//printf("|%lu|\n",tamanio);
			if(tamanio<100000){
				bufferAux = bufferP;
				k=i;
			} else {
				//enviarBuffer(bufferAux);
				bufferP = string_new();
				fseek(fArchivo, k, SEEK_SET);
				j++;
				if((tamanioA-k)<100000){
					j++;
				}
			}
		}
		string_append(&bufferP,c);
		tamanio = strlen(bufferP);
		//printf("BUFFER:%s\n",buffer);
		i++;*/
	}
	printf("Cantidad Bloques:%d\n",i);
	free(buffer);
}

int procesarArchivo(){
	long unsigned tamanio;
	FILE * fArchivo;
	//char * buffer = malloc(100);

	if(subirArchivo(&tamanio,&fArchivo)){
		recorrerArchivo(fArchivo);
		return 1;
	}
		//printf("Buffer:%s\n",buffer);
	return 1;
}

void eliminarFilesystem(){
	int i = 0;
	while(i<list_size(lista_filesystem)){
		list_remove_and_destroy_element(lista_filesystem,i++,(void*)filesystem_destroy);
	}
	free(lista_filesystem);
}

void eliminarArchivos(){
	int i = 0;
	while(i<list_size(lista_archivos)){
		list_remove_and_destroy_element(lista_archivos,i++,(void*)archivo_destroy);
	}
	free(lista_archivos);
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
		eliminarFilesystem();
		printf("Se elimino el filesystem\n");
		eliminarArchivos();
		printf("Se elimino los archivos\n");
		log_info(logger, "Ejecutando Formatear MDFS...\n");
		break;
	case 2:
		log_info(logger, "Se realizo Eliminar/Renombrar/Mover Archivos\n");
		break;
	case 3:
		log_info(logger, "Se realizo Crear/Eliminar/Renombrar/Mover Directorios\n");
		break;
	case 4:
		if(procesarArchivo()){
			log_info(logger, "Se realizo Copiar un archivo local al MDFS\n");
		} else {
			log_info(logger, "No se realizo Copiar un archivo local al MDFS\n");
		}
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
		eliminarNodo();
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
void mostrarError(t_error unError){
	switch(unError){
		case CantidadArgumentosIncorrecta: puts("CantidadArgumentosIncorrecta.");break;
		case NoSePudoAbrirConfig:		   puts("No se pudo abrir configuracion");break;
		case NoSePuedeObtenerPuerto:	   puts("No se pudo obtener puerto");break;
		case NoSePuedeObtenerNodos:		   puts("No se pudo obtener nodos");break;
		case NosePuedeCrearHilo:		   puts("Error al crear hilo - pthread_create() return code: %d\n"); break;
		case OtroError:	 				   puts("OtroError.");break;
	};
}

#if 1 // METODOS CONFIGURACION //
void levantarConfig() {

	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {
		// Puerto de escucha
		if (config_has_property(config, "PUERTO_LISTEN")) {
			fs_Puerto = config_get_int_value(config, "PUERTO_LISTEN");
		} else
			Error("No se pudo leer el parametro PUERTO_LISTEN");
		if (config_has_property(config, "CANTIDAD_NODOS")) {
			fs_CantidadNodos = config_get_int_value(config, "CANTIDAD_NODOS");
		} else
			Error("No se pudo leer el parametro CANTIDAD_NODOS");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif

t_bloque *bloque_create(int bloque) {
	t_bloque *new = malloc(sizeof(t_bloque));
	new->bloque   = bloque;
	return new;
}

void archivo_destroy(t_archivo* self) {
	int i = 0;
	while(i<list_size(self->listaBloques)){
		list_remove_and_destroy_element(self->listaBloques,i++,(void*)bloque_destroy);
	}
	free(self->listaBloques);
	free(self);
}

void bloque_destroy(t_bloque* self) {
	free(self);
}

t_archivo *archivo_create(char *nombreArchivo,long unsigned tamanio,int padre, int estado) {
	t_archivo *new     = malloc(sizeof(t_archivo));
    new->nombreArchivo = strdup(nombreArchivo);
    new->padre = padre;
    new->tamanio = tamanio;
    new->estado = estado;
    new->listaBloques  = list_create();
    return new;
}

void filesystem_destroy(t_filesystem* self) {
	free(self);
}

t_filesystem* filesystem_create(int index, char* directorio, int padre){
	t_filesystem * new=malloc(sizeof(t_filesystem));
	new->index = index;
	new->directorio = strdup(directorio);
	new->padre = padre;
	return new;
}

t_nodo *nodo_create(char *nombreNodo, char *ipNodo, char* puertoNodo, char* tamanio, int activo) {
	t_nodo *new = malloc(sizeof(t_nodo));
	new->nombre = strdup(nombreNodo);
	new->ip = strdup(ipNodo);
	new->puerto = puertoNodo;
	new->tamanio = tamanio;
	new->estado = activo;
	new->bloquesDisponibles = list_create();
	return new;
}

void nodo_destroy(t_nodo* self) {
	free(self);
}

t_array_copias* array_copias_create(char* nombre, int bloque){
	t_array_copias* new = malloc(sizeof(t_array_copias));
	new->nombreNodo = nombre;
	new->nro_bloque = bloque;
	return new;
}

t_array_nodo *array_nodo_create(char *nombre) {
    t_array_nodo *new = malloc(sizeof(t_array_nodo));
    new->nombre=strdup(nombre);
    new->bloqueArchivo=string_new();
    new->nombreArchivo=string_new();
    new->padre=0;
    return new;
}

t_envio_nodo *envio_nodo_create(char *buffer, char* ip, char* puerto, int bloque) {
    t_envio_nodo *new = malloc(sizeof(t_envio_nodo));
    new->buffer=strdup(nombre);
    new->ip=strdup(ip);
    new->puerto=strdup(puerto);
    new->bloque = bloque;
    return new;
}

t_bloque_disponible *bloque_disponible_create(int bloque) {
    t_bloque_disponible *new = malloc(sizeof(t_bloque_disponible));
    new->bloque = bloque;
    return new;
}
