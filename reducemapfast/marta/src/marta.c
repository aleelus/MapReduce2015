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

	sem_init(&semaforoListaNodos, 0, 1);
	sem_init(&semaforoListaArchivos, 0, 1);
	sem_init(&semaforoListaJobEnviados, 0, 1);
	sem_init(&semSocket,0,1);
	sem_init(&semIdJob, 0, 1);

	lista_nodos = list_create();

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
		if (config_has_property(config, "IP_FS")) {
			g_Ip_Fs = config_get_string_value(config, "IP_FS");
		} else
			Error("No se pudo leer el parametro IP_FS");
		if (config_has_property(config, "PUERTO_FS")) {
			g_Puerto_Fs = config_get_string_value(config, "PUERTO_FS");
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
	memset(bufferAux,0,1);
	int digTamanio=0;
	if (buffer != NULL ) {
		free(buffer);
	}

	if(*cantRafaga==1){
		bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char)+1);
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}

		if(strcmp(bufferAux,"Ok")!=0){
			digTamanio=PosicionDeBufferAInt(bufferAux,1);
			*tamanio=ObtenerTamanio(bufferAux,2,digTamanio);
		}

	}else if(*cantRafaga==2 && *tamanio>0){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char)+1);
		memset(bufferAux, 0, *tamanio * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}
	}else{

		if(*cantRafaga==3){

			bufferAux = realloc(bufferAux,500* sizeof(char));
			memset(bufferAux, 0, 500 * sizeof(char)); //-> llenamos el bufferAux con ceros.

			if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, 500, 0)) == -1) {
				Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
			}
			//*cantRafaga=1;
		}
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket,(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}

