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

	lista_nodos = list_create();
	lista_archivos=list_create();



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
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char)+1);
		memset(bufferAux, 0, *tamanio * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}
	}else{

		if(*cantRafaga==3){

			bufferAux = realloc(bufferAux,100* sizeof(char));
			memset(bufferAux, 0, 100 * sizeof(char)); //-> llenamos el bufferAux con ceros.

			if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, 100, 0)) == -1) {
				Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
			}
			*cantRafaga=1;
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

void CargarArchivoALista(char* nomArchivo, int id){

	list_add(lista_archivos,archivo_create(nomArchivo,id));

}
void AtiendeJob (int * contIdJob,char *buffer, int *cantRafaga){

	char *nArchivo,*nResultado;
	int digitosCantDeArchivos=0,cantDeArchivos=0;
	int x,posActual=0;
	int tieneCombiner;
	*contIdJob=id_job;
	t_archivo *el_archivo;

	//wait(mutex)
	id_job++;
	//signal(mutex)

	//BUFFER RECIBIDO = 2270 (EJEMPLO)
	//BUFFER RECIBIDO = 2112220temperatura-2012.txt220temperatura-2013.txt213resultado.txt1
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 211

	digitosCantDeArchivos=PosicionDeBufferAInt(buffer,2);
	printf("CANTIDAD DE DIGITOS:%d\n",digitosCantDeArchivos);
	cantDeArchivos=ObtenerTamanio(buffer,3,digitosCantDeArchivos);
	printf("Cantidad de Archivos: %d\n",cantDeArchivos);
	posActual=3+digitosCantDeArchivos;

	for(x=0;x<cantDeArchivos;x++){
		nArchivo=DigitosNombreArchivo(buffer,&posActual);
		printf("NOMBRE:%s\n",nArchivo);
		CargarArchivoALista(nArchivo,*contIdJob);
	}
	nResultado=DigitosNombreArchivo(buffer,&posActual);
	tieneCombiner=PosicionDeBufferAInt(buffer,posActual); // CAMBIE strlen(buffer)-3 por posActual

	//Muestro por pantalla los Archivos
	int i=0;
	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		if (el_archivo->idJob == *contIdJob) {
			el_archivo->nombreArchivoResultado=nResultado;
			el_archivo->tieneCombiner=tieneCombiner;
		}
		i++;
	}
	*cantRafaga=1;
}

void RecorrerArchivos(){
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
	printf("Nombre Archivo:"COLOR_VERDE " %s\n"DEFAULT,el_archivo->nombreArchivo);
	for(i=0;i<list_size(el_archivo->listaBloques)*3;i++){
		j=0;
		while(j<list_size(el_archivo->array_de_listas[i])){
			el_dato = list_get(el_archivo->array_de_listas[i],j);
			if(j==0){
				printf("%s :::: ",el_dato->dato);
			} else {
				printf("%s--",el_dato->dato);
			}
			j++;
			if(j==list_size(el_archivo->array_de_listas[i]))
				printf("\n");
		}

	}
}


