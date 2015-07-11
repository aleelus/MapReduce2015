/*
 ============================================================================
 Name        : job.c
 Author      : Los Barderos
 Version     : 1.0
 Copyright   : Los Barderos - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 1C 2015
 Testing	 :
 ============================================================================

*/
#include "job.h"



int main(int argv, char** argc) {
	//inicializamos los semaforos


	sem_init(&semaforoLogger, 0, 1);
	sem_init(&semaforoMarta,1,1);
	sem_init(&semaforoNodo,1,1);
	//sem_init(&semaforo,1,0);
	//sem_init(&semaforoJob,1,0);
	// Instanciamos el archivo donde se grabará lo solicitado por consola
	//g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	//g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	//Log de trabajo
	logger = log_create(NOMBRE_ARCHIVO_LOG, "job", true, LOG_LEVEL_TRACE);

	//Buffers
	char * buffer= string_new();
	char *bufferRafaga_Dos=string_new();
	char *bufferRafaga_Uno=string_new();
	bufferANodo=string_new();

	//Flags
	int cantidadRafagaMarta=1, cantRafaga=1;
	int desconexionCliente = 0;
	int g_Ejecutando = 1;

	//Tamaños iniciales y contadores
	int tamanio=10;
	int bytesRecibidos;
	int j=0,contadorArchivos=0;

	//Estructura para pasarle al nodo
	t_job_a_nodo *el_job = malloc(sizeof(t_job_a_nodo));

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	// Contamos y separamos los archivos
	obtenerArrayArchivos(&contadorArchivos);

	// Muestro los archivos por pantalla
	printf("***********************\n");
	for(j=0;j<contadorArchivos;j++){
		printf("Archivo %d: %s\n",j,array_archivos[j]);
	}
	printf("Resultado: %s\n",array_archivos[contadorArchivos]);
	printf("***********************\n");

	// Creación de Buffer de archivos (2° ráfaga)
	bufferRafaga_Dos=procesarArchivos(bufferRafaga_Dos,contadorArchivos);
	
	// Creación de Buffer de datos del buffer (1° ráfaga)
	bufferRafaga_Uno=obtenerRafaga_Uno(bufferRafaga_Uno,bufferRafaga_Dos);
	
	// Muestro los buffers por pantalla
	//printf("=======> Buffer a Enviar a MaRTA RAFAGA 1=======> %s \n",bufferRafaga_Uno);
	//printf("=======> Buffer a Enviar a MaRTA RAFAGA 2=======> %s \n",bufferRafaga_Dos);

	// Conexión con proceso Marta
	conectarMarta();
	
	// Envío la primer ráfaga
	EnviarDatos(socket_Marta,bufferRafaga_Uno, strlen(bufferRafaga_Uno));
	//log_trace(logger, "ENVÍO DATOS. socket: %d. buffer: %s tamanio:%d", socket_Marta, bufferRafaga_Uno, strlen(bufferRafaga_Uno));
	cantidadRafagaMarta=2;

	// Escucho posibles entradas
	while ((!desconexionCliente) && g_Ejecutando) {
			//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

			// Buffers de envío y recepción
			if (buffer != NULL )
				free(buffer);
			buffer = string_new();
			bufferANodo=string_new();

			// Recibimos los datos del cliente
			buffer = RecibirDatos(socket_Marta,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
			//log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket_Marta, buffer, tamanio);

			// Control de recepción no vacía
			//printf("BytesRecibidos:%d\n",bytesRecibidos);
			if (bytesRecibidos>0) {

				//printf("--------El BUFFER:%s\n", buffer);
				// Si el flag es correcto y Marta devolvió el Ok
				if(cantidadRafagaMarta==3 && strcmp(buffer,"Ok")!=0){

					printf("Recibe Planificacion de Marta: %s\n",buffer);

					// Buffer para los nodos
					el_job->buffer=string_new();
					string_append(&el_job->buffer,buffer);

					// Aca hay que crear un nuevo hilo, que será el encargado de atender al nodo
					pthread_t hNuevoCliente;
					pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente,(void *) el_job);

					//pthread_join(hNuevoCliente,NULL);
					cantRafaga=3;


					//cantidadRafagaMarta=1;
				}
				else if(cantidadRafagaMarta==2){
					// Envío la segunda ráfaga
					EnviarDatos(socket_Marta,bufferRafaga_Dos, strlen(bufferRafaga_Dos));
					//log_trace(logger, "ENVÍO DATOS. socket: %d. buffer: %s tamanio:%d", socket_Marta, bufferRafaga_Dos, strlen(bufferRafaga_Dos));

					// Cambio de flags
					cantidadRafagaMarta=3;
					cantRafaga=3;

				}


			} else{
				desconexionCliente = 1;
				free(buffer);
			}
		}


	return 0;
}