int EnviarDatos(int socket, char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	//printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a al clientes. Socket: %d", socket);

	//Traza("ENVIO datos. socket: %d. buffer: %s", socket, (char*) buffer);

	//char * bufferLogueo = malloc(5);
	//bufferLogueo[cantidadDeBytesAEnviar] = '\0';

	//memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,(char*) buffer);

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

void CargarArchivoALista(char* nomArchivo, int id,t_list* lista_archivos){

	list_add(lista_archivos,archivo_create(nomArchivo,id));

}
void AtiendeJob (int * contIdJob,char *buffer, int *cantRafaga,t_list** lista_archivos){

	char *nArchivo,*nResultado;
	int digitosCantDeArchivos=0,cantDeArchivos=0;
	int x,posActual=0;
	int tieneCombiner;
	t_archivo *el_archivo;
	sem_wait(&semIdJob);
	*contIdJob=id_job;
	id_job++;
	sem_post(&semIdJob);

	//BUFFER RECIBIDO = 2270 (EJEMPLO)
	//BUFFER RECIBIDO = 2112220temperatura-2012.txt220temperatura-2013.txt213resultado.txt1
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 211

	digitosCantDeArchivos=PosicionDeBufferAInt(buffer,2);
	//printf("CANTIDAD DE DIGITOS:%d\n",digitosCantDeArchivos);
	cantDeArchivos=ObtenerTamanio(buffer,3,digitosCantDeArchivos);
	//printf("Cantidad de Archivos: %d\n",cantDeArchivos);
	posActual=3+digitosCantDeArchivos;

	for(x=0;x<cantDeArchivos;x++){
		nArchivo=DigitosNombreArchivo(buffer,&posActual);
		printf("NOMBRE:%s\n",nArchivo);
		CargarArchivoALista(nArchivo,*contIdJob,*lista_archivos);
	}
	nResultado=DigitosNombreArchivo(buffer,&posActual);
	tieneCombiner=PosicionDeBufferAInt(buffer,posActual); // CAMBIE strlen(buffer)-3 por posActual

	int i=0;
	while(i<list_size(*lista_archivos)){

		el_archivo = list_get(*lista_archivos, i);

		if (el_archivo->idJob == *contIdJob) {
			el_archivo->nombreArchivoResultado=nResultado;
			el_archivo->tieneCombiner=tieneCombiner;
		}
		i++;
	}
	*cantRafaga=1;
}

void RecorrerArchivos(t_list* lista_archivos){
	t_archivo * el_archivo;

	int i=0;
	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		printf("El id del Job:"COLOR_VERDE "%d\n"DEFAULT,el_archivo->idJob);
		printf("El archivo:"  COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);
		printf("El archivo de resultado:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivoResultado);
		printf("Tiene Combiner:"COLOR_VERDE "%d\n"DEFAULT,el_archivo->tieneCombiner);
		i++;
	}
}

void RecorrerArrayListas(t_archivo *el_archivo){
	t_dato * el_dato;

	int i,j;
	printf("Nombre Archivo: %s\n",el_archivo->nombreArchivo);
	printf("Cantidad de Bloques:%d\n",list_size(el_archivo->listaBloques));
	for(i=0;i<list_size(el_archivo->listaBloques)*3;i++){
		j=0;
		while(j<list_size(el_archivo->array_de_listas[i])){
			el_dato = list_get(el_archivo->array_de_listas[i],j);
			if(j==0){
				printf("%s ::%d:: ",el_dato->dato,el_dato->peso);
			} else {
				printf("%s--",el_dato->dato);
			}
			j++;
			if(j==list_size(el_archivo->array_de_listas[i]))
				printf("\n");
		}

	}
}


void RecorrerListaBloques(t_list* lista_archivos){
	t_archivo * el_archivo;
	t_bloque * el_bloque;

	int i=0;
	int j=0;
	printf("IDJOB:"COLOR_VERDE"%d\n"DEFAULT,id_job);

	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		printf("El id del Job:"COLOR_VERDE"%d\n"DEFAULT,el_archivo->idJob);
		printf("El archivo:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);

		while(j<list_size(el_archivo->listaBloques)){
			el_bloque = list_get(el_archivo->listaBloques, j);
			printf("%s :: ",el_bloque->bloque);
			//printf("Copia1:\n");
			printf("%s--",el_bloque->array[0].nodo);
			printf("%s  ",el_bloque->array[0].bloque);
			//printf("Copia2:\n");
			printf("%s--",el_bloque->array[1].nodo);
			printf("%s  ",el_bloque->array[1].bloque);
			//printf("Copia3:\n");
			printf("%s--",el_bloque->array[2].nodo);
			printf("%s  \n",el_bloque->array[2].bloque);
			j++;
		}
		j=0;
		i++;
	}

}

void RecorrerListaNodos(){

	t_nodo *el_nodo;
	t_bloqueArchivo *el_bloqueArchivo;
	int i=0,j=0;

	while(i<list_size(lista_nodos)){
		el_nodo=list_get(lista_nodos,i);
		printf("************************************************\n");
		printf("Nombre: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->nombreNodo);
		printf("IP: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->ipNodo);
		printf("Puerto: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->puertoNodo);
		printf("ListaBloqueArchivo: ");
		j=0;
		while(j<list_size(el_nodo->listaBloqueArchivo)){
			el_bloqueArchivo=list_get(el_nodo->listaBloqueArchivo,j);
			printf(COLOR_VERDE"  %s"DEFAULT"::::"COLOR_VERDE"%s"DEFAULT,el_bloqueArchivo->bloque,el_bloqueArchivo->archivo);
			j++;
			if(j==list_size(el_nodo->listaBloqueArchivo))
				printf("\n");

		}
		i++;
	}
	printf("************************************************\n");

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

int cuentaDigitos(int valor){
	int cont = 0;
	float tamDigArch=valor;
	while(tamDigArch>=1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	return cont;
}

void conectarAFileSystem() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON UN NODO
		log_info(logger, "Intentando conectar a FS\n");

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


		if (getaddrinfo(g_Ip_Fs, g_Puerto_Fs, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
			log_info(logger,
					"ERROR: cargando datos de conexion socket_fs");
		}
		sem_wait(&semaforoListaArchivos);
		if ((socket_fs = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol)) < 0) {
			log_info(logger, "ERROR: crear socket_fs");
		}
		if (connect(socket_fs, serverInfo->ai_addr, serverInfo->ai_addrlen)
				< 0) {
			log_info(logger, "ERROR: conectar socket_fs");
		}
		freeaddrinfo(serverInfo);	// No lo necesitamos mas
		sem_post(&semaforoListaArchivos);
}


void ObtenerInfoDeNodos(int id,t_list** lista_archivos,t_list*** array_listas){


	//BUFFER RECIBIDO = 4284 (EJEMPLO)
	//BUFFER RECIBIDO = 4112237/user/juan/datos/temperatura-2012.txt237/user/juan/datos/temperatura-2013.txt

	t_archivo * el_archivo;
	//t_bloque * el_bloque;
	char *bufferUno=string_new();
	char *bufferDos=string_new();

	//Se carga la informacion de los bloques del archivo
	int i=0,contArchivos=0;
	//Para el FS
	string_append(&bufferDos,"41");
	while(i<list_size(*lista_archivos)){
		el_archivo = list_get(*lista_archivos, i);
		if (el_archivo->idJob == id) {
			contArchivos++;
		}
		i++;
	}

	//cuento los digitos
	float tam=contArchivos;
	int cont=0;
	while(tam>=1){
			tam=tam/10;
			cont++;
	}
	if(cont==0)
		cont=1;

	string_append(&bufferDos,string_itoa(cont));
	string_append(&bufferDos,string_itoa(contArchivos));
	i=0;
	while(i<list_size(*lista_archivos)){
			el_archivo = list_get(*lista_archivos, i);
			if (el_archivo->idJob == id) {
				cont=0;
				tam=strlen(el_archivo->nombreArchivo);
				while(tam>=1){
					tam=tam/10;
					cont++;
				}
				string_append(&bufferDos,string_itoa(cont));
				string_append(&bufferDos,string_itoa(strlen(el_archivo->nombreArchivo)));
				string_append(&bufferDos,el_archivo->nombreArchivo);

			}
			i++;
	}
	tam=strlen(bufferDos);
	cont=0;
	string_append(&bufferUno,"4");
	while(tam>=1){
		tam=tam/10;
		cont++;
	}
	string_append(&bufferUno,string_itoa(cont));
	string_append(&bufferUno,string_itoa(strlen(bufferDos)));

/*
	printf("****** BUFFER'S A ENVIAR: \n");
	printf("****** %s\n",bufferUno);
	printf("****** %s\n",bufferDos);
*/
	//Me conecto con el FS
	conectarAFileSystem();

	int bytesRecibidos=0;
	int tamanio=10,cantRafaga=1;

	char *buffer=string_new();
	char *rafaga2Fs=string_new();

	//Envio Rafaga Uno a FS
	sem_wait(&semSocket);
	EnviarDatos(socket_fs, bufferUno,strlen(bufferUno));
	sem_post(&semSocket);
	//Recibo el Ok
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	cantRafaga=1;

	//Envio la Segunda Rafaga
	sem_wait(&semSocket);
	EnviarDatos(socket_fs, bufferDos,strlen(bufferDos));
	sem_post(&semSocket);

	//Recibo la primera rafaga de FS
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	sem_wait(&semSocket);
	EnviarDatos(socket_fs, "Ok",strlen("Ok"));
	sem_post(&semSocket);

	cantRafaga=2;
	//Recibo la segunda rafaga de Fs
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	rafaga2Fs= buffer;
	printf("RAFAGA2 DE FS:%s\n",rafaga2Fs);


	//BUFFER RECIBIDO = 1212237/user/juan/datos/temperatura-2012.txt1215NODOA18Bloque3015NODOB18Bloque3715NODOF17Bloque8
		//											 				  15NODOE18Bloque1315NODOA18Bloque3815NODOC17Bloque7
		//			        237/user/juan/datos/temperatura-2013.txt1215NODOP18Bloque3115NODOF18Bloque4215NODOH17Bloque9
		//										     			      15NODOK18Bloque1115NODOB18Bloque5515NODOF17Bloque3
		//				  1815NODOA19127.0.0.114600015NODOB19127.0.0.1146000
	//						15NODOC19127.0.0.114600015NODOE19127.0.0.1146000
	//						15NODOF19127.0.0.114600015NODOH19127.0.0.1146000
	//						15NODOK19127.0.0.114600015NODOP19127.0.0.1146000



	//Bloque0 :: NODOA:Bloque30 NODOB:Bloque37 NODOF:Bloque8
	//Bloque1 :: NODOE:Bloque13 NODOA:Bloque38 NODOC:Bloque7

	//Bloque0 :: NODOP:Bloque21 NODOF:Bloque42 NODOH:Bloque9
	//Bloque1 :: NODOK:Bloque11 NODOB:Bloque55 NODOF:Bloque3



	int x=0,cantArch=0,digCantArch=0,posActual=0,j=0,c=0;
	int digCantBloq=0,cantBloq=0;
	char *nomArch;
	char *bloque=string_new();
	char *nodo=string_new();
	char *ip=string_new();
	char *puerto=string_new();
	char *aux=string_new();
	int contador=0;
	int cantNodo=0, digCantNodo=0;




	t_bloque *el_bloque= malloc(sizeof(t_bloque));


	string_append(&aux,"Bloque");

	digCantArch=PosicionDeBufferAInt(rafaga2Fs,2);
	cantArch=ObtenerTamanio(rafaga2Fs,3,digCantArch);
	posActual=3+digCantArch;


	char **array;
	int con=0;


	for(x=0;x<cantArch;x++){
		j=0;
		nomArch=DigitosNombreArchivo(rafaga2Fs,&posActual);

		while(j<list_size(*lista_archivos)){
			el_archivo=list_get(*lista_archivos,j);

			array = string_split(el_archivo->nombreArchivo,"/");
			cont=0;
			 while(array[con]!=NULL){
			    	  con++;
			 }

			if(el_archivo->idJob==id && strcmp(array[con-1],nomArch)==0){

				j=list_size(*lista_archivos);

			}
			j++;
		}

		digCantBloq=PosicionDeBufferAInt(rafaga2Fs,posActual);
		cantBloq=ObtenerTamanio(rafaga2Fs,posActual+1,digCantBloq);
		posActual=posActual+1+digCantBloq;
		for(c=0;c<cantBloq;c++){
			for(j=0;j<3;j++){
				nodo=DigitosNombreArchivo(rafaga2Fs,&posActual);
				bloque=DigitosNombreArchivo(rafaga2Fs,&posActual);
				el_bloque->array[j].nodo=nodo;
				el_bloque->array[j].bloque=bloque;
				el_bloque->array[j].estado=0;
			}

			string_append(&aux,string_itoa(contador));
			contador++;
			list_add(el_archivo->listaBloques,bloque_create(aux,el_bloque->array));

			aux=string_new();
			string_append(&aux,"Bloque");

		}
		contador=0;
		c=0;

	}


	i=0;
	while(i<list_size(*lista_archivos)){
		el_archivo = list_get(*lista_archivos, i);
		if(el_archivo->idJob == id){
			FuncionMagica(el_archivo->listaBloques,array_listas);//cargo array_listas
			el_archivo->array_de_listas=*array_listas;
			//RecorrerArrayListas(el_archivo);
		}
		i++;
	}

	//Cantidad de Nodos
	digCantNodo=PosicionDeBufferAInt(rafaga2Fs,posActual);
	cantNodo=ObtenerTamanio(rafaga2Fs,posActual+1,digCantNodo);
	posActual=posActual+1+digCantNodo;
	t_nodo *el_nodo=NULL;

	for(c=0;c<cantNodo;c++){
		nodo=DigitosNombreArchivo(rafaga2Fs,&posActual);
		sem_wait(&semaforoListaNodos);
		for(j=0;j<list_size(lista_nodos);j++){
			el_nodo=list_get(*lista_archivos,j);
			bool _true(void *elem){
				return ( !strcmp(((t_nodo*) elem)->nombreNodo,nodo) );
			}

			el_nodo = list_find(lista_nodos, _true);
			if(el_nodo != NULL){
				j=list_size(lista_nodos);
			}
		}
		sem_post(&semaforoListaNodos);
		if(el_nodo != NULL){

			//No hago nada por ahora

		}else{

			ip=DigitosNombreArchivo(rafaga2Fs,&posActual);
			puerto=DigitosNombreArchivo(rafaga2Fs,&posActual);
			sem_wait(&semaforoListaNodos);
			list_add(lista_nodos,nodo_create(nodo,ip,puerto));
			sem_post(&semaforoListaNodos);
		}
	}




	CerrarSocket(socket_fs);

}

void FuncionMagica(t_list* listaBloques,t_list***array_listas){

	t_bloque *el_bloque;
	t_dato *el_dato;

	int cantidadBloques;
	int posicion=0;
	int i,j,k;
	cantidadBloques = list_size(listaBloques);

	*array_listas =(t_list**) malloc (cantidadBloques*3*sizeof(t_dato));

	for(i=0;i<cantidadBloques*3;i++){
		(*array_listas)[i] = list_create();
	}

	for(i=0;i<cantidadBloques;i++){//recorro los bloques del archivo

		el_bloque = list_get(listaBloques,i);

		for(j=0;j<3;j++){          //recorro las 3 copias del bloque del archivo

			for(k=0;k<cantidadBloques*3;k++){        //recorre el array de la lista de nodos y bloques

				bool _true(void *elem) {
					return ( !strcmp(((t_dato*) elem)->dato,el_bloque->array[j].nodo) );
				}
				//buscar si el nodo de la copia existe en una lista de array_lista

				el_dato = list_find((*array_listas)[k], _true);

				if(el_dato != NULL){
					posicion = k;
					k=cantidadBloques*3;// salgo del for (no se si deberia usar exit o break)
				}

			}

			if(el_dato != NULL){
				list_add((*array_listas)[posicion],dato_create(el_bloque->bloque,el_bloque->array[j].bloque));
			}else{

				int h = 0;
				while((*array_listas)[h]->head != NULL){
					h++;
				}

				list_add((*array_listas)[h],dato_create(el_bloque->array[j].nodo,""));
				list_add((*array_listas)[h],dato_create(el_bloque->bloque,el_bloque->array[j].bloque));

			}
		}
	}
}


int obtenerPesoMax(t_list* lista_archivos){



	t_archivo *el_archivo;

	int cantNodo=0,total=0,cantBloquesPor3=0;
	int i=0;

	while(i<list_size(lista_archivos)){
		el_archivo=list_get(lista_archivos,i);
		cantBloquesPor3= cantBloquesPor3 + list_size(el_archivo->listaBloques)*3;
		i++;
	}
	cantNodo=list_size(lista_nodos);
	total=cantNodo*cantBloquesPor3;

	return total;

}

t_nodo* buscarNodo(char * nodo){

	t_nodo *el_nodo;
	int i;


	i=0;
	while(i<list_size(lista_nodos)){
		el_nodo=list_get(lista_nodos,i);
		if(strcmp(el_nodo->nombreNodo,nodo)==0){
			return el_nodo;
		}

		i++;
	}
	return NULL;

}

void Planificar(int id,t_list** lista_archivos){
	printf("PLANIFICANDO...\n");

	t_archivo *el_archivo;
	t_nodo *el_nodo;
	t_dato *el_dato,*el_dato_dos;

	t_list *aux=(t_list*)malloc(sizeof(t_dato));

	int pesoMax=0;
	int i=0,cantBloques=0,c=0,k=0;

	pesoMax=obtenerPesoMax(*lista_archivos);//
	while(i<list_size(*lista_archivos)){
		el_archivo=list_get(*lista_archivos,i);
		//SIN COMBINER
		if(el_archivo->idJob==id && el_archivo->tieneCombiner==0){
			for(c=0;c<list_size(el_archivo->listaBloques)*3;c++){
				if(list_size(el_archivo->array_de_listas[c])>0){
					cantBloques=list_size(el_archivo->array_de_listas[c])-1;
					el_dato=list_get(el_archivo->array_de_listas[c],0);
					sem_wait(&semaforoListaNodos);
					el_nodo=buscarNodo(el_dato->dato);
					sem_post(&semaforoListaNodos);
					if(el_nodo!=NULL){
						if(cantBloques!=0)
							el_dato->peso=(cantBloques*pesoMax-el_nodo->cantTareasPendientes)*el_nodo->estado;
						else
							el_dato->peso=0;
					}
				}
			}
		}else{
		//CON COMBINER
			if(el_archivo->idJob==id && el_archivo->tieneCombiner==1){
				for(c=0;c<list_size(el_archivo->listaBloques)*3;c++){
					if(list_size(el_archivo->array_de_listas[c])>0){
						cantBloques=list_size(el_archivo->array_de_listas[c])-1;
						el_dato=list_get(el_archivo->array_de_listas[c],0);
						sem_wait(&semaforoListaNodos);
						el_nodo=buscarNodo(el_dato->dato);
						sem_post(&semaforoListaNodos);
						if(el_nodo!=NULL){
							if(cantBloques!=0)
								el_dato->peso=(pesoMax-el_nodo->cantTareasPendientes)*el_nodo->estado;
							else
								el_dato->peso=0;
						}
					}

				}
			}

		}
		i++;
	}

	i=0;
	int ordenado =0;
	while(i<list_size(*lista_archivos)){
		el_archivo=list_get(*lista_archivos,i);
		//SIN COMBINER y CON COMBINER ==> Ordeno el Array de punteros a listas
		if(el_archivo->idJob==id){
			ordenado=0;
			while(ordenado==0){
				ordenado=1;
				for(k=1;k<list_size(el_archivo->listaBloques)*3;k++){
					if(list_size(el_archivo->array_de_listas[k])>0){
						el_dato=list_get(el_archivo->array_de_listas[k-1],0);
						el_dato_dos=list_get(el_archivo->array_de_listas[k],0);
						if(el_dato->peso<el_dato_dos->peso){

							aux= el_archivo->array_de_listas[k-1];
							el_archivo->array_de_listas[k-1]=el_archivo->array_de_listas[k];
							el_archivo->array_de_listas[k]=aux;
							ordenado=0;
						}
					}
				}
			}
		}
		i++;
	}



	//MUESTRO POR PANTALLA
	i=0;
	while(i<list_size(*lista_archivos)){
		el_archivo = list_get(*lista_archivos, i);
		if(el_archivo->idJob == id){
			RecorrerArrayListas(el_archivo);
		}
		i++;
	}
}

void marcarBloquesEnArray(t_archivo *el_archivo,char *bloque){

	int k=0;
	int c=0;
	t_dato *el_dato;


	for(k=0;k<list_size(el_archivo->listaBloques)*3;k++){

		if(list_size(el_archivo->array_de_listas[k])>0){
			c=0;
			while(c<list_size(el_archivo->array_de_listas[k])){
				el_dato=list_get(el_archivo->array_de_listas[k],c);
				if(strcmp(el_dato->dato,bloque)==0){
					el_dato->peso=(-10);
				}

				c++;
			}
		}
	}


}


char* procesarContadorArchivoParcial(int contador,char* resultado){


	char *archResultado=string_new();
	char **array;
	int j=0,aux=0,cont=0;

	array =(char**) malloc (strlen(resultado));
	array=string_split(resultado,"/");

	while (array[cont]!=NULL){
		for(j=0;j<strlen(array[cont]);j++){
			if(array[cont][j]=='.'){
				aux=cont;

			}
		}
		cont++;
	}


	string_append(&archResultado,string_itoa(contador));
	string_append(&archResultado,array[aux]);

	return archResultado;

}



void enviarPlanificacionAJob (int id,int socket,t_list** lista_archivos,t_list** lista_job_enviado){

	//415NodoA212192.168.1.2614600018Bloque30213resultado.txt
	t_archivo *el_archivo;
	t_nodo *el_nodo;
	t_dato *el_dato;
	t_dato *el_dato_aux;

	int i=0,c=0,contadorBloques=0;
	char* nodo;
	char* ipNodo;
	char* puertoNodo;
	char *bloque;
	char *resultado;
	int contadorArchivoParcial=0;
	char *archParcial=string_new();
	int contador=0;

	while(i<list_size(*lista_archivos)){
		el_archivo=list_get(*lista_archivos,i);
		contador=0;
		//SIN COMBINER
		if(el_archivo->idJob==id && el_archivo->tieneCombiner==0){
			for(c=0;c<list_size(el_archivo->listaBloques)*3;c++){
				if(list_size(el_archivo->array_de_listas[c])>0){
					contadorBloques=0;
					el_dato_aux=list_get(el_archivo->array_de_listas[c],contadorBloques);
					sem_wait(&semaforoListaNodos);
					el_nodo=buscarNodo(el_dato_aux->dato);
					sem_post(&semaforoListaNodos);
					for(contadorBloques=1;contadorBloques<list_size(el_archivo->array_de_listas[c]);contadorBloques++){
						el_dato=list_get(el_archivo->array_de_listas[c],contadorBloques);

						if(el_dato->peso==0){
							//Agrego a la lista de nodos un bloque de archivo listo para trabajar (de determinado nodo)
							list_add(el_nodo->listaBloqueArchivo,bloqueArchivo_create(el_dato->bloqueDelNodo,el_archivo->nombreArchivo,0));
							el_nodo->cantTareasPendientes++;

							archParcial=procesarContadorArchivoParcial(contadorArchivoParcial,el_archivo->nombreArchivoResultado);
							//Agrego a la lista de job's a enviar (ESTA LISTA ME VA A QUEDAR, NO LA VOY A ELIMINAR)
							list_add(*lista_job_enviado,job_enviado_create(el_dato_aux->dato,el_dato->bloqueDelNodo,el_archivo->nombreArchivo,archParcial));
							contadorArchivoParcial++;
							archParcial=string_new();
							//Marco los bloques q subo a la lista de nodos, EJ: subo el bloque0 entonces marco todos los bloques0 como ya subidos.
							marcarBloquesEnArray(el_archivo,el_dato->dato);
						}
					}

				}
			}

		}
		// CON COMBINER

		// CON COMBINER
		if(el_archivo->idJob==id && el_archivo->tieneCombiner==1){
			for(c=0;c<list_size(el_archivo->listaBloques)*3;c++){
				if(list_size(el_archivo->array_de_listas[c])>0){
					contadorBloques=0;
					el_dato_aux=list_get(el_archivo->array_de_listas[c],contadorBloques);
					sem_wait(&semaforoListaNodos);
					el_nodo=buscarNodo(el_dato_aux->dato);
					sem_post(&semaforoListaNodos);
					for(contadorBloques=1;contadorBloques<list_size(el_archivo->array_de_listas[c]);contadorBloques++){
						el_dato=list_get(el_archivo->array_de_listas[c],contadorBloques);

						if(el_dato->peso==0){
							//Agrego a la lista de nodos un bloque de archivo listo para trabajar (de determinado nodo)
							list_add(el_nodo->listaBloqueArchivo,bloqueArchivo_create(el_dato->bloqueDelNodo,el_archivo->nombreArchivo,0));
							el_nodo->cantTareasPendientes++;

							archParcial=procesarContadorArchivoParcial(contadorArchivoParcial,el_archivo->nombreArchivoResultado);
							//Agrego a la lista de job's a enviar (ESTA LISTA ME VA A QUEDAR, NO LA VOY A ELIMINAR)
							list_add(*lista_job_enviado,job_enviado_create(el_dato_aux->dato,el_dato->bloqueDelNodo,el_archivo->nombreArchivo,archParcial));
							contadorArchivoParcial++;
							contador++;
							//Marco los bloques q subo a la lista de nodos, EJ: subo el bloque0 entonces marco todos los bloques0 como ya subidos.
							marcarBloquesEnArray(el_archivo,el_dato->dato);
							contadorBloques=list_size(el_archivo->array_de_listas[c]);


						}
					}

				}
				if(contador<list_size(el_archivo->listaBloques)){
					sem_wait(&semaforoListaNodos);
					if(c+1>=list_size(lista_nodos)){
						c=0-1;

					}
					sem_post(&semaforoListaNodos);
				}
			}

		}
		i++;
	}

	contador = 0;


	t_bloqueArchivo *el_bloqueArchivo;
	t_job_enviado *el_job_enviado;
	int x=0;
	char* buffer=string_new();
	i=0;
	x=0;
		while(x<list_size(*lista_job_enviado)){
			el_job_enviado=list_get(*lista_job_enviado,x);

			//if(strcmp(el_job_enviado->bloque,bloque)==0 && strcmp(el_job_enviado->nodo,nodo)==0 && el_job_enviado->estado==0){

			if(el_job_enviado->estado==0){

				el_job_enviado->estado=1;
				sem_wait(&semaforoListaNodos);
				el_nodo=buscarNodo(el_job_enviado->nodo);
				sem_post(&semaforoListaNodos);

				bool _true3(void *elem){
					return ( !strcmp(((t_bloqueArchivo*) elem)->bloque,el_job_enviado->bloque));
				}

				el_bloqueArchivo = list_find(el_nodo->listaBloqueArchivo, _true3);

				el_bloqueArchivo->procesando=1;

				nodo=obtenerSubBuffer(el_nodo->nombreNodo);
				ipNodo=obtenerSubBuffer(el_nodo->ipNodo);
				puertoNodo=obtenerSubBuffer(el_nodo->puertoNodo);
				bloque=obtenerSubBuffer(el_job_enviado->bloque);
				resultado=obtenerSubBuffer(el_job_enviado->resultadoParcial);

				string_append(&buffer,"4");
				string_append(&buffer,"1");
				string_append(&buffer,nodo);
				string_append(&buffer,ipNodo);
				string_append(&buffer,puertoNodo);
				string_append(&buffer,bloque);
				string_append(&buffer,resultado);
				sem_wait(&semaforoListaNodos);
				el_nodo->cantTareasPendientes--;
				sem_post(&semaforoListaNodos);

				printf(COLOR_VERDE"----%s---\n"DEFAULT,buffer);

				sem_wait(&semSocket);
				EnviarDatos(socket, buffer,strlen(buffer));
				sem_post(&semSocket);




				buffer=string_new();
				nodo=string_new();
				ipNodo=string_new();
				puertoNodo=string_new();
				bloque=string_new();
				resultado=string_new();

				sleep(1);
			}
			x++;

		}
		sem_wait(&semSocket);
		EnviarDatos(socket, "+",strlen("+"));
		sem_post(&semSocket);
}

t_bloque* buscarNodoYBloque (char * nodo, char * bloque,int *numCopia,t_archivo **archivo,int *estado,t_list* lista_archivos){

	t_archivo *el_archivo;
	t_bloque *el_bloque;
	int i=0,j=0,c=0;


	while(i<list_size(lista_archivos)){
		el_archivo=list_get(lista_archivos,i);
		j=0;
		while(j<list_size(el_archivo->listaBloques)){
			el_bloque=list_get(el_archivo->listaBloques,j);

			for(c=0;c<3;c++){

				if(strcmp(el_bloque->array[c].nodo,nodo)==0 && strcmp(el_bloque->array[c].bloque,bloque)==0){
					if(el_bloque->array[c].estado==0)
						*estado=1;
					else if(el_bloque->array[c].estado==1)
						*estado=2;

					*numCopia=c;
					*archivo=el_archivo;
					return el_bloque;
				}
			}
			j++;
		}
		i++;
	}
	return NULL;

}

void eliminarBloquesDelArchivoEnArray(t_archivo *el_archivo, char* bloque){

	int k=0,c=0;
	t_dato *aux;

	for(c=0;c<list_size(el_archivo->listaBloques)*3;c++){
		if(list_size(el_archivo->array_de_listas[c])>0){
			for(k=0;k<list_size(el_archivo->array_de_listas[c]);k++){
				aux=list_get(el_archivo->array_de_listas[c],k);

				if(strcmp(aux->dato,bloque)==0){
					list_remove(el_archivo->array_de_listas[c],k);
				}
			}
		}
	}
}

int tareasRestantes(int estadoAEvaluar,t_list** lista_archivos){
	t_archivo *el_archivo;
	int i=0,c=0,j=0;
	int contTareas=0;
	t_bloque *el_bloque;

	while(i<list_size(*lista_archivos)){
		el_archivo=list_get(*lista_archivos,i);
		contTareas=contTareas+list_size(el_archivo->listaBloques);
		i++;
	}



	while(j<list_size(*lista_archivos)){
		el_archivo=list_get(*lista_archivos,j);
		i=0;
		while(i<list_size(el_archivo->listaBloques)){
			el_bloque=list_get(el_archivo->listaBloques,i);

			for(c=0;c<3;c++){

				if(el_bloque->array[c].estado>=estadoAEvaluar){
					contTareas--;
					c=3;
				}
			}
			i++;
		}
		j++;
	}
	return contTareas;

}

char* buscarProximaTarea(t_archivo **archivo,int estado,t_list* lista_archivos){

	int i=0,k=0,j=0;
	t_archivo *el_archivo;
	t_bloque *el_bloque;

	while(j<list_size(lista_archivos)){
		el_archivo=list_get(lista_archivos,j);
		i=0;
		while(i<list_size(el_archivo->listaBloques)){
			el_bloque=list_get(el_archivo->listaBloques,i);
			for(k=0;k<3;k++){

				if(el_bloque->array[k].estado>=estado){
					k=3;
				}else{
					if(k+1==3){
						*archivo=el_archivo;
						return el_bloque->bloque;
					}
				}
			}
			i++;
		}
		j++;
	}

	return NULL;

}

char* buscarProximaTareaEnArray(t_archivo *el_archivo,char *bloque){

	int i=0,k=0;
	t_dato *el_dato;

	for(k=0;k<list_size(el_archivo->listaBloques)*3;k++){

		if(list_size(el_archivo->array_de_listas[k])>0){
			i=0;
			while(i<list_size(el_archivo->array_de_listas[k])){
				el_dato=list_get(el_archivo->array_de_listas[k],i);

				if(strcmp(el_dato->dato,bloque)==0){
					el_dato=list_get(el_archivo->array_de_listas[k],0);
					return el_dato->dato;

				}

				i++;
			}

		}

	}

	return NULL;

}

t_archivo * buscarPorNombre (char *nombre,t_list* lista_archivos){

	t_archivo *el_archivo;
	int i=0;

	while(i<list_size(lista_archivos)){
		el_archivo=list_get(lista_archivos,i);
		if(strcmp(el_archivo->nombreArchivoResultado,nombre)==0){
			return el_archivo;
		}
		i++;
	}

	return NULL;
}


void eliminarPorId(int id,t_list** lista_archivos){
	t_archivo* el_archivo;
	int i=0;
	while(i<list_size(*lista_archivos)){
		el_archivo = list_get(*lista_archivos,i);
		if(el_archivo->idJob == id){
			/*free(el_archivo->nombreArchivo);
			free(el_archivo->nombreArchivoResultado);
			j=0;
			while(j<list_size(el_archivo->listaBloques)){
				bloque = list_remove(el_archivo->listaBloques,j);
				free(bloque->bloque);
				for(p=0;p<3;p++){
					free(bloque->array[p].bloque);
			s		free(bloque->array[p].nodo);
				}
				free(bloque);
			}*/
			el_archivo = list_remove(*lista_archivos,i);
			free(el_archivo);
		} else {
			i++;
		}
	}
}

void borrarTodo(t_archivo *el_archivo,t_list** lista_archivos){

	int id = el_archivo->idJob;
	eliminarPorId(id,lista_archivos);
}

void reducesOk(char *buffer,t_list** lista_archivos,t_list** lista_job_enviado){
	//24234/users/dasdas/resultado.txt
	t_archivo *el_archivo;
	char *nombreArchRes=string_new();
	int pos=2;


	nombreArchRes=DigitosNombreArchivo(buffer,&pos);
	el_archivo=buscarPorNombre(nombreArchRes,*lista_archivos);

	if(el_archivo!=NULL){
		printf(COLOR_VERDE"****** TODOS LOS MAP's Y REDUCES FUERON HECHOS! ******\n"DEFAULT);
		printf("\nArchivo [ "COLOR_VERDE"%s"DEFAULT" ] procesado con exito!\n\n",el_archivo->nombreArchivo);
	}
	eliminarJobEnviados(el_archivo,lista_job_enviado);
	borrarTodo(el_archivo,lista_archivos);

}

void eliminarBloqueDeListaDeNodos(char* nodo,char *bloque){

	int x=0;
	t_nodo *el_nodo;
	t_bloqueArchivo *el_bloqueArchivo;
	sem_wait(&semaforoListaNodos);
	el_nodo = buscarNodo(nodo);
	sem_post(&semaforoListaNodos);
	if(el_nodo!=NULL){
		printf("CANTIDAD DE BLOQUES:%d\n",list_size(el_nodo->listaBloqueArchivo));
		while(x<list_size(el_nodo->listaBloqueArchivo)){

			el_bloqueArchivo=list_get(el_nodo->listaBloqueArchivo,x);

			if(strcmp(el_bloqueArchivo->bloque,bloque)==0){
				list_remove(el_nodo->listaBloqueArchivo,x);
				x=list_size(el_nodo->listaBloqueArchivo)+2;

			}
			x++;
		}
	}
}

void eliminarJobEnviados(t_archivo *el_archivo,t_list** lista_job_enviado){
	t_job_enviado* el_job;
	int i=0;
	while(i<list_size(*lista_job_enviado)){
		el_job = list_get(*lista_job_enviado,i);
		if(!strcmp(el_archivo->nombreArchivo,el_job->archivo)){
			el_job = list_remove(*lista_job_enviado,i);
			free(el_job);
		} else {
			i++;
		}
	}
}

void reciboOk(char *buffer,int socket,t_list** lista_archivos,t_list** lista_job_enviado){

	//2318Bloque3015NodoA
	t_bloque *el_bloque;
	t_nodo *el_nodo;
	t_job_enviado *el_job_enviado;
	char *bloque=string_new();
	char *nodo=string_new(),*nodoAnt=string_new();
	int pos=2,tareaCompleta=0;
	t_archivo *el_archivo;
	bloque=DigitosNombreArchivo(buffer,&pos);
	nodo=DigitosNombreArchivo(buffer,&pos);


	int numCopia=0,x=0,j=0;
	int contTareasMap=-10;//,contTareasReduce=-10;
	int estado=0;
	//sem_wait(&semaforoListaNodos);
	el_bloque=buscarNodoYBloque(nodo,bloque,&numCopia,&el_archivo,&estado,*lista_archivos);
	//sem_post(&semaforoListaNodos);
	printf("***********************************************************************\n");
	if(estado==1){
		printf("* Recibo OK del Job ("COLOR_VERDE"map"DEFAULT"):   "COLOR_VERDE"%s"DEFAULT"--"COLOR_VERDE"%s\n"DEFAULT,bloque,nodo);
		string_append(&nodoAnt,nodo);


	}else if(estado==2){
		printf("* Recibo OK del Job ("COLOR_VERDE"reduce"DEFAULT"):   "COLOR_VERDE"%s"DEFAULT"--"COLOR_VERDE"%s\n"DEFAULT,bloque,nodo);

	}



	if(el_bloque!=NULL){


		//Cambio el estado
		if(estado==1 || estado==2){
			el_bloque->array[numCopia].estado=estado;
			//sem_wait(&semaforoListaNodos);
			el_nodo=buscarNodo(el_bloque->array[numCopia].nodo);
			//sem_post(&semaforoListaNodos);
		}



		if(estado==1){
			eliminarBloquesDelArchivoEnArray(el_archivo,el_bloque->bloque);
			//sem_wait(&semaforoListaNodos);
			eliminarBloqueDeListaDeNodos(nodo,bloque);
			//sem_post(&semaforoListaNodos);
		}
		if(estado==2){
			printf("* Reduces Hechos: %d \n",el_archivo->contTareas);
			el_archivo->contTareas++;
			//sem_wait(&semaforoListaNodos);
			eliminarBloqueDeListaDeNodos(nodo,bloque);
			//sem_post(&semaforoListaNodos);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////


		if(tareaCompleta==0){

			//Cuento Tareas Pendientes del Archivo
			contTareasMap=tareasRestantes(1,lista_archivos);
			//contTareasReduce=tareasRestantes(2);

			printf("* Tareas Restantes de Map: "COLOR_VERDE"%d\n"DEFAULT,contTareasMap);




			if(el_archivo->tieneCombiner==0){

				el_archivo->contTareas++;

				t_archivo *el_archivo_auxiliar;
				int z=0,contador_tareas=0,contador_tareas_actual=0,id_auxiliar=el_archivo->idJob;


				while(z<list_size(*lista_archivos)){
					el_archivo_auxiliar=list_get(*lista_archivos,z);
					if(el_archivo_auxiliar->idJob==id_auxiliar)
						contador_tareas=contador_tareas+list_size(el_archivo_auxiliar->listaBloques);
					z++;
				}

				z=0;
				while(z<list_size(*lista_archivos)){
					el_archivo_auxiliar=list_get(*lista_archivos,z);
					if(el_archivo_auxiliar->idJob==id_auxiliar){
						contador_tareas_actual= contador_tareas_actual + el_archivo_auxiliar->contTareas;
					}
					z++;
				}


				if(contador_tareas_actual==contador_tareas){
					printf(COLOR_VERDE"ENVIADO REDUCE\n"DEFAULT);
					char *bufferAJob_Reduce=string_new();
					char *nodoAnterior=string_new();
					int contarNodos=0;

					t_list *lista_archivos_reduce = list_create();
					j=0;
					z=0;
					while(z<list_size(*lista_archivos)){
						el_archivo_auxiliar=list_get(*lista_archivos,z);
						j=0;
						while(j<list_size(*lista_job_enviado)){
							el_job_enviado=list_get(*lista_job_enviado,j);
							if(strcmp(el_job_enviado->archivo,el_archivo_auxiliar->nombreArchivo)==0){

								list_add(lista_archivos_reduce,job_enviado_create(el_job_enviado->nodo,el_job_enviado->bloque,el_job_enviado->archivo,el_job_enviado->resultadoParcial));


							}
							j++;
						}


						z++;
					}


					// ORDENO POR NOMBRE DE NODOS
					bool _ordenarPorNodo(t_job_enviado *nodoUno, t_job_enviado *nodoDos){
						if(strcmp(nodoUno->nodo,nodoDos->nodo)<0)
							return true;
						else
							return false;
					}
					list_sort(lista_archivos_reduce, (void*)_ordenarPorNodo);


					t_job_enviado_bloque *el_job_enviado_bloque;
					t_job_enviado *el_aux;

					//Cuento los Nodos y agrego bloques a cada nodo
					j=0;
					int k=0;
					while(j<list_size(lista_archivos_reduce)){

						el_job_enviado=list_get(lista_archivos_reduce,j);

						el_aux=el_job_enviado;
						contarNodos++;
						string_append(&nodoAnterior,el_job_enviado->nodo);
						while(strcmp(nodoAnterior,el_job_enviado->nodo)==0 && j<list_size(lista_archivos_reduce)){
							list_add(el_aux->listaBloques,job_enviado_bloque_create(el_job_enviado->bloque,el_job_enviado->resultadoParcial));

							j++;
							if(j<list_size(lista_archivos_reduce))
								el_job_enviado=list_get(lista_archivos_reduce,j);
						}
						nodoAnterior=string_new();
					}

					//Borro los nodos con listas vacias
					j=0;
					while(j<list_size(lista_archivos_reduce)){
						el_job_enviado=list_get(lista_archivos_reduce,j);
						if(list_size(el_job_enviado->listaBloques)==0){
							list_remove(lista_archivos_reduce,j);
							j--;
						}
						j++;
					}


					t_job_enviado *aux_job;
					int ordenado =0;

					while(ordenado==0){
						ordenado=1;
						for(k=1;k<list_size(lista_archivos_reduce);k++){
							el_job_enviado=list_get(lista_archivos_reduce,k);
							aux_job=list_get(lista_archivos_reduce,k-1);
							if(list_size(el_job_enviado->listaBloques)>list_size(aux_job->listaBloques)){

								list_replace(lista_archivos_reduce,k,aux_job);
								list_replace(lista_archivos_reduce,k-1,el_job_enviado);

								ordenado=0;
							}
						}
					}




					//---------------HAY QUE VER BIEN EL PROTOCOLO------------------------
					// 4311 15NodoA  13 18Bloque30     18Bloque38    18Bloque43      15NodoA212192.168.1.27146000 230/user/juan/datos/resultado.txt
					string_append(&bufferAJob_Reduce,"43");
					string_append(&bufferAJob_Reduce,string_itoa(cuentaDigitos(contarNodos)));
					string_append(&bufferAJob_Reduce,string_itoa(contarNodos));



					j=0;
					k=0;
					while(j<list_size(lista_archivos_reduce)){
						el_job_enviado=list_get(lista_archivos_reduce,j);
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_job_enviado->nodo));
						string_append(&bufferAJob_Reduce,string_itoa(cuentaDigitos(list_size(el_job_enviado->listaBloques))));
						string_append(&bufferAJob_Reduce,string_itoa(list_size(el_job_enviado->listaBloques)));
						k=0;
						while(k<list_size(el_job_enviado->listaBloques)){
							el_job_enviado_bloque=list_get(el_job_enviado->listaBloques,k);
							string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_job_enviado_bloque->resultadoParcial));
							k++;
						}
						//sem_wait(&semaforoListaNodos);
						el_nodo=buscarNodo(el_job_enviado->nodo);
						//sem_post(&semaforoListaNodos);
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->nombreNodo));
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->ipNodo));
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->puertoNodo));
						j++;
					}

					string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_archivo->nombreArchivoResultado));

					//sem_wait(&semSocket);
					EnviarDatos(socket,bufferAJob_Reduce,strlen(bufferAJob_Reduce));
					//sem_post(&semSocket);
					el_archivo->contTareas=0;//Asi no entra mas
					tareaCompleta=1;
					//printf("//////////"COLOR_VERDE " %s "DEFAULT"//////////////////\n",bufferAJob_Reduce);

				}

			}

		//Aplico reduce a cada bloque que recibo con OK (CON COMBINER)
			if(el_archivo->tieneCombiner==1){


				t_archivo *el_archivo_auxiliar;
				int z=0,contador_tareas=0,contador_tareas_actual=0,id_auxiliar=el_archivo->idJob;


				while(z<list_size(*lista_archivos)){
					el_archivo_auxiliar=list_get(*lista_archivos,z);
					if(el_archivo_auxiliar->idJob==id_auxiliar)
						contador_tareas=contador_tareas+list_size(el_archivo_auxiliar->listaBloques);
					z++;
				}

				z=0;
				while(z<list_size(*lista_archivos)){
					el_archivo_auxiliar=list_get(*lista_archivos,z);
					if(el_archivo_auxiliar->idJob==id_auxiliar){
						contador_tareas_actual= contador_tareas_actual + el_archivo_auxiliar->contTareas;
					}
					z++;
				}




				if(contador_tareas_actual==contador_tareas){

					printf(COLOR_VERDE"ENVIO REDUCE TOTAL\n"DEFAULT);

					char *bufferAJob_Reduce=string_new();
					char *nodoAnterior=string_new();
					int contarNodos=0;

					t_list *lista_archivos_reduce = list_create();
					z=0;
					while(z<list_size(*lista_archivos)){
						el_archivo_auxiliar=list_get(*lista_archivos,z);
						j=0;
						while(j<list_size(*lista_job_enviado)){
							el_job_enviado=list_get(*lista_job_enviado,j);
							if(strcmp(el_job_enviado->archivo,el_archivo_auxiliar->nombreArchivo)==0){

								list_add(lista_archivos_reduce,job_enviado_create(el_job_enviado->nodo,el_job_enviado->bloque,el_job_enviado->archivo,el_job_enviado->resultadoParcialReduce));


							}
							j++;
						}


						z++;
					}

					// ORDENO POR NOMBRE DE NODOS
					bool _ordenarPorNodo(t_job_enviado *nodoUno, t_job_enviado *nodoDos){
						if(strcmp(nodoUno->nodo,nodoDos->nodo)<0)
							return true;
						else
							return false;
					}
					list_sort(lista_archivos_reduce, (void*)_ordenarPorNodo);



					t_job_enviado_bloque *el_job_enviado_bloque;
					t_job_enviado *el_aux;

					//Cuento los Nodos y agrego bloques a cada nodo
					j=0;
					int k=0;
					while(j<list_size(lista_archivos_reduce)){
						el_job_enviado=list_get(lista_archivos_reduce,j);
						el_aux=el_job_enviado;
						contarNodos++;
						string_append(&nodoAnterior,el_job_enviado->nodo);
						while(strcmp(nodoAnterior,el_job_enviado->nodo)==0 && j<list_size(lista_archivos_reduce)){
							list_add(el_aux->listaBloques,job_enviado_bloque_create(el_job_enviado->bloque,el_job_enviado->resultadoParcial));

							j++;
							if(j<list_size(lista_archivos_reduce))
								el_job_enviado=list_get(lista_archivos_reduce,j);
						}
						nodoAnterior=string_new();
					}

					//Borro los nodos con listas vacias
					j=0;
					while(j<list_size(lista_archivos_reduce)){
						el_job_enviado=list_get(lista_archivos_reduce,j);
						if(list_size(el_job_enviado->listaBloques)==0){
							list_remove(lista_archivos_reduce,j);
							j--;
						}
						j++;
					}


					t_job_enviado *aux_job;
					int ordenado =0;
					t_nodo* nodo1,*nodo2;
					while(ordenado==0){
						ordenado=1;
						for(k=1;k<list_size(lista_archivos_reduce);k++){
							el_job_enviado=list_get(lista_archivos_reduce,k);
							//sem_wait(&semaforoListaNodos);
							nodo1 = buscarNodo(el_job_enviado->nodo);
							aux_job=list_get(lista_archivos_reduce,k-1);
							nodo2 = buscarNodo(aux_job->nodo);
							//sem_post(&semaforoListaNodos);
							if((list_size(el_job_enviado->listaBloques)>list_size(aux_job->listaBloques))&&
									(nodo1->reduceFinal<=nodo2->reduceFinal)){

								list_replace(lista_archivos_reduce,k,aux_job);
								list_replace(lista_archivos_reduce,k-1,el_job_enviado);

								ordenado=0;
							}
						}
					}




					//---------------HAY QUE VER BIEN EL PROTOCOLO------------------------
					// 4311 15NodoA  13 18Bloque30     18Bloque38    18Bloque43      15NodoA212192.168.1.27146000 230/user/juan/datos/resultado.txt
					string_append(&bufferAJob_Reduce,"43");
					string_append(&bufferAJob_Reduce,string_itoa(cuentaDigitos(contarNodos)));
					string_append(&bufferAJob_Reduce,string_itoa(contarNodos));



					j=0;
					k=0;
					while(j<list_size(lista_archivos_reduce)){
						el_job_enviado=list_get(lista_archivos_reduce,j);
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_job_enviado->nodo));
						string_append(&bufferAJob_Reduce,string_itoa(cuentaDigitos(list_size(el_job_enviado->listaBloques))));
						string_append(&bufferAJob_Reduce,string_itoa(list_size(el_job_enviado->listaBloques)));
						k=0;
						while(k<list_size(el_job_enviado->listaBloques)){
							el_job_enviado_bloque=list_get(el_job_enviado->listaBloques,k);
							string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_job_enviado_bloque->resultadoParcial));
							k++;
						}
						//sem_wait(&semaforoListaNodos);
						el_nodo=buscarNodo(el_job_enviado->nodo);
						//sem_post(&semaforoListaNodos);
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->nombreNodo));
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->ipNodo));
						string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_nodo->puertoNodo));
						j++;
					}

					string_append(&bufferAJob_Reduce,obtenerSubBuffer(el_archivo->nombreArchivoResultado));

					printf("TOTAL:%s\n",bufferAJob_Reduce);
					//sem_wait(&semSocket);
					EnviarDatos(socket,bufferAJob_Reduce,strlen(bufferAJob_Reduce));
					//sem_post(&semSocket);
					el_archivo->contTareas=0;//Asi no entra mas
					tareaCompleta=1;

					//printf("//////////"COLOR_VERDE " %s "DEFAULT"//////////////////\n",bufferAJob_Reduce);

				}

				if(estado==1 && el_archivo->contTareas<list_size(el_archivo->listaBloques)){
					char *nodoR=string_new(),*ipNodoR=string_new(),*puertoNodoR=string_new(),*bloqueR=string_new(),*resultadoR=string_new();
					char *bufferReduce=string_new();
					//sem_wait(&semaforoListaNodos);
					el_nodo=buscarNodo(nodo);
					//sem_post(&semaforoListaNodos);
					nodoR=el_nodo->nombreNodo;
					ipNodoR=el_nodo->ipNodo;
					puertoNodoR=el_nodo->puertoNodo;
					bloqueR=el_bloque->array[numCopia].bloque;

					x=0;
					while(x<list_size(*lista_job_enviado)){
						el_job_enviado=list_get(*lista_job_enviado,x);
						if(strcmp(el_job_enviado->bloque,bloque)==0 && strcmp(el_job_enviado->nodo,nodo)==0 && el_job_enviado->estado==1){
							el_job_enviado->estado=2;


							list_add(el_nodo->listaBloqueArchivo,bloqueArchivo_create(bloqueR,el_archivo->nombreArchivo,0));
							char *nombre_archmap = string_new();
							char *nombre_archred = string_new();

							string_append(&nombre_archred,"Reduce");
							string_append(&nombre_archred,el_job_enviado->resultadoParcial);
							el_job_enviado->resultadoParcialReduce=strdup(nombre_archred);

							nodoR=obtenerSubBuffer(nodoR);
							ipNodoR=obtenerSubBuffer(ipNodoR);
							puertoNodoR=obtenerSubBuffer(puertoNodoR);
							bloqueR=obtenerSubBuffer(el_job_enviado->bloque);
							nombre_archmap=obtenerSubBuffer(el_job_enviado->resultadoParcial);
							resultadoR=obtenerSubBuffer(el_job_enviado->resultadoParcialReduce);
							string_append(&bufferReduce,"4");
							string_append(&bufferReduce,"2");
							string_append(&bufferReduce,nodoR);
							string_append(&bufferReduce,ipNodoR);
							string_append(&bufferReduce,puertoNodoR);
							string_append(&bufferReduce,bloqueR);
							string_append(&bufferReduce,nombre_archmap);
							string_append(&bufferReduce,resultadoR);

							//sem_wait(&semSocket);
							EnviarDatos(socket,bufferReduce,strlen(bufferReduce));
							//sem_post(&semSocket);
							x=list_size(*lista_job_enviado);



						}
						x++;
					}

				}

			}
			if(estado==1 || estado==2){
				printf("* Envio *\n");
				char *envio=string_new();
				string_append(&envio,"*");
				sleep(1);
				EnviarDatos(socket, envio,strlen(envio));

				free(envio);
			}
		}
	}


}