void RecorrerListaBloques(){
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
	int i=0;

	while(i<list_size(lista_nodos)){
		el_nodo=list_get(lista_nodos,i);
		printf("************************************************\n");
		printf("Nombre: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->nombreNodo);
		printf("IP: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->ipNodo);
		printf("Puerto: "COLOR_VERDE"%s\n"DEFAULT,el_nodo->puertoNodo);
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
	while(tam>1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioNombre));
	string_append(&aux,nombre);

	return aux;
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

		if ((socket_fs = socket(serverInfo->ai_family, serverInfo->ai_socktype,
				serverInfo->ai_protocol)) < 0) {
			log_info(logger, "ERROR: crear socket_fs");
		}
		if (connect(socket_fs, serverInfo->ai_addr, serverInfo->ai_addrlen)
				< 0) {
			log_info(logger, "ERROR: conectar socket_fs");
		}
		freeaddrinfo(serverInfo);	// No lo necesitamos mas
}


void ObtenerInfoDeNodos(int id){


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
	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		if (el_archivo->idJob == id) {
			contArchivos++;
		}
		i++;
	}

	//cuento los digitos
	float tam=contArchivos;
	int cont=0;
	while(tam>1){
			tam=tam/10;
			cont++;
	}
	if(cont==0)
		cont=1;

	string_append(&bufferDos,string_itoa(cont));
	string_append(&bufferDos,string_itoa(contArchivos));
	i=0;
	while(i<list_size(lista_archivos)){
			el_archivo = list_get(lista_archivos, i);
			if (el_archivo->idJob == id) {
				cont=0;
				tam=strlen(el_archivo->nombreArchivo);
				while(tam>1){
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
	while(tam>1){
		tam=tam/10;
		cont++;
	}
	string_append(&bufferUno,string_itoa(cont));
	string_append(&bufferUno,string_itoa(strlen(bufferDos)));


	printf("****** BUFFER'S A ENVIAR: \n");
	printf("****** %s\n",bufferUno);
	printf("****** %s\n",bufferDos);

	//Me conecto con el FS
	conectarAFileSystem();

	int bytesRecibidos=0;
	int tamanio=10,cantRafaga=1;

	char *buffer=string_new();
	char *rafaga1Fs=string_new(),*rafaga2Fs=string_new();

	//Envio Rafaga Uno a FS
	EnviarDatos(socket_fs, bufferUno,strlen(bufferUno));

	//Recibo el Ok
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	cantRafaga=1;

	//Envio la Segunda Rafaga
	EnviarDatos(socket_fs, bufferDos,strlen(bufferDos));


	//Recibo la primera rafaga de FS
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	EnviarDatos(socket_fs, "Ok",strlen("Ok"));

	rafaga1Fs= buffer;


	cantRafaga=2;
	//Recibo la segunda rafaga de Fs
	buffer = RecibirDatos(socket_fs,buffer, &bytesRecibidos,&cantRafaga,&tamanio);
	rafaga2Fs= buffer;


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


	for(x=0;x<cantArch;x++){
		j=0;
		nomArch=DigitosNombreArchivo(rafaga2Fs,&posActual);

		while(j<list_size(lista_archivos)){
			el_archivo=list_get(lista_archivos,j);
			if(el_archivo->idJob==id && strcmp(el_archivo->nombreArchivo,nomArch)==0){
				j=list_size(lista_archivos);
			}
			j++;
		}

		//Cantidad de Bloques
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
	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		if(el_archivo->idJob == id){
			FuncionMagica(el_archivo->listaBloques);//cargo array_listas
			el_archivo->array_de_listas=array_listas;
			RecorrerArrayListas(el_archivo);
		}
		i++;
	}

	//Cantidada de Nodos
	digCantNodo=PosicionDeBufferAInt(rafaga2Fs,posActual);
	cantNodo=ObtenerTamanio(rafaga2Fs,posActual+1,digCantNodo);
	posActual=posActual+1+digCantNodo;
	t_nodo *el_nodo=NULL;




	for(c=0;c<cantNodo;c++){
		nodo=DigitosNombreArchivo(rafaga2Fs,&posActual);

		for(j=0;j<list_size(lista_nodos);j++){
			el_nodo=list_get(lista_archivos,j);
			bool _true(void *elem){
				return ( !strcmp(((t_nodo*) elem)->nombreNodo,nodo) );
			}

			el_nodo = list_find(lista_nodos, _true);
			if(el_nodo != NULL){
				j=list_size(lista_nodos);
			}
		}
		if(el_nodo != NULL){

			//No hago nada por ahora

		}else{

			ip=DigitosNombreArchivo(rafaga2Fs,&posActual);
			puerto=DigitosNombreArchivo(rafaga2Fs,&posActual);
			list_add(lista_nodos,nodo_create(nodo,ip,puerto));
		}
	}




	CerrarSocket(socket_fs);

}

void FuncionMagica(t_list* listaBloques){

	t_bloque *el_bloque;
	t_dato *el_dato;

	int cantidadBloques;
	int posicion=0;
	int i,j,k;
	cantidadBloques = list_size(listaBloques);

	array_listas =(t_list**) malloc (cantidadBloques*3*sizeof(t_dato));

	for(i=0;i<cantidadBloques*3;i++){
		array_listas[i] = list_create();
	}

	for(i=0;i<cantidadBloques;i++){//recorro los bloques del archivo

		el_bloque = list_get(listaBloques,i);

		for(j=0;j<3;j++){          //recorro las 3 copias del bloque del archivo

			for(k=0;k<cantidadBloques*3;k++){        //recorre el array de la lista de nodos y bloques

				bool _true(void *elem) {
					return ( !strcmp(((t_dato*) elem)->dato,el_bloque->array[j].nodo) );
				}
				//buscar si el nodo de la copia existe en una lista de array_lista

				el_dato = list_find(array_listas[k], _true);

				if(el_dato != NULL){
					posicion = k;
					k=cantidadBloques*3;// salgo del for (no se si deberia usar exit o break)
				}

			}

			if(el_dato != NULL){

				list_add(array_listas[posicion],dato_create(el_bloque->bloque));

			}else{

				int h = 0;
				while(array_listas[h]->head != NULL){
					h++;
				}

				list_add(array_listas[h],dato_create(el_bloque->array[j].nodo));
				list_add(array_listas[h],dato_create(el_bloque->bloque));

			}
		}
	}
}

void Planificar(int id){
	printf("laalala ESTOY PLANIFICANDO\n");
/*	t_archivo *el_archivo;

	int i=0;

	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		if(el_archivo->idJob == id){
			FuncionMagica(el_archivo->listaBloques);//cargo array_listas
			el_archivo->array_de_listas=array_listas;
			RecorrerArrayListas(el_archivo);
		}
		i++;
	}

*/
}

void enviarPlanificacionAJob (int id,int socket){


	char *mensaje;
	mensaje=string_new();
	//En la lista de archivos en el array esta la planificacion
	//busco en base a la id y envio planificacion de cada archivo y de cada nodo.

	string_append(&mensaje,"4");//Viene de MarTA
	string_append(&mensaje,"15");//1: cantidad de dig del nomb del nodo 5:cantidad del nomb del nodo
	string_append(&mensaje,"NodoA");
	string_append(&mensaje,"19");//1: cantidad de dig de la ip 9:cantidad de la ip
	string_append(&mensaje,"127.0.0.1");
	string_append(&mensaje,"146000");//1:cantidad de dig del puerto 4: cantidad del puerto 6000:cantidad del puerto
	string_append(&mensaje,"18");
	string_append(&mensaje,"Bloque30");
	string_append(&mensaje,"213");
	string_append(&mensaje,"resultado.txt");


	string_append(&mensaje,"\0");
	EnviarDatos(socket, mensaje,strlen(mensaje));

}

void reciboOk(char *buffer){

	//2318Bloque3015NodoA
	char *bloque=string_new();
	char *nodo=string_new();
	int pos=2;
	bloque=DigitosNombreArchivo(buffer,&pos);
	nodo=DigitosNombreArchivo(buffer,&pos);
	printf("Recibo OK del Job:   "COLOR_VERDE"%s"DEFAULT"--"COLOR_VERDE"%s\n"DEFAULT,bloque,nodo);

}


void implementoJob(int *id,char * buffer,int * cantRafaga,char ** mensaje, int socket){



	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	printf("RAFAGA:%d\n",tipo_mensaje);
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case RECIBIR_ARCHIVO:
			AtiendeJob(id,buffer,cantRafaga);
			*cantRafaga=0;
			ObtenerInfoDeNodos(*id);
			Planificar(*id);
			enviarPlanificacionAJob(*id,socket);
			*cantRafaga=1;
			break;
		case NOTIFICACION_NODO:
			break;
		case RECIBIDO_OK:
			reciboOk(buffer);
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

int AtiendeCliente(void * arg) {
	int socket = (int) arg;
	int id=-1;
	cantHilos++;
	printf("Hilo numero:%d\n",cantHilos);

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
				implementoJob(&id,buffer,&cantRafaga,&mensaje,socket);

				break;
			case ES_FS:
				printf("implementar atiendeFS\n");
				//implementoFS(buffer,&cantRafaga,&mensaje);
				break;
			case COMANDO:
				printf("Muestre toda la lista de Archivos:\n");
				RecorrerArchivos();
				mensaje = "Ok";
				break;
			case COMANDOBLOQUES:
				printf("Muestre toda la lista de Bloques:\n");
				RecorrerListaBloques();
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


