/*
 * filesystemdev.c
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#include "filesystem.h"
#include "mongodev.h"

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
	//printf("Cantidad de digitos de Tamanio de Ip:%d\n",digitosCantNumIp);
	tamanioDeIp=ObtenerTamanio(buffer,3,digitosCantNumIp);
	//printf("Tamaño de IP:%d\n",tamanioDeIp);
	if(tamanioDeIp>=10){
		posActual=digitosCantNumIp;
	} else {
		posActual=1+digitosCantNumIp;
	}
	la_Ip=DigitosNombreArchivo(buffer,&posActual);
	//printf("Ip:%s\n",la_Ip);
	el_Puerto=DigitosNombreArchivo(buffer,&posActual);
	//printf("Puerto:%s\n",el_Puerto);
	tamanioDatos=DigitosNombreArchivo(buffer,&posActual);
	//printf("Tamaño:%s\n",tamanioDatos);

	el_nodo = buscarNodo(la_Ip,el_Puerto);

	if(el_nodo==NULL){
		free(nombre);
		nombre = string_new();
		if(letra>'Z'){
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

int existeArchivo(char* nArchivo, t_archivo** archivo,int padre){

	*archivo = buscarArchivoPorNombre(nArchivo,padre);

	if(*archivo==NULL){
		return 0;
	}
	return 1;
}

int validarDirectorios(char* archivoConPath,char**nArchivo){
	char** array;
	int listo=0,i=0,j=0,valido=1,tamanioArray,correcto=0;

	array = string_split(archivoConPath,"/");

	while(array[i]!=NULL){
		i++;
	}

	tamanioArray=i;
	i=0;

	if(tamanioArray<2){
		*nArchivo = strdup(array[0]);
		return 0;
	}

	while(!listo){
		while(valido&&(j<(tamanioArray-1))){
			i = validarDirectorio(array[j],i);
			//printf("LA I:%d\n",i);
			if(i!=0){
				listo=0;
				j++;
				if(j==(tamanioArray-1)){
					valido = 0;
					listo=1;
					correcto=1;
				}
			} else {
				listo=1;
				valido=0;
			}
		}
	}
	if(correcto==0){
		return -1;
	}
	*nArchivo = strdup(array[tamanioArray-1]);
	return i;
}

int AtiendeMarta(char* buffer,int*cantRafaga,char** bufferE){

	char *nArchivo;
	char *archivoConPath;
	t_list * nodos = list_create();
	int digitosCantDeArchivos=0,cantDeArchivos=0;
	int x,posActual=0,cont,i,k;
	t_archivo * el_archivo = malloc(sizeof(t_archivo));
	t_bloque * el_bloque = malloc(sizeof(t_bloque));
	t_nodo * el_nodo = malloc(sizeof(t_nodo));

	//BUFFER RECIBIDO = 4270 (EJEMPLO)
	//BUFFER RECIBIDO = 4112220temperatura-2012.txt220temperatura-2013.txt
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 411
	//printf("BUFFER:%s\n",buffer);
	digitosCantDeArchivos=PosicionDeBufferAInt(buffer,2);
	//printf("CANTIDAD DE DIGITOS:%d\n",digitosCantDeArchivos);
	cantDeArchivos=ObtenerTamanio(buffer,3,digitosCantDeArchivos);
	//printf("Cantidad de Archivos:%d\n",cantDeArchivos);
	posActual=3+digitosCantDeArchivos;

	string_append(&*bufferE,"12"); // 1 es FS y 2 es el tipo de mensaje
	cont = cuentaDigitos(cantDeArchivos); //me cuenta los digitos que tiene la cantidad de Archivos
	string_append(&*bufferE,string_itoa(cont)); //agrego la cantidad de digitos al buffer
	string_append(&*bufferE,string_itoa(cantDeArchivos));//agrego la cantidad de archivos

	for(x=0;x<cantDeArchivos;x++){

		int padre;

		archivoConPath=DigitosNombreArchivo(buffer,&posActual);

		padre=validarDirectorios(archivoConPath,&nArchivo);
		//printf(COLOR_VERDE"PADRE:%d\n"DEFAULT,padre);
		if(nArchivo!=NULL){
			//printf("NOMBRE:%s\n",nArchivo);
		}
		//printf("PADRE:%d ARCHIVO:%s\n",padre,nArchivo);
		if(padre!=-1){
			if(!existeArchivo(nArchivo,&el_archivo,padre)){
				return 0;
			}else {
				string_append(&*bufferE,obtenerSubBuffer(el_archivo->nombreArchivo));//aca va la ruta, ahora no lo hace
				cont = cuentaDigitos(list_size(el_archivo->listaBloques));
				string_append(&*bufferE,string_itoa(cont));
				string_append(&*bufferE,string_itoa(list_size(el_archivo->listaBloques)));
				for(i=0;i<list_size(el_archivo->listaBloques);i++){
					el_bloque = list_get(el_archivo->listaBloques,i);
					for(k=0;k<3;k++){
						if((el_bloque->array+k)!=NULL){
							string_append(&*bufferE,obtenerSubBuffer(el_bloque->array[k].nombreNodo));
							bool _true(void *elem){
								return (!strcmp((char*)elem,el_bloque->array[k].nombreNodo));
							}
							if(!list_any_satisfy(nodos,_true)){
								list_add(nodos,el_bloque->array[k].nombreNodo);
							}
							string_append(&*bufferE,obtenerSubBuffer(el_bloque->array[k].nro_bloque));
						}
					}
				}
			}

		}
	}
	cont = cuentaDigitos(list_size(nodos)); //me cuenta los digitos que tiene la cantidad de nodos
	string_append(&*bufferE,string_itoa(cont)); //agrego la cantidad de digitos al buffer
	string_append(&*bufferE,string_itoa(list_size(nodos)));//agrego la cantidad de nodos

	for(i=0;i<list_size(nodos);i++){
		el_nodo = buscarNodoPorNombre(list_get(nodos,i));
		string_append(&*bufferE,obtenerSubBuffer(el_nodo->nombre));
		string_append(&*bufferE,obtenerSubBuffer(el_nodo->ip));
		string_append(&*bufferE,obtenerSubBuffer(el_nodo->puerto));
	}
	return 1;
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return PosicionDeBufferAInt(buffer, 0);
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

int EnviarInfoMarta(int socket,char * buffer) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);
	int cont,cantidadDeBytesAEnviar;
	int bytecount,bytesRecibidos,cantRafaga=1,tamanio;

	char*bufferR = string_new();
	char*bufferE = string_new();

	cantidadDeBytesAEnviar = strlen(buffer);
	cont = cuentaDigitos(cantidadDeBytesAEnviar);
	string_append(&bufferE,"1");
	string_append(&bufferE,string_itoa(cont));
	string_append(&bufferE,string_itoa(cantidadDeBytesAEnviar));

	//Primera Rafaga para Marta
	if ((bytecount = send(socket, bufferE, strlen(bufferE), 0)) == -1)
		log_info(logger,"No puedo enviar información al cliente. Socket: %d", socket);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
		(char*) bufferE);

	bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);

	if(strcmp(bufferR,"Ok")==0){
		//Segunda Rafaga para Marta
		if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1)
			log_info(logger,"No puedo enviar información al cliente. Socket: %d", socket);
		log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			(char*) buffer);
		bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);

	} else {
		log_info(logger,"Marta tuvo algun problema con la rafaga 1. Socket: %d", socket);
		return 0;
	}
	return bytecount;
}

void implementoMarta(int *id,char * buffer,int * cantRafaga,char ** mensaje, int socket){

	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	//printf("RAFAGA:%d\n",tipo_mensaje);
	char*bufferE=string_new();
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case CONSULTA_ARCHIVO:
			if(AtiendeMarta(buffer,cantRafaga,&bufferE)){
				EnviarInfoMarta(socket,bufferE);
			} else {
				EnviarInfoMarta(socket,"No");
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

void implementoNodo(char * buffer,int * cantRafaga,char ** mensaje, int socket){

	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	//printf("RAFAGA:%d\n",tipo_mensaje);
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
	char *bufferAux = malloc(1);
	memset(bufferAux,0,1);
	int digTamanio;
	if (buffer != NULL ) {
		free(buffer);
	}

	if(*cantRafaga==1){
		bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char));
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
			log_info(logger,"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}

		digTamanio=PosicionDeBufferAInt(bufferAux,1);
		*tamanio=ObtenerTamanio(bufferAux,2,digTamanio);


	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char)+1);
		memset(bufferAux, 0, *tamanio * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

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

	//printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		log_info(logger,"No puedo enviar información a al clientes. Socket: %d", socket);
	//printf("Cuanto Envie:%d\n",bytecount);
	//Traza("ENVIO datos. socket: %d. buffer: %s", socket, (char*) buffer);

	//char * bufferLogueo = malloc(5);
	//bufferLogueo[cantidadDeBytesAEnviar] = '\0';

	//memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
	if(strlen(buffer)<50){
		log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,(char*) buffer);
	} else {
		log_info(logger, "ENVIO DATOS. socket: %d. Tamanio:%d ",socket,strlen(buffer));
	}

	return bytecount;
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
}

void RecorrerNodosYBloques(){
	t_nodo * el_nodo;
	t_array_nodo* array;
	int i=0,j,posicion;
	while(i<list_size(lista_nodos)){
		el_nodo = list_get(lista_nodos, i);
		printf("Nodo:"COLOR_VERDE "%s\n"DEFAULT,el_nodo->nombre);
		printf("La IP:"  COLOR_VERDE"%s\n"DEFAULT,el_nodo->ip);
		printf("El Puerto:"COLOR_VERDE"%s\n"DEFAULT,el_nodo->puerto);
		printf("El tamaño:"COLOR_VERDE"%s\n"DEFAULT,el_nodo->tamanio);
		printf("Estado:"COLOR_VERDE "%d\n"DEFAULT,el_nodo->estado);
		posicion = buscarNodoEnArrayPorNombre(el_nodo->nombre);
		j=1;
		while(j<list_size(arrayNodos[posicion])){
			array = list_get(arrayNodos[posicion],j);
			printf("Bloque:"COLOR_VERDE "%s\n"DEFAULT,array->nombre);
			printf("Nombre Archivo:"COLOR_VERDE "%s\n"DEFAULT,array->nombreArchivo);
			printf("Bloque Archivo:"COLOR_VERDE "%s\n"DEFAULT,array->bloqueArchivo);
			printf("Padre:"COLOR_VERDE "%d\n"DEFAULT,array->padre);
			j++;
		}
		i++;
	}
}

void RecorrerListaBloques(){
	t_archivo * el_archivo;
	t_bloque * el_bloque;

	int i=0;
	int j=0;

	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		printf("El archivo:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);

		while(j<list_size(el_archivo->listaBloques)){
			el_bloque = list_get(el_archivo->listaBloques, j);
			printf("%d :: ",el_bloque->bloque);
			printf("Copia1:\n");
			printf("%s--",el_bloque->array[0].nombreNodo);
			printf("%s  ",el_bloque->array[0].nro_bloque);
			printf("Copia2:\n");
			printf("%s--",el_bloque->array[1].nombreNodo);
			printf("%s  ",el_bloque->array[1].nro_bloque);
			printf("Copia3:\n");
			printf("%s--",el_bloque->array[2].nombreNodo);
			printf("%s  \n",el_bloque->array[2].nro_bloque);
			j++;
		}
		j=0;
		i++;
	}

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
	t_list* lista,*listaArchivos;
	t_filesystem * fs = malloc(sizeof(t_filesystem));
	t_archivo* arch = malloc(sizeof(t_archivo));
	int i=0,l;
	bool _true(void *elem){
		return ((t_filesystem*) elem)->padre==padre;
	}
	if (lista_filesystem==NULL){
		lista_filesystem=list_create();
	}
	lista = list_filter(lista_filesystem,_true);
	bool _true2(void *elem){
			return ((t_archivo*) elem)->padre==padre;
	}
	if (lista_archivos==NULL){
		lista_archivos = list_create();
	}
	listaArchivos = list_filter(lista_archivos,_true2);
	//printf("CANTIDAD LISTA FILESYSTEM FILTRADA:%d\n",list_size(lista));

	if(list_size(listaArchivos)>0) for(l=0;l<impre;l++) printf(" ");

	for(l=0;l<list_size(listaArchivos);l++){
		arch = list_get(listaArchivos,l);
		printf(COLOR_VERDE" %s\n"DEFAULT,arch->nombreArchivo);
	}


	if(list_size(lista)>0){
		//printf("Index:%d Nombre:%s Padre:%d\n",fs->index,fs->directorio,fs->padre);
		impre++;
		//printf("CANTIDAD LISTA FILESYSTEM FILTRADA:%d\n",list_size(lista));
		//printf("LA I:%d\n",i);
		while(i<list_size(lista)){
			fs = list_get(lista,i);
			//printf("PADRE:%d\n",fs->padre);
			if(fs->padre==0){
				printf("/%s\n",fs->directorio);
			} else {
				for(l=0;l<impre;l++) printf(" ");
				printf("/%s\n",fs->directorio);
			}
			recursiva(fs->index);
			i++;
		}
	} else {
		impre--;
	}
}

void mostrarFilesystem(){
	printf("Raiz(/)\n");
	int i = list_size(lista_filesystem);
	int j = list_size(lista_archivos);
	//printf(COLOR_VERDE"CANTIDAD DE FILESYSTEM:%d\n"DEFAULT,i);
	if(i>0||j>0){
		recursiva(0);
		impre=0;
	}
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
				//printf("implementar atiendeNodo\n");
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
			case COMANDO2:
				printf("Muestre toda la lista de Nodos y sus Bloques Ocupados:\n");
				RecorrerNodosYBloques();
				mensaje = "Ok";
				break;
			case 6:
				RecorrerListaBloques();
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
		log_info(logger,
				"No se pudo inicializar el socket que escucha a los clientes");

	if (setsockopt(socket_host, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		log_info(logger,"Error al hacer el 'setsockopt'");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(fs_Puerto);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY );
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		log_info(logger,"Error al hacer el Bind. El puerto está en uso");

	if (listen(socket_host, 10) == -1) // el "10" es el tamaño de la cola de conexiones.
		log_info(logger,
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
			log_info(logger,"ERROR AL ACEPTAR LA CONEXIÓN DE UN CLIENTE");
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

void cargarListaBloquesDisponibles(t_nodo* nodo){
	int i,cantBloques = atoi(nodo->tamanio)/TAMANIO_BLOQUE;//controla que sea entero
	//printf("Cantidad Bloques:%d",cantBloques);
	t_bloque_disponible * bloque;
	for(i=0;i<cantBloques;i++){
		bloque = bloque_disponible_create(i);
		list_add(nodo->bloquesDisponibles,bloque);
	}
}

int agregarNodo(){
	char ipNodo[TAMANIO_IP];
	char puertoNodo[5];


	t_nodo *el_nodo;

	printf("Ingrese la ip del nodo: ");
	scanf("%s",ipNodo);
	fflush(stdin);
	printf("Ingrese el puerto de escucha del nodo: ");
	scanf("%s",puertoNodo);
	fflush(stdin);
	el_nodo = buscarNodo(ipNodo,puertoNodo);

	if(el_nodo!=NULL){
		if(el_nodo->estado==-1){
			printf("\nVolvio a habilitarse el %s !\n",el_nodo->nombre);
			el_nodo->estado = 1;
		} else {
			printf("\n%s Habilitado!\n",el_nodo->nombre);
			el_nodo->estado = 1;
			cargarListaBloquesDisponibles(el_nodo);
		}
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

	//printf("El Tamaño del archivo es:%lu\n",*tamanio);

	if(!tamanio){
		printf("No se pudo abrir correctamente el archivo:%s\n",nombreArchivo);
	}

	int j=0,padre=0,k;
	while(!correcto){
		mostrarFilesystem();
		printf("Ingrese de un directorio: ejemplo: home\n");
		printf("Path Ingresados:%s\n",path);
		printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
		scanf("%s",directorio);
		fflush(stdin);
		//printf("PADRE INICIO:%d\n",padre);
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
			//printf("PADRE FIN:%d\n",padre);
		}
		//system("clear");
	}
	//printf("PADRE POSTA:%d\n",padre);
	archivo = archivo_create(nombreArchivo,*tamanio,padre,1);
	list_add(lista_archivos,archivo);
	return 1;
}

int cargarArchivo(int *padre,FILE ** fArchivo,char**nombreArchivo){

	char pathConArchivo[100];
	*nombreArchivo = (char*)malloc(sizeof(char)*30);
	char archivoSalida[50];

	mostrarFilesystem();

	printf("Ingrese la ruta completa con el nombre del archivo para recuperar\n ");
	printf("Ejemplo: /home/utnso/temperatura.txt\n");
	scanf("%s",pathConArchivo);
	fflush(stdin);

	*padre = validarDirectorios(pathConArchivo,nombreArchivo);
	if(*padre!=-1){
		printf("Ingrese el nombre para guardar este archivo:\n");
		scanf("%s",archivoSalida);
		fflush(stdin);

		if(( *fArchivo = fopen(archivoSalida, "w") ) == NULL){
			//Si no se pudo abrir, imprimir el error y abortar;
			fprintf(stderr, "Error al abrir el archivo '%s': %s\n", *nombreArchivo, strerror(errno));
			return 0;
		}
		return 1;
	} else {
		return 0;
	}
}


int buscarNodoEnArrayPorNombre (char* nombre){

    int k=0;
    int cantNodos=list_size(lista_nodos);
    t_array_nodo *el_array_nodo;

    for(k=0;k<cantNodos;k++){
        if(list_size(arrayNodos[k])>0){
            el_array_nodo=list_get(arrayNodos[k],0);
            if(strcmp(el_array_nodo->nombre,nombre)==0 ){
            	return k;
            }
        }
    }
    return 0;
}


int buscarNodoEnArray (int bloque){

    int i=0,k=0,bandera=0;
    int cantNodos=list_size(lista_nodos);
    t_array_nodo *el_array_nodo;

    char *bloqueChar = string_new();
    string_append(&bloqueChar,"bloque");
    string_append(&bloqueChar,string_itoa(bloque));

    for(k=0;k<cantNodos;k++){
        if(list_size(arrayNodos[k])>0){
            i=1;
            while(i<list_size(arrayNodos[k])){
                el_array_nodo=list_get(arrayNodos[k],i);
                //printf("COMPARACION: Array:%s Parametro:%s\n",el_array_nodo->bloqueArchivo,bloqueChar);
                if(strcmp(el_array_nodo->nombreArchivo,archivo->nombreArchivo)==0 && el_array_nodo->padre==archivo->padre
                		&& strcmp(el_array_nodo->bloqueArchivo,bloqueChar)==0){
                    bandera=1;
                    i=list_size(arrayNodos[k]);
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
    return -1;
}

void imprimirArrayNodos(){
	int i=0;
	t_array_nodo *el_array_nodo;

	for(i=0;i<list_size(lista_nodos);i++){
    	if(list_size(arrayNodos[i])>0){
    		el_array_nodo = list_get(arrayNodos[i],0);
    		printf("%s\n",el_array_nodo->nombre);
    	}
    }
}

void llenarArrayDeNodos (){

    arrayNodos=(t_list**)malloc(list_size(lista_nodos)*sizeof(t_array_nodo));

    int i=0,k=0;
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
                k=list_size(lista_nodos);
            }

        }


        if(el_array_nodo == NULL){

        	for(k=0;k<list_size(lista_nodos);k++){

        		if(list_size(arrayNodos[k])==0){
        			list_add(arrayNodos[k],array_nodo_create(el_nodo->nombre));
        			k=list_size(lista_nodos);
        		}
        	}

        }

    }
    imprimirArrayNodos();
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
	imprimirArrayNodos();
	//sleep(5);
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
    int bandera=0;

    while(i<list_size(lista_nodos)){
        el_nodo=list_get(lista_nodos,i);
        if(el_nodo->estado==1){
			printf("Nro (%d)\n",i);
			printf("%s\n",el_nodo->nombre);
			printf("IP: %s\n",el_nodo->ip);
			printf("Puerto: %s\n",el_nodo->puerto);
			bandera=1;
        }
		i++;
    }
    if(bandera==1){
    	printf("Elija el Nro de nodo a eliminar: ");
    	scanf("%d",&eleccion);
    	if(eleccion>=0 && eleccion<i){
    		el_nodo=list_get(lista_nodos,eleccion);
    		el_nodo->estado=-1;
    	}
    } else {
    	printf("No hay nodos disponibles para eliminar\n");
    }
    i=0;
    while(i<list_size(lista_nodos)){
    	el_nodo=list_get(lista_nodos,i);
    	if(el_nodo->estado==1){
			printf("Nro (%d)\n",i);
			printf("%s\n",el_nodo->nombre);
			printf("IP: %s\n",el_nodo->ip);
			printf("Puerto: %s\n",el_nodo->puerto);
    	}
        i++;
    }

}


void enviarBufferANodo(t_envio_nodo* envio_nodo){
	int socket,bytesRecibidos,cantRafaga=1,tamanio=10;
	char* buffer1,*buffer2,*bufferR;
	buffer1=string_new();
	buffer2=string_new();
	bufferR=string_new();
	char *aux=string_new();
	conectarNodo(&socket,envio_nodo->ip,envio_nodo->puerto);
	//Segunda Rafaga
	string_append(&buffer2,"13");
	aux=obtenerSubBuffer(string_itoa(envio_nodo->bloque));
	string_append(&buffer2,aux);
	free(aux);
	aux=string_new();
	aux=obtenerSubBuffer(envio_nodo->buffer);
	string_append(&buffer2,aux);
	free(aux);
	aux=string_new();

	//Primer Rafaga
	string_append(&buffer1,"13");
	aux=string_itoa(cuentaDigitos(strlen(buffer2)));
	string_append(&buffer1,aux);
	free(aux);
	aux=string_new();
	aux=string_itoa(strlen(buffer2));
	string_append(&buffer1,aux);
	free(aux);
	aux=string_new();
	//printf("PRIMERA RAFAGA:%s\n",buffer1);
	EnviarDatos(socket,buffer1,strlen(buffer1));
	bufferR=RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
	//EnviarDatos(socket,buffer2,strlen(buffer2));
	if(strcmp(bufferR,"1")==0){
		long unsigned len=0;
		len=strlen(buffer2);
		printf("LEN : %lu\n",len);
		if (sendall(socket, buffer2, &len) == -1) {
			printf("ERROR AL ENVIAR\n");
		}
	}
	free(buffer2);
	free(buffer1);
	free(envio_nodo->buffer);
	close(socket);
	//pthread_exit(NULL);
}

t_nodo* buscarNodoPorNombre(char* nombre){
	t_nodo* el_nodo = malloc(sizeof(t_nodo));
	bool _true(void *elem){
		return ((!strcmp(((t_nodo*) elem)->nombre,nombre)));
	}
	el_nodo = list_find(lista_nodos, _true);
	return el_nodo;
}

t_archivo* buscarArchivoPorNombre(char* nombre, int padre){
	t_archivo* el_archivo = malloc(sizeof(t_archivo));
	bool _true(void *elem){
		return ((!strcmp(((t_archivo*) elem)->nombreArchivo,nombre))&&(((t_archivo*) elem)->padre==padre));
	}
	el_archivo = list_find(lista_archivos, _true);
	return el_archivo;
}

t_archivo* buscarArchivoPorPadre(int padre){
	t_archivo* el_archivo = malloc(sizeof(t_archivo));
	bool _true(void *elem){
		return (((t_archivo*) elem)->padre==padre);
	}
	el_archivo = list_find(lista_archivos, _true);
	return el_archivo;
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

void agregarBloqueEnArrayNodos(int nroNodo, int bloqueDisponible,int bloque){
	t_array_nodo* nodo = malloc(sizeof(t_array_nodo));
	char * nombre = string_new();
	char * bloqueChar = string_new();
	string_append(&nombre,"bloque");
	string_append(&nombre,string_itoa(bloqueDisponible));
	string_append(&bloqueChar,"bloque");
	string_append(&bloqueChar,string_itoa(bloque));

	nodo->nombre = nombre;
	nodo->nombreArchivo = archivo->nombreArchivo;
	nodo->bloqueArchivo = bloqueChar;
	nodo->padre = archivo->padre;

	list_add(arrayNodos[nroNodo],nodo);
}

int funcionLoca(char* buffer,t_bloque ** bloque,int j){
	int nroNodo,bloqueDisponible;
	t_nodo* el_nodo;
	t_array_nodo* nodo;
	t_envio_nodo* envio_nodo;
	ordenarArrayNodos();
	nroNodo = buscarNodoEnArray((*bloque)->bloque);
	if(nroNodo!=-1){
		nodo = list_get(arrayNodos[nroNodo],0);
		el_nodo=buscarNodoPorNombre(nodo->nombre);
		//printf("Nodo:%s BloqueArchivo:%d\n",nodo->nombre,(*bloque)->bloque);
		bloqueDisponible = buscarBloqueDisponible(el_nodo);
		envio_nodo = envio_nodo_create(buffer,el_nodo->ip,el_nodo->puerto,bloqueDisponible);
		agregarBloqueEnArrayNodos(nroNodo,bloqueDisponible,(*bloque)->bloque);
		//printf("EL GRAN BLOQUE:%d",(*bloque)->bloque);

		int iThreadHilo = pthread_create(&hNodos, NULL,
				(void*) enviarBufferANodo, (void*) envio_nodo );
		if (iThreadHilo) {
			fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadHilo);
			exit(EXIT_FAILURE);
		}


		pthread_join(hNodos, NULL );

		char *nombre = string_new();
		string_append(&nombre,"Bloque");
		string_append(&nombre,string_itoa(bloqueDisponible));

		(*bloque)->array[j].nombreNodo=nodo->nombre;
		(*bloque)->array[j].nro_bloque=nombre;




		return 1;

	}
	log_info(logger, "No se pudo realizar la copia\n");
	return -1;
}

int enviarCopias(char*bufferAux,t_bloque ** bloque){
	int j;
	for(j=0;j<3;j++){
		if(funcionLoca(bufferAux,&*bloque,j)!=1){
			return -1;
		}
	}
	return 1;
}

int enviarBloque(char *bufferAux){
	t_bloque * bloque;
	tamanioTotal = tamanioTotal + strlen(bufferAux);
	bloque = bloque_create(nroBloque++);
	if(enviarCopias(bufferAux,&bloque)==1){
		list_add(archivo->listaBloques,bloque);
		return 1;
	} else {
		return -1;
	}
}

int recorrerArchivo(FILE *fArchivo){
	char * buffer,*bufferAux;
	buffer = malloc(TAMANIO_BLOQUE+1);
	memset(buffer,0,TAMANIO_BLOQUE+1);
	//memset(&buffer,0,TAMANIO_BLOQUE+1);
	long unsigned tamanio,tamanioA;
	fseek(fArchivo,0L,SEEK_END);
	tamanioA=ftell(fArchivo);
	rewind(fArchivo);
	int i=0,j=0;
	llenarArrayDeNodos();
	nroBloque=0;
	while(!feof(fArchivo)){
		fread(buffer,1,TAMANIO_BLOQUE,fArchivo);
		//printf("STRLEN BUFFER:%lu\n",strlen(buffer));
		for(j=0;j<TAMANIO_BLOQUE;j++){
			if(buffer[TAMANIO_BLOQUE-j]=='\n'){
				bufferAux = malloc(TAMANIO_BLOQUE-j+1);
				memset(bufferAux,0,TAMANIO_BLOQUE-j+1);
				memcpy(bufferAux,buffer,TAMANIO_BLOQUE-j);
				if(enviarBloque(bufferAux)){
					i++; //Contador de Bloques
					//printf(COLOR_VERDE"Contador de Bloques:%d\n"DEFAULT,i);
					//printf("LA J:%d\n",j);
					tamanio = ftell(fArchivo);
					if(tamanio!=tamanioA){
						fseek(fArchivo,-j,SEEK_CUR);
						//printf("TAMANIO:%lu\n",tamanio);
					}
					free(bufferAux);
					j=TAMANIO_BLOQUE;
				} else {
					return -1;
				}
			}
		}
		memset(buffer,0,TAMANIO_BLOQUE+1);

	}
	printf(COLOR_VERDE"TAMANIO TOTAL DEL ARCHIVO:%lu\n",tamanioTotal);
	//printf("Cantidad Bloques:%d\n",i);
	free(buffer);
	return 1;
}

int getBloque(int nroBloque,char* ip,char*puerto,char**buffer){
	char* bufferE,*bufferR;
	int cont,socket,tamanioE,bytesRecibidos,cantRafaga=1,tamanio;
	bufferE = string_new();
	bufferR = string_new();
	*buffer = string_new();
	//SOLO UNA RAFAGA
	string_append(&bufferE,"12");
	if(nroBloque == 0){
		string_append(&bufferE,"10");
	} else {
		cont = cuentaDigitos(nroBloque); //me cuenta los digitos que tiene el nro de bloque
		string_append(&bufferE,string_itoa(cont)); //agrego la cantidad de digitos del nro de bloque
		string_append(&bufferE,string_itoa(nroBloque));//agrego el numero de bloque
	}
	if(conectarNodo(&socket, ip, puerto)) {
		tamanioE = strlen(bufferE);
		if(tamanioE==EnviarDatos(socket,bufferE,tamanioE)) {
			bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
			printf("TAMANIO DE BUFFER:%d\n",tamanio);
			if(bufferR!=NULL){
				bufferE=string_new();
				string_append(&bufferE,"1");
				EnviarDatos(socket,bufferE,2);
				cantRafaga = 2;
				char *aux=malloc(tamanio+1);

				char *bloque=malloc(tamanio+1);

				memset(bloque,0,tamanio+1);

				char *recibido=string_new();
				memset(aux,0,tamanio+1);

				ssize_t numBytesRecv = 0;

				do{
					numBytesRecv = numBytesRecv + recv(socket, aux, tamanio, 0);
					if ( numBytesRecv < 0)
						printf("ERROR\n");
					string_append(&recibido,aux);
					strcat(bloque,recibido);
					free(recibido);
					recibido=string_new();
					printf("------ %d -----\n",strlen(bloque));
					memset(aux, 0, tamanio+1);

				}while (numBytesRecv <tamanio);
				printf("TAMANIO:%d NUMBYTESRECV:%d\n",tamanio,numBytesRecv);
				*buffer=malloc(tamanio+1);
				memset(*buffer,0,tamanio+1);
				memcpy(*buffer,bloque,tamanio);
				free(bloque);
				printf("BLOQUE:%d TAMANIO:%lu Recibido Ok\n",nroBloque,(long unsigned)strlen(*buffer));
				return 1;
			}
		}
	}
	return 0;
}

int pedirBloque(int nroBloque, t_array_copias copia,char** buffer){
	t_nodo * nodo;
	char** array = string_split(copia.nro_bloque,"e");
	nodo = buscarNodoPorNombre(copia.nombreNodo);
	if (nodo!=NULL){
		if(nodo->estado==1){
			if(getBloque(atoi(array[1]),nodo->ip,nodo->puerto,buffer)){
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int recuperarArchivo(FILE *fArchivo,int padre, char*nombre){
	char * buffer;
	int i=0,j=0;
	t_archivo* archivo = buscarArchivoPorNombre(nombre,padre);
	if(archivo!=NULL){
		t_list * listaBloques = archivo->listaBloques;
		if(listaBloques != NULL){
			i = list_size(listaBloques);
			for(j=0;j<i;j++){
				int l = 0;
				t_bloque * bloque = malloc(sizeof(t_bloque));
				bloque = list_get(listaBloques,j);
				while(!pedirBloque(bloque->bloque,bloque->array[l++],&buffer))
					if(l==3) return 0;
				fwrite(buffer,sizeof(char),strlen(buffer),fArchivo);
			}
			fclose(fArchivo);
			free(buffer);
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}


int procesarArchivo(){
	long unsigned tamanio;
	FILE * fArchivo;
	//char * buffer = malloc(100);

	if(subirArchivo(&tamanio,&fArchivo)){
		if(recorrerArchivo(fArchivo)){
			return 1;
		} else {
			return -1;
		}
	} else {
		return 0;
	}
}

void eliminarFilesystem(){
	int i = 0;
	while(i<list_size(lista_filesystem)){
		list_remove_and_destroy_element(lista_filesystem,i++,(void*)filesystem_destroy);
	}
	//free(lista_filesystem);
}

void eliminarArchivos(){
	int i = 0;
	while(i<list_size(lista_archivos)){
		list_remove_and_destroy_element(lista_archivos,i++,(void*)archivo_destroy);
	}
	//free(lista_archivos);
}

int crearDirectorio(){
	char directorio[20];
	char * path = string_new();
	int k,padre=0,correcto=0;
	t_filesystem * filesystem;

	while(!correcto){
		printf("Ingrese un directorio: ejemplo: home\n");
		printf("Path Ingresados:%s\n",path);
		printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
		scanf("%s",directorio);
		fflush(stdin);
		if(strcmp(directorio,"1")){
			if(!strcmp(directorio,"0")){
				padre=0;
				path=string_new();
			} else {
				k=validarDirectorio(directorio,padre);
				if(k!=0){
					padre=k;
					string_append(&path,"/");
					string_append(&path,directorio);
				}
			}
		} else {
			correcto=1;
		}
	}
	printf("Ingrese el nombre del directorio a crear\n");
	scanf("%s", directorio);
	filesystem = malloc(sizeof(t_filesystem));
	filesystem->index = indexGlobal++;
	filesystem->padre = padre;
	filesystem->directorio = strdup(directorio);
	bool _true(void *elem){
		return ((((t_filesystem*) elem)->padre==padre)&&(!strcmp(((t_filesystem*) elem)->directorio,directorio)));
	}
	int existe = list_any_satisfy(lista_filesystem,_true);
	if(existe==0){
		list_add(lista_filesystem,filesystem);
		mostrarFilesystem();
		return 1;
	} else {
		printf("Este directorio ya existe\n");
		return 0;
	}
}

int eliminarDirectorio(){
	char directorio[20];
	char * path = string_new();
	int k,padre=0,correcto=0,indeX=0;

	if(list_size(lista_filesystem)>0){
		while(!correcto){
			printf("Ingrese un directorio: ejemplo: home\n");
			printf("Path Ingresados:%s\n",path);
			printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
			scanf("%s",directorio);
			fflush(stdin);
			if(strcmp(directorio,"1")){
				if(!strcmp(directorio,"0")){
					padre=0;
					path=string_new();
				} else {
					k=validarDirectorio(directorio,padre);
					if(k!=0){
						padre=k;
						string_append(&path,"/");
						string_append(&path,directorio);
					}
				}
			} else {
				correcto=1;
			}
		}
		path = string_new();
		printf("Ingrese el nombre del directorio a eliminar\n");
		scanf("%s", directorio);
		fflush(stdin);

		if((indeX=validarDirectorio(directorio,padre))==0){
			printf("Nombre de directorio incorrecto.\n");
			return 0;
		}

		bool _true(void *elem){
			return ((t_filesystem*) elem)->padre==indeX;
		}
		bool _true3(void *elem){
			return ((t_archivo*) elem)->padre==indeX;
		}
		int cant = list_count_satisfying(lista_filesystem,_true);
		int cantf = list_count_satisfying(lista_archivos,_true3);
		if(cant>0||cantf>0){
			printf("Este directorio no se pudo eliminar porque no esta vacio\n");
			return 0;
		}
		bool _true2(void *elem){
			return ((((t_filesystem*) elem)->index==indeX)&&(!strcmp(((t_filesystem*) elem)->directorio,directorio)));
		}

		list_remove_and_destroy_by_condition(lista_filesystem,_true2,(void*)filesystem_destroy);
		printf("Se elimino el directorio correctamente\n");
		return 1;
	} else {
		printf("No existen archivos o directorios para eliminar\n");
		return 0;
	}
}

int moverDirectorio(){
	char directorio[20],destino[20];
	char * path = string_new();
	int k,padre=0,correcto=0,indeX=0,indexDestino;
	t_filesystem* fs;

	if(list_size(lista_filesystem)>0){
		while(!correcto){
			printf("Ingrese un directorio: ejemplo: home\n");
			printf("Path Ingresados:%s\n",path);
			printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
			scanf("%s",directorio);
			fflush(stdin);
			if(strcmp(directorio,"1")){
				if(!strcmp(directorio,"0")){
					path=string_new();
					padre=0;
				}else {
					k=validarDirectorio(directorio,padre);
					if(k!=0){
						padre=k;
						string_append(&path,"/");
						string_append(&path,directorio);
					}
				}
			} else {
				correcto=1;
			}
		}

		path = string_new();
		printf("Ingrese el nombre del directorio a mover\n");
		scanf("%s", directorio);
		fflush(stdin);

		if((indeX=validarDirectorio(directorio,padre))==0){
			printf("Nombre de directorio incorrecto.\n");
			return 0;
		}

		bool _true(void *elem){
			return ((((t_filesystem*) elem)->index==indeX)&&(!strcmp(((t_filesystem*) elem)->directorio,directorio)));
		}
		fs = list_find(lista_filesystem,_true);
		correcto=0;
		padre=0;
		k=0;
		if(fs != NULL){
			while(!correcto){
				printf("Ingrese un directorio: ejemplo: home\n");
				printf("Path Ingresados de destino:%s\n",path);
				printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
				scanf("%s",destino);
				fflush(stdin);
				if(strcmp(destino,"1")){
					if(!strcmp(destino,"0")){
						padre=0;
						path=string_new();
					} else {
						k=validarDirectorio(destino,padre);
						if(k!=indeX){
							if(k!=0){
								padre=k;
								string_append(&path,"/");
								string_append(&path,destino);
							}
						} else {
							printf("Esta intentando mover el directorio en si mismo. Error\n");
							return 0;
						}
					}
				} else {
					correcto=1;
				}
			}
			printf("Ingrese el directorio de destino o / si es raiz, directorio a mover:%s\n",directorio);
			scanf("%s", destino);
			fflush(stdin);
			if(strcmp(destino,"/")){
				indexDestino=validarDirectorio(destino,padre);
				if(indexDestino>0){
					fs->padre = indexDestino;
					printf("Se ha realizado el movimiento con exito!");
					return 1;
				} else {
					printf("Directorio ingresado invalido.\n");
					return 0;
				}
			} else {
				fs->padre = 0;
				printf("Se ha realizado el movimiento con exito!");
				return 1;
			}
		} else {
			printf("No se encontro el directorio, Error!\n");
			return 0;
		}
	} else {
		printf("No hay directorios para mover.\n");
		return 0;
	}
}

int renombrarDirectorio(){
	char directorio[20],destino[20];
	char * path = string_new();
	int k,padre=0,correcto=0,indeX=0;
	t_filesystem* fs;

	if(list_size(lista_filesystem)>0){
		while(!correcto){
			printf("Ingrese un directorio: ejemplo: home\n");
			printf("Path Ingresados:%s\n",path);
			printf("Ingrese directorio o 1 para confirmar o 0 volver a empezar: ");
			scanf("%s",directorio);
			fflush(stdin);
			if(strcmp(directorio,"1")){
				if(!strcmp(directorio,"0")){
					padre=0;
					path=string_new();
				} else {
					k=validarDirectorio(directorio,padre);
					if(k!=0){
						padre=k;
						string_append(&path,"/");
						string_append(&path,directorio);
					}
				}
			} else {
				correcto=1;
			}
		}
		printf("Ingrese el nombre del directorio a renombrar\n");
		scanf("%s", directorio);
		fflush(stdin);

		if((indeX=validarDirectorio(directorio,padre))==0){
			printf("Nombre de directorio incorrecto.\n");
			return 0;
		}

		bool _true2(void *elem){
			return ((((t_filesystem*) elem)->padre==padre)&&(!strcmp(((t_filesystem*) elem)->directorio,destino)));
		}

		bool _true(void *elem){
			return ((((t_filesystem*) elem)->index==indeX)&&(!strcmp(((t_filesystem*) elem)->directorio,directorio)));
		}
		fs = list_find(lista_filesystem,_true);

		if(fs != NULL){
			printf("Ingrese el nuevo nombre para el directorio:%s:\n",directorio);
			scanf("%s", destino);
			fflush(stdin);
			int cant = list_any_satisfy(lista_filesystem,_true2);
			if(cant==0){
				printf("Se ha renombrado el directorio con exito.\n");
				fs->directorio = strdup(destino);
				return 1;
			} else {
				printf("Ya existe un directorio con este nombre.\n");
				return 0;
			}
		} else {
			printf("No se encontro el directorio, Error!\n");
			return 0;
		}
	} else {
		printf("No hay directorios para renombrar.\n");
		return 0;
	}
}


int manejoDeDirectorios(){
	int seleccion;
	mostrarFilesystem();
	printf("Ingrese un comando:\n");
	printf("1: Crear 2: Eliminar 3: Renombrar 4: Mover\n");
	scanf("%d", &seleccion);
	switch(seleccion){
		case 1: printf("Se eligio Crear\n");
				return crearDirectorio();
				break;
		case 2: printf("Se eligio Eliminar\n");
				return eliminarDirectorio();
				break;
		case 3: printf("Se eligio Renombrar\n");
				return renombrarDirectorio();
				break;
		case 4: printf("Se eligio Mover\n");
				return moverDirectorio();
				break;
		default: printf("Se ha ingresado un comando incorrecto.\n");
				break;
	}
	return 0;
}

int armarArchivoPorHilo(){

	int valor;

	int iThreadHilo = pthread_create(&hNodos, NULL,
					(void*) armarArchivo, NULL );
	if (iThreadHilo) {
		fprintf(stderr,
		"Error al crear hilo - pthread_create() return code: %d\n",
		iThreadHilo);
		exit(EXIT_FAILURE);
	}

	pthread_join(hNodos, (void*)&valor );

	return valor;
}

int armarArchivo(){
	int padre;
	FILE * fArchivo;
	char* nombre;
	//char * buffer = malloc(100);

	if(cargarArchivo(&padre,&fArchivo,&nombre)){
		if(recuperarArchivo(fArchivo,padre,nombre)){
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

void eliminarListaNodos(){
	int i = 0;
	while(i<list_size(lista_nodos)){
		list_remove_and_destroy_element(lista_filesystem,i++,(void*)filesystem_destroy);
	}
	//free(lista_filesystem);
}

int formatearFs(){
	indexGlobal=1;
	eliminarFilesystem();
	printf("Se elimino la estructura del filesystem\n");
	eliminarArchivos();
	printf("Se eliminaron todos los archivos\n");
	eliminarListaNodos();
	printf("Se eliminaron todos los Nodos\n");
	return 1;
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

	//system("clear");
	switch (variable_seleccion) {

	case 0:
		log_info(logger, "Terminando el programa");
		return 0;
		break;
	case 1:
		if(formatearFs()){
			log_info(logger, "Se Formateo el MDFS con exito!\n");
		} else {
			log_info(logger, "No se pudo Formatear el MDFS...\n");
		}
		break;
	case 2:
		log_info(logger, "Se realizo Eliminar/Renombrar/Mover Archivos\n");
		break;
	case 3:
		if(manejoDeDirectorios()){
			log_info(logger, "Se realizo Crear/Eliminar/Renombrar/Mover Directorios\n");
		} else {
			log_info(logger,"No se pudo realizar Crear/Eliminar/Renombrar/Mover Directorios\n");
		}
		break;
	case 4:
		if(procesarArchivo()==1){
			log_info(logger, "Se realizo Copiar un archivo local al MDFS\n");
		} else {
			log_info(logger, "No se realizo Copiar un archivo local al MDFS\n");
		}
		break;
	case 5:
		if(armarArchivoPorHilo()==1){
			log_info(logger, "Se realizo Copiar un archivo del MDFS al filesystem local\n");
		} else {
			log_info(logger, "No se pudo Copiar un archivo del MDFS al filesystem local\n");
		}
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
	new->nro_bloque = string_itoa(bloque);
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
    new->buffer=strdup(buffer);
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

int sendall(int s, char *buf, long unsigned *len){
	long unsigned total = 0; // cuántos bytes hemos enviado
	long unsigned bytesleft = *len; // cuántos se han quedado pendientes
	long unsigned n;
	while(total < *len) {
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1){
			break;
		}
		total += n;
		bytesleft -= n;
		//printf("Cantidad Enviada :%lu\n",n);
	}
	*len = total; // devuelve aquí la cantidad enviada en realidad
	return n==-1?-1:0;	// devuelve -1 si hay fallo, 0 en otro caso
}

void mongo_db_nodos_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	nodosMongo = mongoc_client_get_collection (client, "test", "MDFS_NODOS");
}

void mongo_db_nodos_close(){
	mongoc_collection_destroy (nodosMongo);
	mongoc_client_destroy (client);
}

void armar_nodo_mongo(t_nodo* el_nodo, bson_t** nodo, bson_t** bloquesDisp){
	bson_append_utf8((void*)nodo, "nombre", 6, el_nodo->nombre, strlen(el_nodo->nombre));
		bson_append_utf8((void*)nodo, "ip", 2, el_nodo->ip, strlen(el_nodo->ip));
		bson_append_utf8((void*)nodo, "puerto", 2, el_nodo->puerto, strlen(el_nodo->puerto));
		bson_append_utf8((void*)nodo, "tamanio", 2, el_nodo->tamanio, strlen(el_nodo->tamanio));
		bson_append_int32((void*)nodo, "estado", 6, el_nodo->estado);
		bson_append_array_begin((void*)nodo, "bloquesDisponibles", 18, (void*)bloquesDisp);
		int i;
		for (i = 0; i < list_size(el_nodo->bloquesDisponibles); i++) {

			t_bloque_disponible* el_bloque = list_get(el_nodo->bloquesDisponibles, i); //Chequear si tiene que arrancar en 0 para el primer elemento


			bson_append_int32((void*)bloquesDisp, "bloque", 6, el_bloque->bloque);

		}
		bson_append_array_end((void*)nodo, (void*)bloquesDisp);
}


bool crear_nodo_mongo(t_nodo* el_nodo) {
    bool res;
    bson_t *nodo = bson_new();
    bson_t *bloquesDisp = bson_new();
    bson_error_t error;

    armar_nodo_mongo(el_nodo,&nodo, &bloquesDisp);

	res= mongoc_collection_insert (nodosMongo, MONGOC_INSERT_NONE, nodo, NULL, &error);
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(bloquesDisp);

	bson_destroy(nodo);

	return res;
}

bool borrar_nodo_mongo(t_nodo* el_nodo){
	bool res;
	    bson_t *nodo = bson_new();
	    bson_t *bloquesDisp = bson_new();
	    bson_error_t error;

	    armar_nodo_mongo(el_nodo,&nodo, &bloquesDisp);
	    res = mongoc_collection_remove (nodosMongo,MONGOC_DELETE_SINGLE_REMOVE,nodo,NULL,&error);

	    bson_destroy(bloquesDisp);
	    bson_destroy(nodo);

	    return res;

}

int grabar_nodo_mongo(t_nodo* el_nodo_viejo, t_nodo* el_nodo_nuevo) {
    int res;

    bson_t *nodoViejo = bson_new();
    bson_t *bloquesDispViejo = bson_new();
    bson_t *nodoNuevo = bson_new();
    bson_t *bloquesDispNuevo = bson_new();

    armar_nodo_mongo(el_nodo_viejo,&nodoViejo, &bloquesDispViejo);
    armar_nodo_mongo(el_nodo_nuevo,&nodoNuevo, &bloquesDispNuevo);


    res = mongoc_collection_update(nodosMongo, MONGOC_UPDATE_UPSERT, nodoViejo, nodoNuevo, NULL, NULL);
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(bloquesDispViejo);
    bson_destroy(bloquesDispNuevo);
	bson_destroy(nodoNuevo);
	bson_destroy(nodoViejo);

	return res;
}

int leer_nodo_mongo(){
//t_archivo_json *leerArchivoMongo(bson_t *query)

	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;
	bson_iter_t iter, child;
	uint32_t array_len;
	const uint8_t    *array;
	t_nodo *nodoMongo = malloc(sizeof (t_nodo));
	t_bloque_disponible* el_bloque = malloc(sizeof(t_bloque_disponible));


	mongo_db_nodos_open();

	query  = bson_new ();

	cursor = mongoc_collection_find (nodosMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){
		        if(bson_iter_find(&iter,"nombre")){
		        	nodoMongo->nombre=strdup(bson_iter_utf8(&iter,NULL));
		           	}
		        if(bson_iter_find(&iter,"ip")){
		        	nodoMongo->ip=strdup(bson_iter_utf8(&iter,NULL));
		        	}
		        if(bson_iter_find(&iter,"puerto")){
		        	nodoMongo->puerto=strdup(bson_iter_utf8(&iter,NULL));
		        	}
		        if(bson_iter_find(&iter,"tamanio")){
		        	nodoMongo->tamanio=strdup(bson_iter_utf8(&iter,NULL));
		        	}
		        if(bson_iter_find(&iter,"estado"))
		        	nodoMongo->estado = (int32_t)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"bloquesDisponibles")){
		        	nodoMongo->bloquesDisponibles=list_create();
		        	 bson_iter_array (&iter, &array_len,&array);
		        	 bson_iter_recurse(&iter, &child);
		        	 while(bson_iter_next(&child)){
		        		 if(bson_iter_find(&child,"bloque"))
		        		 el_bloque->bloque = (int32_t)bson_iter_int32(&iter);

		        		 list_add(nodoMongo->bloquesDisponibles,el_bloque);
		        	 }

		        }


		    }

		}
	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_nodos,nodoMongo);
	    list_destroy(nodoMongo->bloquesDisponibles); //Limpio la lista bloques para el proximo archivo a cargar.
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	free(nodoMongo);

	return list_size(lista_nodos);
}



void mongo_db_filesystem_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	filesystemMongo = mongoc_client_get_collection (client, "test", "MDFS_FILESYSTEM");
}

void mongo_db_filesystem_close(){
	mongoc_collection_destroy (filesystemMongo);
	mongoc_client_destroy (client);
}

void armar_filesystem_mongo(t_filesystem* el_fs, bson_t** filesystem){
	bson_append_int32((void*)filesystem, "index", 5, el_fs->index);

		bson_append_utf8((void*)filesystem, "directorio", 10, el_fs->directorio, strlen(el_fs->directorio));

		bson_append_int32((void*)filesystem, "padre", 5, el_fs->padre);
}

bool crear_filesystem_mongo(t_filesystem* el_fs) {
    bool res;

    bson_t *filesystem = bson_new();
    bson_error_t error;
    armar_filesystem_mongo(el_fs,&filesystem);

    res= mongoc_collection_insert (filesystemMongo, MONGOC_INSERT_NONE, filesystem, NULL, &error);    // ver si no hay que crear una collection especial para la lista nodo.

	bson_destroy(filesystem);

	return res;
}

bool borrar_filesystem_mongo(t_filesystem* el_fs) {
    bool res;

    bson_t *filesystem = bson_new();
    bson_error_t error;
    armar_filesystem_mongo(el_fs,&filesystem);

    res = mongoc_collection_remove (filesystemMongo,MONGOC_DELETE_SINGLE_REMOVE,filesystem,NULL,&error);

	bson_destroy(filesystem);

	return res;
}

int grabar_filesystem_mongo(t_filesystem* el_fs_viejo, t_filesystem* el_fs_nuevo) {
    int res;

    bson_t *filesystemViejo = bson_new();
    bson_t *filesystemNuevo = bson_new();

    armar_filesystem_mongo(el_fs_viejo,&filesystemViejo);
    armar_filesystem_mongo(el_fs_nuevo,&filesystemNuevo);

    res = mongoc_collection_update(filesystemMongo, MONGOC_UPDATE_UPSERT, filesystemViejo, filesystemNuevo, NULL, NULL);
    // ver si no hay que crear una collection especial para la lista nodo.

	bson_destroy(filesystemViejo);
	bson_destroy(filesystemNuevo);

	return res;
}


int leer_filesystem_mongo(){
//t_archivo_json *leerArchivoMongo(bson_t *query)

	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;
	bson_iter_t iter;

	t_filesystem *archivoFilesystem = malloc(sizeof (t_filesystem));

	mongo_db_filesystem_open();

	query  = bson_new ();

	cursor = mongoc_collection_find (filesystemMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){

		    	if(bson_iter_find(&iter,"index"))
		    		archivoFilesystem->index = (int32_t)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"directorio")){
		        	archivoFilesystem->directorio=strdup(bson_iter_utf8(&iter,NULL));
		           	}

		        if(bson_iter_find(&iter,"padre"))
		        	archivoFilesystem->padre = (int32_t)bson_iter_int32(&iter);

		        	 }

		        }

	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_filesystem,archivoFilesystem);
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	free(archivoFilesystem);

	return list_size(lista_filesystem);
}

void mongo_db_archivosM_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	archivosMongo = mongoc_client_get_collection (client, "test", "MDFS_ARCHIVOSMONGO");
}

void mongo_db_archivosM_close(){
	mongoc_collection_destroy (archivosMongo);
	mongoc_client_destroy (client);
}

void armar_archivo_mongo(t_archivo*el_archivo, bson_t **archivo, bson_t **listaBloques, bson_t **copiasArray){
	bson_append_utf8((void*)archivo, "nombreArchivo", 13, el_archivo->nombreArchivo, strlen(el_archivo->nombreArchivo));
		bson_append_int32((void*)archivo, "padre", 5, el_archivo->padre);
		bson_append_int32((void*)archivo, "tamanio", 7, el_archivo->tamanio);
		bson_append_int32((void*)archivo, "estado", 6, el_archivo->estado);

		bson_append_array_begin((void*)archivo, "listaBloques", 12, (void*)listaBloques);
		int j;
		for (j=0; j < list_size(el_archivo->listaBloques); j++) {

			t_bloque* el_bloque = list_get(el_archivo->listaBloques, j); //Chequear si tiene que arrancar en 0 para el primer elemento


			bson_append_int32((void*)listaBloques, "bloque", 6, el_bloque->bloque);
			bson_append_array_begin((void*)listaBloques, "copias", 6, (void*)copiasArray);
			int i ;
			for (i= 0; i < 3; i++) {

				t_array_copias* la_copia = malloc(sizeof(t_array_copias));
				la_copia->nombreNodo = el_bloque->array[i].nombreNodo; //Chequear si tiene que arrancar en 0 para el primer elemento
				la_copia->nro_bloque = el_bloque->array[i].nro_bloque;

				bson_append_utf8((void*)copiasArray, "nombreNodo", 10, la_copia->nombreNodo, strlen(la_copia->nombreNodo));
				bson_append_utf8((void*)copiasArray, "nroBloque", 9, la_copia->nro_bloque, strlen(la_copia->nro_bloque));


				}
			bson_append_array_end((void*)listaBloques, (void*)copiasArray);
		}
		bson_append_array_end((void*)archivo, (void*)listaBloques);
}

bool crear_archivo_mongo(t_archivo* el_archivo) {
    bool res;

    bson_t *archivo = bson_new();
    bson_t *listaBloques = bson_new();
    bson_t *copiasArray = bson_new();
    bson_error_t error;


    armar_archivo_mongo(el_archivo, &archivo, &listaBloques, &copiasArray);


    res= mongoc_collection_insert (archivosMongo, MONGOC_INSERT_NONE, archivo, NULL, &error);    // ver si no hay que crear una collection especial para la lista nodo.
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(listaBloques);
	bson_destroy(archivo);
	bson_destroy(copiasArray);

	return res;
}

bool borrar_archivo_mongo(t_archivo* el_archivo) {
    bool res;

    bson_t *archivo = bson_new();
    bson_t *listaBloques = bson_new();
    bson_t *copiasArray = bson_new();
    bson_error_t error;


    armar_archivo_mongo(el_archivo, &archivo, &listaBloques, &copiasArray);

    res = mongoc_collection_remove (archivosMongo,MONGOC_DELETE_SINGLE_REMOVE,archivo,NULL,&error);

    bson_destroy(listaBloques);
	bson_destroy(archivo);
	bson_destroy(copiasArray);

	return res;
}

int grabar_archivo_mongo(t_archivo* el_archivo_viejo, t_archivo* el_archivo_nuevo) {
    int res;

    bson_t *archivoViejo = bson_new();
        bson_t *listaBloquesViejo = bson_new();
        bson_t *copiasArrayViejo = bson_new();
        bson_t *archivoNuevo = bson_new();
            bson_t *listaBloquesNuevo = bson_new();
            bson_t *copiasArrayNuevo = bson_new();

            armar_archivo_mongo(el_archivo_viejo, &archivoViejo, &listaBloquesViejo, &copiasArrayViejo);
            armar_archivo_mongo(el_archivo_nuevo, &archivoNuevo, &listaBloquesNuevo, &copiasArrayNuevo);


    res = mongoc_collection_update(archivosMongo, MONGOC_UPDATE_UPSERT, archivoViejo, archivoNuevo, NULL, NULL);
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(listaBloquesViejo);
    	bson_destroy(archivoViejo);
    	bson_destroy(copiasArrayViejo);
    	bson_destroy(listaBloquesNuevo);
    		bson_destroy(archivoNuevo);
    		bson_destroy(copiasArrayNuevo);

	return res;
}

int leer_archivo_mongo(){
//t_archivo_json *leerArchivoMongo(bson_t *query)

	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;
	bson_iter_t iter, child,child2;
	uint32_t array_len;
	const uint8_t    *array;
	t_archivo *archivoMongo = malloc(sizeof (t_archivo));
	t_bloque* el_bloque=malloc(sizeof (t_bloque));
	//t_array_copias* el_array =malloc(sizeof (t_array_copias));

	mongo_db_archivosM_open();

	query  = bson_new ();

	cursor = mongoc_collection_find (archivosMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){
		        if(bson_iter_find(&iter,"nombreArchivo")){
		        	archivoMongo->nombreArchivo=strdup(bson_iter_utf8(&iter,NULL));
		           	}
		        if(bson_iter_find(&iter,"padre"))
		            archivoMongo->padre = (int32_t)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"tamanio"))
		        	 archivoMongo->tamanio = (int32_t)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"estado"))
		        	 archivoMongo->estado = (int32_t)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"listaBloques")){
		        	 archivoMongo->listaBloques=list_create();
		        	 bson_iter_array (&iter, &array_len,&array);
		        	 bson_iter_recurse(&iter, &child);
		        	 while(bson_iter_next(&child)){
		        		 if(bson_iter_find(&child,"bloque"))
		        		 el_bloque->bloque = (int32_t)bson_iter_int32(&iter);
		        		 int i=0;
		        		 if(bson_iter_find(&child,"copias")){


		        			 bson_iter_array (&child, &array_len,&array);
		        			 bson_iter_recurse(&child, &child2);
		        			 while(bson_iter_next(&child2)){
		        			 if(bson_iter_find(&child2,"nombreNodo")){
		        				 el_bloque->array[i].nombreNodo =strdup(bson_iter_utf8(&child2,NULL));
		        			 }
		        			 if(bson_iter_find(&child2,"nroBloque")){
		        				 el_bloque->array[i].nro_bloque = strdup(bson_iter_utf8(&child2,NULL));
		        				 }


		        			 }
		        			 i++;

		        		 }
		        	 }

		        }


		    }

		}
	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_archivos,archivoMongo);
	    list_destroy(archivoMongo->listaBloques); //Limpio la lista bloques para el proximo archivo a cargar.
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	free(archivoMongo);

	return list_size(lista_archivos);
}