void implementoJob(int *id,char * buffer,int * cantRafaga,char ** mensaje, int socket,t_list** lista_archivos,t_list** lista_job_enviado,t_list*** array_listas){



	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);

	if(*cantRafaga == 2 || *cantRafaga == 3){
		switch(tipo_mensaje){
		case RECIBIR_ARCHIVO:

			AtiendeJob(id,buffer,cantRafaga,lista_archivos);
			*cantRafaga=0;
			ObtenerInfoDeNodos(*id,lista_archivos,array_listas);
			Planificar(*id,lista_archivos);
			enviarPlanificacionAJob(*id,socket,lista_archivos,lista_job_enviado);
			*cantRafaga=3;

			break;
		case ERRORES:
			printf("BUFFER:%s\n",buffer);
			sem_wait(&semaforoListaArchivos);
			reciboError(buffer,socket,lista_archivos);
			sem_post(&semaforoListaArchivos);

			break;
		case RECIBIDO_OK:


			reciboOk(buffer,socket,lista_archivos,lista_job_enviado);


			//*mensaje=string_new();
			//string_append(mensaje,"Ok");
			*cantRafaga=3;
			break;
		case 4:

			reducesOk(buffer,lista_archivos,lista_job_enviado);

			*cantRafaga=3;
			break;
		default:
			break;
		}
		//*mensaje = "Ok";
	} else {
		if (*cantRafaga==1) {
			*mensaje="Ok";
			*cantRafaga = 2;
		} else {
			*mensaje = "No";
		}
	}
}