int cuentaDigitos(int valor){
	int cont = 0;
	float tamDigArch=valor;
	while(tamDigArch>=1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	return cont;
}


int AtiendeCliente(void * arg) {
	t_job_a_nodo *el_job = (t_job_a_nodo*) arg;
	char *buff=el_job->buffer;
	char *aux=string_new();
	char * bufferANodo=string_new();
	int emisor=0;



	emisor=PosicionDeBufferAInt(buff,1);

	// 1: Aplicar Map ............... 2: Aplicar Reduce Local Con combiner............3: Aplicar Reduces Sin combiner
	if(emisor==1){
		aux=string_new();
		el_job=procesoJob(buff);
		string_append(&bufferANodo,"21");
		aux=abrir_Mapper(aux,g_Mapper);
		string_append(&bufferANodo,aux);
		free(aux);
		aux=obtenerSubBuffer(g_Mapper);
		string_append(&bufferANodo,aux);
		free(aux);
		aux = obtenerSubBuffer(el_job->bloque);
		string_append(&bufferANodo,aux);
		free(aux);
		aux=obtenerSubBuffer(el_job->archResultado);
		string_append(&bufferANodo,aux);
		free(aux);
	}else if (emisor==2){
		aux=string_new();
		el_job=procesoJob(buff);
		string_append(&bufferANodo,"21");
		aux=abrir_Mapper(aux,g_Reduce);
		string_append(&bufferANodo,aux);
		free(aux);
		aux=obtenerSubBuffer(g_Reduce);
		string_append(&bufferANodo,aux);
		free(aux);
		aux = obtenerSubBuffer(el_job->bloque);
		string_append(&bufferANodo,aux);
		free(aux);
		aux=obtenerSubBuffer(el_job->archResultado);
		string_append(&bufferANodo,aux);
		free(aux);
	}else if(emisor==3){
		// DE MARTA => 431215NodoB2202144resultado.txt2143resultado.txt2142resultado.txt2141resultado.txt2140resultado.txt21519resultado.txt21518resultado.txt21517resultado.txt21516resultado.txt21515resultado.txt21514resultado.txt21513resultado.txt21512resultado.txt21511resultado.txt21510resultado.txt2149resultado.txt2148resultado.txt2147resultado.txt2146resultado.txt2145resultado.txt15NodoB19127.0.0.1146001
		//			       15NodoC1621525resultado.txt21524resultado.txt21523resultado.txt21522resultado.txt21521resultado.txt21520resultado.txt15NodoC19127.0.0.1146002
		//				230/user/juan/datos/resultado.txt
		//
		//
		//
		//
		// A NODO => 2311 15NodoA  13 18Bloque30     18Bloque38    18Bloque43      15NodoA212192.168.1.27146000   230/user/juan/datos/resultado.txt
		int cantNodos=0,cantDigNodos=0,cantBloques=0,cantDigBloques=0,posicion=0,x=0,y=0;


		string_append(&bufferANodo,"23");
		cantDigNodos=PosicionDeBufferAInt(buff,2);
		cantNodos=ObtenerTamanio(buff,3,cantDigNodos);
		posicion=3+cantDigNodos;


		string_append(&bufferANodo,string_itoa(cantDigNodos));
		string_append(&bufferANodo,string_itoa(cantNodos));
		for(y=0;y<cantNodos;y++){

			aux=string_new();
			aux=DigitosNombreArchivo(buff,&posicion);
			string_append(&bufferANodo,obtenerSubBuffer(aux));
			aux=string_new();

			cantDigBloques=PosicionDeBufferAInt(buff,posicion);
			cantBloques=ObtenerTamanio(buff,posicion+1,cantDigBloques);
			posicion=posicion+1+cantDigBloques;


			string_append(&bufferANodo,string_itoa(cantDigBloques));
			string_append(&bufferANodo,string_itoa(cantBloques));


			for(x=0;x<cantBloques;x++){
				aux=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(aux));
				aux=string_new();
			}

			if(y==0){
				el_job->nodo=string_new();
				el_job->ip=string_new();
				el_job->puerto=string_new();
				el_job->nodo=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(el_job->nodo));
				el_job->ip=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(el_job->ip));
				el_job->puerto=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(el_job->puerto));
			}else{
				aux=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(aux));
				aux=string_new();
				aux=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(aux));
				aux=string_new();
				aux=DigitosNombreArchivo(buff,&posicion);
				string_append(&bufferANodo,obtenerSubBuffer(aux));
				aux=string_new();
			}

		}
		//aux=string_new();
		aux=DigitosNombreArchivo(buff,&posicion);
		el_job->archResultado=aux;
		string_append(&bufferANodo,obtenerSubBuffer(aux));
		aux = string_new();
		aux = abrir_Reduce(aux,g_Reduce);
		string_append(&bufferANodo,aux);
		string_append(&bufferANodo,obtenerSubBuffer(g_Reduce));


