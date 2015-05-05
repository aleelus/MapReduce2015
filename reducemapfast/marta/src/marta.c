/*o
 ============================================================================
 Name        : marta.c
 Author      : Los Barderos
 Version     : 1.0
 Copyright   : Los Barderos - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 1C 2015
 Testing	 :
 ============================================================================
 */

#include "marta.h"

int main(int argv, char** argc) {
	//HARDCODEANDO
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	t_list *lista_nodos;
	lista_nodos = list_create();

	list_add(lista_nodos,nodo_create("NodoA","192.168.0.1",3000));
	list_add(lista_nodos,nodo_create("NodoB","192.168.0.2",3500));
	list_add(lista_nodos,nodo_create("NodoC","192.168.0.3",3200));
	list_add(lista_nodos,nodo_create("NodoD","192.168.0.4",3600));
	list_add(lista_nodos,nodo_create("NodoE","192.168.0.5",5050));

	t_nodo *el_nodo;
	int i=0;
	while(i<list_size(lista_nodos)){
		el_nodo = list_get(lista_nodos, i);
		printf("Nodo: %s \t IP: %s \t Puerto: %d\n",el_nodo->nombreNodo,el_nodo->ipNodo,el_nodo->puertoNodo);
		i++;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//inicializamos los semaforos
	//sem_init(&semaforoAccesoMemoria, 0, 1);
	//sem_init(&semaforoMarcosLibres, 0, 0);

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	//g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	//g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	logger = log_create(NOMBRE_ARCHIVO_LOG, "marta", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();



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

	return 0;
}


#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Puerto de escucha
		if (config_has_property(config, "PUERTO")) {
			g_Puerto = config_get_int_value(config, "PUERTO");
		} else
			Error("No se pudo leer el parametro PUERTO");
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
int chartToInt(char x) {
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


int posicionDeBufferAInt(char* buffer, int posicion) {
	int logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return chartToInt(buffer[posicion]);
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return posicionDeBufferAInt(buffer, 0);
}

char* RecibirDatos(int socket, char *buffer, int *bytesRecibidos) {
	*bytesRecibidos = 0;
	if (buffer != NULL ) {
		free(buffer);
	}

	char* bufferAux = malloc(BUFFERSIZE * sizeof(char));
	memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

	if ((*bytesRecibidos = *bytesRecibidos
			+ recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
		Error(
				"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",
				socket);
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%i", socket,
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
	char * bufferLogueo = malloc(cantidadDeBytesAEnviar+1);
	bufferLogueo[cantidadDeBytesAEnviar] = '\0';
	memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			(char*) bufferLogueo);

	return bytecount;
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
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

char* digitosNombreArchivo(char *buffer,int posicion){

	char *nombreArch=string_new();
	int digito=0,i=0,j=0,algo=0,aux=0,x=0;

	digito=posicionDeBufferAInt(buffer,posicion);
	for(i=1;i<=digito;i++){
		algo=posicionDeBufferAInt(buffer,posicion+i);
		aux=aux*10+algo;
	}
	for(j=posicion+i;j<posicion+i+aux;j++){
		nombreArch[x]=buffer[j];
		x++;
	}
	nombreArch[x]='\0';

	return nombreArch;

}

#define DIG_NARCHIVO 1

void atiendeJob (char *buffer){
	//BUFFER RECIBIDO = 21210file02.txt (EJEMPLO)

	int tipo_mensaje=0;
	char *nArchivo=string_new();
	tipo_mensaje=posicionDeBufferAInt(buffer,1);
	switch (tipo_mensaje) {
	case DIG_NARCHIVO:
		nArchivo=digitosNombreArchivo(buffer,2);
		printf("Nombre Archivo: %s\n",nArchivo);
		break;
	default:
		break;
	}

}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;

//Es el ID del programa con el que está trabajando actualmente el HILO.
//Nos es de gran utilidad para controlar los permisos de acceso (lectura/escritura) del programa.
//(en otras palabras que no se pase de vivo y quiera acceder a una posicion de memoria que no le corresponde.)
//	int id_Programa = 0;
//	int tipo_Cliente = 0;
	int longitudBuffer;
	//printf("ENTRE");

// Es el encabezado del mensaje. Nos dice que acción se le está solicitando a la msp
	int tipo_mensaje = 0;

// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(BUFFERSIZE * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

// Cantidad de bytes recibidos.
	int bytesRecibidos;

// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

// Código de salida por defecto
	int code = 0;

	while ((!desconexionCliente) & g_Ejecutando) {
		//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();
		char * mensajeOk = "Ok\n";

		//Recibimos los datos del cliente
		buffer = RecibirDatos(socket, buffer, &bytesRecibidos);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			tipo_mensaje = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (tipo_mensaje) {
			case ES_JOB:
				printf("Implemento Job(atiendeJob)\n");
				atiendeJob(buffer);
				break;
			case ES_FS:
				printf("implementar atiendeFS\n");
				//atiendeFS(buffer);
				break;
			default:
				break;
			}
			longitudBuffer=strlen(mensajeOk);
			//printf("\nRespuesta: %s\n",buffer);
			// Enviamos datos al cliente.
			EnviarDatos(socket, mensajeOk,longitudBuffer);
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
	my_addr.sin_port = htons(g_Puerto);
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
			socket_host, g_Puerto);

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