void reciboError(char*buffer,int socket,t_list** lista_archivos){

	//2218resu.txt15NodoA
	t_bloque *el_bloque;
	t_nodo *el_nodo;
	t_archivo *el_archivo;


	char *nodo;
	char *bloque;
	int pos=2;

	bloque=DigitosNombreArchivo(buffer,&pos);
	nodo=DigitosNombreArchivo(buffer,&pos);

	sem_wait(&semaforoListaNodos);
	el_nodo=buscarNodo(nodo);
	sem_post(&semaforoListaNodos);

	int numCopia=0;
	int estado=0;
	sem_wait(&semaforoListaNodos);
	el_bloque=buscarNodoYBloque(nodo,bloque,&numCopia,&el_archivo,&estado,*lista_archivos);
	sem_post(&semaforoListaNodos);
	if(el_bloque == NULL){
		printf("Error el bloque no existe\n");
	} else {
		printf("***************"COLOR_VERDE"ERROR"DEFAULT"***************\n");
		printf("* --> Aborto Job: "COLOR_VERDE"%s"DEFAULT"\n",el_archivo->nombreArchivo);
		printf("* --> Fallo del "COLOR_VERDE"%s"DEFAULT"\n",el_nodo->nombreNodo);
		printf("***********************************%s\n",el_bloque->bloque);

	}
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;
	int id=-1;
	//cantHilos++;
	t_list* lista_archivos = list_create();
	t_list* lista_job_enviado = list_create();
	t_list **array_listas;

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
	char * mensaje=string_new();
	while ((!desconexionCliente) && g_Ejecutando) {
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
				implementoJob(&id,buffer,&cantRafaga,&mensaje,socket,&lista_archivos,&lista_job_enviado,&array_listas);
				break;
			case ES_FS:
				printf("implementar atiendeFS\n");
				//implementoFS(buffer,&cantRafaga,&mensaje);
				break;
			case COMANDO:
				printf("Muestre toda la lista de Archivos:\n");
				RecorrerArchivos(lista_archivos);
				mensaje = "Ok";
				break;
			case COMANDOBLOQUES:
				printf("Muestre toda la lista de Bloques:\n");
				RecorrerListaBloques(lista_archivos);
				mensaje = "Ok";
				break;
			case COMANDONODOS:
				printf("Muestre toda la lista de NODOS:\n");
				RecorrerListaNodos();
				mensaje = "Ok";
				break;
			default:
				break;
			}

				longitudBuffer=strlen(mensaje);
				sem_wait(&semSocket);
				EnviarDatos(socket, mensaje,longitudBuffer);
				sem_post(&semSocket);
				mensaje=string_new();

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