/*
		printf(COLOR_VERDE"%s \t %s \t%s \n"DEFAULT,el_job->nodo,el_job->ip,el_job->puerto);
		printf(COLOR_VERDE" %s \n"DEFAULT,bufferANodo);
*/

	}


	int socket_nodo=0;
	char * bufferEnvia = string_new();
	char * bufferR = string_new();
	int bytesRecibidos, cantRafaga=1, tamanio=10;
	//Me conecto con el Nodo
	socket_nodo=conectarNodo(*el_job,socket_nodo);

	//Primera Rafaga
	string_append(&bufferEnvia,"2");
	string_append(&bufferEnvia,string_itoa(cuentaDigitos(strlen(bufferANodo))));
	string_append(&bufferEnvia,string_itoa(strlen(bufferANodo)));
	//printf("BUFFER ENVIA:%s\n",bufferEnvia);
	sem_wait(&semaforoNodo);
	EnviarDatos(socket_nodo,bufferEnvia, strlen(bufferEnvia));
	sem_post(&semaforoNodo);
	//log_trace(logger, "ENVÍO DATOS. socket: %d. buffer: %s tamanio:%d", socket_nodo, bufferEnvia, strlen(bufferEnvia));
//	sem_wait(&semaforoNodo);
	bufferR = RecibirDatos(socket_nodo, bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
	//log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket_nodo, bufferR, strlen(bufferR));
//	sem_post(&semaforoNodo);
	//Le envio el buffer al Nodo
	printf(COLOR_VERDE"TAMAÑO BUFFER A NODO:%d\n",strlen(bufferANodo));
	sem_wait(&semaforoNodo);
	EnviarDatos(socket_nodo,bufferANodo, strlen(bufferANodo));
	sem_post(&semaforoNodo);
	printf(COLOR_VERDE"TAMAÑO BUFFER A NODO:%d\n",strlen(bufferANodo));
	//log_trace(logger, "ENVÍO DATOS. socket: %d. buffer: %s tamanio:%d", socket_nodo, bufferANodo, strlen(bufferANodo));

	int code=0;

	// Dentro del buffer se guarda el mensaje recibido por el nodo.
	char* buffer;
	buffer = malloc(BUFFERSIZE * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.


	// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

	char *bufferAMartaUno=string_new();
	char *bufferAMartaDos=string_new();



	while ((!desconexionCliente) && g_Ejecutando) {

		if (buffer != NULL )
			free(buffer);
		buffer = string_new();

		//Recibimos los datos del nodo
//		sem_wait(&semaforoNodo);
		buffer = RecibirDatos(socket_nodo, buffer, &bytesRecibidos,&cantRafaga,&tamanio);
//		sem_post(&semaforoNodo);
		//log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket_nodo, buffer, tamanio);


		if (bytesRecibidos > 0) {

			if(strcmp(buffer,"31")==0){
				//Envio el a MaRTA 2518Bloque3015NODOA    El 3 es de que hizo bien la tarea.

				if(emisor==3){
					//24234/users/dasdas/resultado.txt
					printf("SE HICERON TODOS LOS REDUCES==>"COLOR_VERDE" Archivo final: %s\n"DEFAULT,el_job->archResultado);

					string_append(&bufferAMartaDos,"24");
					string_append(&bufferAMartaDos,obtenerSubBuffer(el_job->archResultado));

					string_append(&bufferAMartaUno,"2");
					string_append(&bufferAMartaUno,string_itoa(cuentaDigitos(strlen(bufferAMartaDos))));
					string_append(&bufferAMartaUno,string_itoa(strlen(bufferAMartaDos)));


					//HAY QUE VER BIEN PORQUE NO ANDA CON EL RECIBIR EN EL MEDIO

					//RAFAGA 1
					//printf("---bufferAMartaUno : %s\n",bufferAMartaUno);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaUno, strlen(bufferAMartaUno));
					sem_post(&semaforoMarta);


					//RAFAGA 2
					//printf("---bufferAMartaDos : %s\n",bufferAMartaDos);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaDos, strlen(bufferAMartaDos));
					sem_post(&semaforoMarta);

					bufferAMartaDos=string_new();
					bufferAMartaUno=string_new();

				}else if (emisor==1){

					printf(COLOR_VERDE"RECIBO OK DEL NODO (map): %s--%s\n"DEFAULT,el_job->bloque,el_job->nodo);
					string_append(&bufferAMartaDos,"23");
					string_append(&bufferAMartaDos,obtenerSubBuffer(el_job->bloque));
					string_append(&bufferAMartaDos,obtenerSubBuffer(el_job->nodo));

					string_append(&bufferAMartaUno,"2");
					string_append(&bufferAMartaUno,string_itoa(cuentaDigitos(strlen(bufferAMartaDos))));
					string_append(&bufferAMartaUno,string_itoa(strlen(bufferAMartaDos)));


					//HAY QUE VER BIEN PORQUE NO ANDA CON EL RECIBIR EN EL MEDIO

					//RAFAGA 1
					//printf("---bufferAMartaUno : %s\n",bufferAMartaUno);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaUno, strlen(bufferAMartaUno));
					sem_post(&semaforoMarta);

					//RAFAGA 2
					//printf("---bufferAMartaDos : %s\n",bufferAMartaDos);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaDos, strlen(bufferAMartaDos));
					sem_post(&semaforoMarta);
					/*buffer=string_new();
					recv(socket_Marta, buffer, 10, 0);
					while(strcmp(buffer,"Ok")!=0){
						EnviarDatos(socket_Marta,bufferAMartaDos, strlen(bufferAMartaDos));
						buffer=string_new();
						recv(socket_Marta, buffer, 10, 0);
					}*/

					bufferAMartaDos=string_new();
					bufferAMartaUno=string_new();

				}else if(emisor == 2){

					printf(COLOR_VERDE"RECIBO OK DEL NODO (reduce): %s--%s\n"DEFAULT,el_job->bloque,el_job->nodo);
					string_append(&bufferAMartaDos,"23");
					string_append(&bufferAMartaDos,obtenerSubBuffer(el_job->bloque));
					string_append(&bufferAMartaDos,obtenerSubBuffer(el_job->nodo));

					string_append(&bufferAMartaUno,"2");
					string_append(&bufferAMartaUno,string_itoa(cuentaDigitos(strlen(bufferAMartaDos))));
					string_append(&bufferAMartaUno,string_itoa(strlen(bufferAMartaDos)));


					//HAY QUE VER BIEN PORQUE NO ANDA CON EL RECIBIR EN EL MEDIO

					//RAFAGA 1
					//printf("---bufferAMartaUno : %s\n",bufferAMartaUno);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaUno, strlen(bufferAMartaUno));
					sem_post(&semaforoMarta);


					//RAFAGA 2
					//printf("---bufferAMartaDos : %s\n",bufferAMartaDos);
					sem_wait(&semaforoMarta);
					EnviarDatos(socket_Marta,bufferAMartaDos, strlen(bufferAMartaDos));
					sem_post(&semaforoMarta);
				}
				bufferAMartaUno=string_new();
				bufferAMartaDos=string_new();
				buffer=string_new();

				//pthread_exit(NULL);

			}else if (strcmp(buffer,"30")==0){

			}


		} else{
			desconexionCliente = 1;
			free(buffer);
			free(bufferANodo);
		}

	}

	CerrarSocket(socket_nodo);

	return code;
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
}


int conectarNodo(t_job_a_nodo el_job,int socket_nodo){


	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON UN NODO
	log_info(logger, "Intentando conectar a %s\n",el_job.nodo);

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(el_job.ip, el_job.puerto, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_error(logger,
				"Error de carga de datos de conexion en socket_nodo");
	}

	if ((socket_nodo = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_error(logger, "Error al crear socket_nodo");
	}
	if (connect(socket_nodo, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_error(logger, "Error al conectar con socket_nodo");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return socket_nodo;
}

t_job_a_nodo *procesoJob (char *buffer){

	//41 15 NodoA   19 127.0.0.1    14 6000      18 Bloque30      213 resultado.txt
	//42

	t_job_a_nodo *el_job = malloc(sizeof(t_job_a_nodo));
	int pos=2;
	//printf("BUFFER:%s\n",buffer);

	el_job->nodo=DigitosNombreArchivo(buffer,&pos);
	//printf("Nodo:%s\n",el_job->nodo);
	el_job->ip=DigitosNombreArchivo(buffer,&pos);
	//printf("IP:%s\n",el_job->ip);
	el_job->puerto=DigitosNombreArchivo(buffer,&pos);
	//printf("Puerto:%s\n",el_job->puerto);
	el_job->bloque=DigitosNombreArchivo(buffer,&pos);
	//printf("Bloque:%s\n",el_job->bloque);
	el_job->archResultado=DigitosNombreArchivo(buffer,&pos);
	//printf("Resultado:%s\n",el_job->archResultado);
	return el_job;
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

char* abrir_Mapper(char *aux, char *nombreScript){

	FILE *f;
	int tamanioArchivo=0;
	float tam=0;
	int cont=0;
	char *aux2=string_new();

	f= fopen(nombreScript,"rb");
	fseek(f,0,SEEK_END);
	tamanioArchivo=ftell(f);
	rewind(f);
	aux2=malloc(tamanioArchivo+1);
	memset(aux2, 0, tamanioArchivo+1);
	fread(aux2,1,tamanioArchivo,f);

	tam=tamanioArchivo;
	while(tam>=1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioArchivo));
	string_append(&aux,aux2);

	free(aux2);
	fclose(f);

	return aux;
}

char* abrir_Reduce(char *aux, char *nombreScript){

	FILE *f;
	int tamanioArchivo=0;
	float tam=0;
	int cont=0;
	char *aux2=string_new();

	f= fopen(nombreScript,"rb");
	fseek(f,0,SEEK_END);
	tamanioArchivo=ftell(f);
	rewind(f);
	aux2=malloc(tamanioArchivo+1);
	memset(aux2, 0, tamanioArchivo+1);
	fread(aux2,1,tamanioArchivo,f);

	tam=tamanioArchivo;
	while(tam>=1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioArchivo));
	string_append(&aux,aux2);

	free(aux2);
	fclose(f);

	return aux;
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


int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return PosicionDeBufferAInt(buffer, 0);
}


char* DigitosNombreArchivo(char *buffer,int *posicion){

	char *nombreArch = string_new();
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

char* obtenerRafaga_Uno(char *buffer,char* bufferAux){
	int cont=0;
	float tamanio=0;
	tamanio=strlen(bufferAux);
	string_append(&buffer,"2");
	while(tamanio>1){
		tamanio=tamanio/10;
		cont++;
	}
	string_append(&buffer,string_itoa(cont));
	string_append(&buffer,string_itoa(strlen(bufferAux)+5));//Le agrego 5 bytes mas por las dudas POR AHORA

	return buffer;
}

char * procesarArchivos (char *bufferArch,int contArch){
	//2271
	//212220temperatura-2012.txt220temperatura-2013.txt213resultado.txt1 TAMANIO 66 + los 5 que le agrego por las dudas

	int j=0;
	int contDig=0,cont=0,combiner=9;
	float tam=0,tamDigArch=0;

	tamDigArch=contArch;
	while(tamDigArch>=1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	string_append(&bufferArch,"2");
	string_append(&bufferArch,"1");
	string_append(&bufferArch,string_itoa(cont));
	string_append(&bufferArch,string_itoa(contArch));

	for(j=0;j<=contArch;j++){
		tam=strlen(array_archivos[j]);
		contDig=0;
		while(tam>=1){
			tam=tam/10;
			contDig++;
		}
		string_append(&bufferArch,string_itoa(contDig));
		string_append(&bufferArch,string_itoa(strlen(array_archivos[j])));
		string_append(&bufferArch,array_archivos[j]);
	}
	if(strcmp(g_Combiner,"SI")==0)
		combiner=1;
	else
		combiner=0;

	string_append(&bufferArch,string_itoa(combiner));
	string_append(&bufferArch,"\0");

	return bufferArch;

}
/*
  char* obtenerNombreResultado(){
	char **array;
	int cont=0;
	array =(char**) malloc (strlen(g_Resultado));
	array=string_split(g_Resultado,"/");
	while (array[cont]!=NULL){
				cont++;
	}
	free(array);
	return array[cont-1];
}
 */

void obtenerArrayArchivos(int *contadorArchivos){

	char **array;
	int cont=0,j=0,i=0,c=0,index=0;
	char *aux=string_new();

	array =(char**) malloc (strlen(g_Archivos));
	array=string_split(g_Archivos,"/");

	while (array[cont]!=NULL){
		for(j=0;j<strlen(array[cont]);j++){
			if(array[cont][j]=='.'){
				*contadorArchivos=*contadorArchivos+1;
			}
		}
		cont++;
	}


		array_archivos=(char**) malloc (*contadorArchivos*strlen(g_Archivos)+strlen(g_Resultado));

		cont=0;
		i=0;
		while (array[cont]!=NULL){
				for(j=0;j<strlen(array[cont]);j++){
					if(array[cont][j]=='.'){
							for(c=index;c<=cont;c++){
								string_append(&aux,"/");
								string_append(&aux,array[c]);

							}
							index=cont+1;
							array_archivos[i]=aux;
							aux=string_new();
							i++;
					}
				}
				cont++;
		}
		array_archivos[*contadorArchivos]=g_Resultado;
		free(aux);
		free(array);


}



int ObtenerTamanio (char *buffer , int posicion, int dig_tamanio){
	int x,digito,aux=0;
	for(x=0;x<dig_tamanio;x++){
		digito=PosicionDeBufferAInt(buffer,posicion+x);
		aux=aux*10+digito;
	}
	return aux;
}

char* RecibirDatos(int socket,char *buffer, int *bytesRecibidos,int *cantRafaga,int *tamanio) {
	*bytesRecibidos = 0;
	char *bufferAux= malloc(1);
	int digTamanio;
	if (buffer != NULL ) {
		free(buffer);
	}

	if(*cantRafaga==1){
		bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char));
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde Marta. Socket: %d",socket);
		}

		digTamanio=PosicionDeBufferAInt(bufferAux,1);
		*tamanio=ObtenerTamanio(bufferAux,2,digTamanio);

	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char));
		memset(bufferAux, 0, *tamanio * sizeof(char)); //-> llenamos el bufferAux con ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}

	}else{

		if(*cantRafaga==3){

			bufferAux = realloc(bufferAux,10000* sizeof(char));
			memset(bufferAux, 0, 10000 * sizeof(char)); //-> llenamos el bufferAux con ceros.

			if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, 10000, 0)) == -1) {
				Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
			}

		}
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket,(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo.
}


int EnviarDatos(int socket,char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	//printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a los clientes. Socket: %d", socket);

	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,(char*) buffer);

	return bytecount;
}


void conectarMarta() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON MARTA
	log_info(logger, "Intentando conectar a Marta\n");
	//char * puerto = "7000";
	
	struct addrinfo infoLocal;
	struct addrinfo *infoMarta;

	memset(&infoLocal, 0, sizeof(infoLocal));
	infoLocal.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	infoLocal.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(g_Ip_Marta, g_Puerto_Marta, &infoLocal, &infoMarta) != 0) {// Carga en serverInfo los datos de la conexion
		log_error(logger,
				"Error de carga de datos de conexion en socket_Marta");
	}

	if ((socket_Marta = socket(infoMarta->ai_family, infoMarta->ai_socktype,
			infoMarta->ai_protocol)) < 0) {
		log_error(logger, "Error en la creación del socket_Marta");
	}
	if (connect(socket_Marta, infoMarta->ai_addr, infoMarta->ai_addrlen)
			< 0) {
		log_error(logger, "Error al conectar con socket_Marta");
	}
	freeaddrinfo(infoMarta);	// No lo necesitamos mas

}



#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Obtenemos la ip de Marta
		if (config_has_property(config, "IP_MARTA")) {
			g_Ip_Marta = config_get_string_value(config, "IP_MARTA");
		} else
			Error("No se pudo leer el parametro IP_MARTA");

		// Obtenemos el puerto de escucha de Marta
		if (config_has_property(config, "PUERTO_MARTA")) {
			g_Puerto_Marta = config_get_string_value(config,"PUERTO_MARTA");
		} else
			Error("No se pudo leer el parametro PUERTO_MARTA");

		// Programa rutina de mapper
		if (config_has_property(config, "MAPPER")) {
			g_Mapper = config_get_string_value(config, "MAPPER");
		} else
			Error("No se pudo leer el parametro MAPPER");

		// Programa rutina de reduce
		if (config_has_property(config, "REDUCE")) {
			g_Reduce = config_get_string_value(config, "REDUCE");
		} else
			Error("No se pudo leer el parametro REDUCE");

		// Es combiner
		if (config_has_property(config, "COMBINER")) {
			g_Combiner = config_get_string_value(config, "COMBINER");
		} else
			Error("No se pudo leer el parametro COMBINER");

		// Archivos sobre los que se aplica el Job
		if (config_has_property(config, "ARCHIVOS")) {
			g_Archivos = config_get_string_value(config, "ARCHIVOS");
		} else
			Error("No se pudo leer el parametro ARCHIVOS");

		// Archivo de resultados
		if (config_has_property(config, "RESULTADO")) {
			g_Resultado = config_get_string_value(config, "RESULTADO");
		} else
			Error("No se pudo leer el parametro RESULTADO");

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
