/*
 * filesystemdev.c
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#include "filesystem.h"
#include "mongodev.h"


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

int CharAToInt(char* x) {
	int numero = 0;
	char * aux = string_new();
	string_append_with_format(&aux, "%c", x);

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
		sem_wait(&semLNodos);
		list_add(lista_nodos,el_nodo);
		sem_post(&semLNodos);
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
	t_array_copias * la_copia= malloc(sizeof(t_array_copias));
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
				sem_wait(&semArchivos);
				string_append(&*bufferE,obtenerSubBuffer(el_archivo->nombreArchivo));//aca va la ruta, ahora no lo hace
				cont = cuentaDigitos(list_size(el_archivo->listaBloques));
				string_append(&*bufferE,string_itoa(cont));
				string_append(&*bufferE,string_itoa(list_size(el_archivo->listaBloques)));
				for(i=0;i<list_size(el_archivo->listaBloques);i++){
					el_bloque = list_get(el_archivo->listaBloques,i);
					for(k=0;k<list_size(el_bloque->listaCopias);k++){
						la_copia = list_get(el_bloque->listaCopias,k);
						if(la_copia != NULL){
							string_append(&*bufferE,obtenerSubBuffer(la_copia->nombreNodo));
							bool _true(void *elem){
								return (!strcmp((char*)elem,la_copia->nombreNodo));
							}
							if(!list_any_satisfy(nodos,_true)){
								list_add(nodos,la_copia->nombreNodo);
							}
							//string_append(&*bufferE,"Bloque"); //Le agrego la palabra que le habia sacado de antes
							string_append(&*bufferE,obtenerSubBuffer(la_copia->nro_bloque));
						}
					}
				}
				sem_post(&semArchivos);
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
	//log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
		//(char*) bufferE);

	bufferR = RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);

	if(strcmp(bufferR,"Ok")==0){
		//Segunda Rafaga para Marta
		if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1)
			log_info(logger,"No puedo enviar información al cliente. Socket: %d", socket);
		//log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			//(char*) buffer);
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

long unsigned ObtenerLu (char *buffer){
	long unsigned x,digito,aux=0;
	for(x=0;x<strlen(buffer);x++){
		digito=ChartToInt(buffer[x]);
		aux=aux*10+digito;
	}
	return aux;
}


int subirArchivoDelNodo(char* buffer,int *cantRafaga, int socket){

	int digitosCantTamanio=0,tamanio;
	long unsigned tamanioBuffer;
	int posActual=0;
	char * bufferR = string_new();

	char * rutaCompleta = malloc(50);
	memset(rutaCompleta,0,50);

	//BUFFER RECIBIDO = 3220 (EJEMPLO)
	//BUFFER RECIBIDO = 3119127.0.0.12460002101000000000
	//3: es nodo 119127.0.0.1: la ip 246000: el puerto 2101000000000: el tamaño del espacio de datos del nodo
	//Ese 3 que tenemos abajo es la posicion para empezar a leer el buffer 411

	digitosCantTamanio=PosicionDeBufferAInt(buffer,2);
	//printf("Cantidad de digitos de Tamanio de Ip:%d\n",digitosCantTamanio);
	tamanio=ObtenerTamanio(buffer,3,digitosCantTamanio);
	//printf("Tamaño de Recibido:%d\n",tamanio);
	if(tamanio>=10){
		posActual=digitosCantTamanio;
	} else {
		posActual=1+digitosCantTamanio;
	}
	tamanioBuffer=ObtenerLu(DigitosNombreArchivo(buffer,&posActual));
	printf("Tamanio de Buffer :%lu\n",tamanioBuffer);

	bufferR = DigitosNombreArchivo(buffer,&posActual);

	memcpy(rutaCompleta,bufferR,strlen(bufferR));

	printf("Ruta Completa:%s\n",rutaCompleta);

	*cantRafaga = 2;

	char *bloque=malloc(tamanioBuffer+1);

	memset(bloque,0,tamanioBuffer+1);

	char *recibido=string_new();

	EnviarDatos(socket, "Ok", strlen("Ok"));

	ssize_t numBytesRecv = 0;
	char * aux = malloc(tamanioBuffer+1);
	memset(aux,0,tamanioBuffer+1);

	do{
		numBytesRecv = numBytesRecv + recv(socket, aux, tamanioBuffer, 0);
		if ( numBytesRecv < 0)
			printf("ERROR\n");
		string_append(&recibido,aux);
		strcat(bloque,recibido);
		free(recibido);
		recibido=string_new();
		printf("------ %d -----\n",strlen(bloque));
		memset(aux, 0, tamanioBuffer+10);
	}while (numBytesRecv <tamanioBuffer);


	EnviarDatos(socket, "Ok", strlen("Ok"));
	//printf("Enviado el OK\n");

	int cont=0;

	char **array = string_split(rutaCompleta,"/");

	while(array[cont]!=NULL){
		  cont++;
	}
	char * nombreResultado = strdup(array[cont-1]);
	array[cont-1] = NULL;
	int j=0;
	int k,padre,correcto=1;

	while(array[j]!=NULL&&correcto){
		if(j==0){
			k=validarDirectorio(array[j],0);
		} else {
			k=validarDirectorio(array[j],padre);
		}
		if(k!=0){
			padre=k;
			j++;
		} else {
			correcto=0;
		}
	}
	if(correcto){
		archivo = archivo_create(nombreResultado,tamanioBuffer,padre,1);
		list_add(lista_archivos,archivo);
		crear_archivo_mongo(archivo);

		FILE * arch = fopen("postadirecto.txt","w");
		fwrite(bloque,1,tamanioBuffer,arch);
		fclose(arch);

		enviarBloque(bloque);

		printf("Lo subio!\n");
	} else {
		printf("No se pudo subir el archivo de resultado final porque no existe el directorio.\n");
		return 0;
	}

	return 1;
}


void implementoNodo(char * buffer,int * cantRafaga,char ** mensaje, int socket){

	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
		//printf("RAFAGA:%d\n",*cantRafaga);
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
				if(tipo_mensaje == SUBIRARCHIVO){
					if(subirArchivoDelNodo(buffer,cantRafaga,socket)){
						*mensaje = "Ok";
					} else {
						*mensaje = "No";
					}
					*cantRafaga=1;
				} else {
					*mensaje = "Ok";
					*cantRafaga = 2;
				}
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

	//log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket,
		//	(char*) bufferAux, strlen(bufferAux));
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
		//log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,(char*) buffer);
	} else {
		//log_info(logger, "ENVIO DATOS. socket: %d. Tamanio:%d ",socket,strlen(buffer));
	}

	return bytecount;
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	//log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
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
	t_array_copias * la_copia;
	int i=0;
	int j=0;
	int k=0;

	while(i<list_size(lista_archivos)){
		el_archivo = list_get(lista_archivos, i);
		printf("El archivo:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);

		while(j<list_size(el_archivo->listaBloques)){
			el_bloque = list_get(el_archivo->listaBloques, j);
			printf("%d :: \n",el_bloque->bloque);
			while(k<list_size(el_bloque->listaCopias)){
			la_copia = list_get(el_bloque->listaCopias,k);
			printf("Copia %d: ", (k+1));
			printf("%s--",la_copia->nombreNodo);
			printf("%s  \n",la_copia->nro_bloque);
			k++;
			}
			k=0;
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
	crear_filesystem_mongo(el_fs);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(2,"home",0);
	crear_filesystem_mongo(el_fs);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(3,"utnso",2);
	crear_filesystem_mongo(el_fs);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(4,"tp",3);
	crear_filesystem_mongo(el_fs);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(5,"tp-2015",4);
	crear_filesystem_mongo(el_fs);
	list_add(lista_filesystem,el_fs);
	el_fs = filesystem_create(6,"var",0);
	crear_filesystem_mongo(el_fs);
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
				printf("Emisor:%d\n",emisor);
				abort();
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
//	log_trace(logger,
	//		"SOCKET LISTO PARA RECBIR CONEXIONES. Numero de socket: %d, puerto: %d",
		//	socket_host, fs_Puerto);

	while (g_Ejecutando) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,(struct sockaddr *) &client_addr, &size_addr)) != -1) {
			//Traza("Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);
			//log_trace(logger,
			//		"NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d",
				//	inet_ntoa(client_addr.sin_addr), client_addr.sin_port,
					//socket_client);
			// Aca hay que crear un nuevo hilo, que será el encargado de atender al cliente
			pthread_t hNuevoCliente;
			sem_wait(&semHilos);
			pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente,
					(void *) socket_client);
			sem_post(&semHilos);
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
	//log_info(logger, "Intentando conectar a nodo\n");
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

	//sem_wait(&semLNodos);
	int cantNodos = list_size(lista_nodos);
	//sem_post(&semLNodos);

	while(i<cantNodos){
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
	sem_wait(&semLNodos);
	el_nodo = list_find(lista_nodos, _true);
	sem_post(&semLNodos);
	return el_nodo;
}

void cargarListaBloquesDisponibles(t_nodo* nodo){
	long unsigned i,cantBloques;
	cantBloques = ObtenerLu(nodo->tamanio)/TAMANIO_BLOQUE;
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
		//sem_wait(&semLNodos);
		el_nodo = buscarNodo(ipNodo,puertoNodo);
		//sem_post(&semLNodos);
		if(el_nodo!=NULL){
			if(el_nodo->estado==-1){
				printf("\nVolvio a habilitarse el %s !\n",el_nodo->nombre);
				borrar_nodo_mongo(el_nodo);
				el_nodo->estado = 1;
				crear_nodo_mongo(el_nodo);
			} else {
				printf("\n%s Habilitado!\n",el_nodo->nombre);
				el_nodo->estado = 1;
				cargarListaBloquesDisponibles(el_nodo);
				crear_nodo_mongo(el_nodo);
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
	sem_wait(&semLFs);
	lista = list_filter(lista_filesystem,_true);
	sem_post(&semLFs);
	if(lista!=NULL){
		bool _true2(void *elem){
			return !strcmp(((t_filesystem*) elem)->directorio,directorio);
		}
		fs = list_find(lista,_true2);
		if(fs==NULL){
			printf("Directorio:%s Invalido\n",directorio);
			return 0;
		} else {
			//printf("Directorio:%s Valido\n",directorio);
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

	if((((*tamanio)/1024/1024)*3)<tamanioDisponible()){
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
		sem_wait(&semArchivos);
		archivo = archivo_create(nombreArchivo,*tamanio,padre,1);
		list_add(lista_archivos,archivo);
		sem_post(&semArchivos);
		return 1;
	} else {
		printf("El espacio disponible del fs es insuficiente para subir este archivo\n");
		return 0;
	}
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
    t_array_nodo *el_array_nodo;
    for(k=0;k<list_size(lista_nodos);k++){
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

    t_array_nodo *el_array_nodo;

    char *bloqueChar = string_new();
    string_append(&bloqueChar,"bloque");
    string_append(&bloqueChar,string_itoa(bloque));

    for(k=0;k<list_size(lista_nodos);k++){
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

    int i=0,k=0;
    t_nodo *el_nodo;
    t_array_nodo *el_array_nodo;


    arrayNodos=(t_list**)malloc(list_size(lista_nodos)*sizeof(t_array_nodo));

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
    //imprimirArrayNodos();
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
	//imprimirArrayNodos();
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
    		  borrar_nodo_mongo(el_nodo);
    		  //t_nodo* nodoViejo = list_get(lista_nodos,eleccion);
    		  el_nodo->estado=-1;
    		  //grabar_nodo_mongo(nodoViejo,el_nodo);
    		  crear_nodo_mongo(el_nodo);
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
	EnviarDatos(socket,buffer1,strlen(buffer1));
	bufferR=RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
	//EnviarDatos(socket,buffer2,strlen(buffer2));
	if(strcmp(bufferR,"1")==0){
		long unsigned len=0;
		len=strlen(buffer2);
		//printf("LEN : %lu\n",len);
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
		t_array_copias * la_copia= malloc(sizeof(t_array_copias));
		ordenarArrayNodos();
		nroNodo = buscarNodoEnArray((*bloque)->bloque);
		if(nroNodo!=-1){
			nodo = list_get(arrayNodos[nroNodo],0);
			el_nodo=buscarNodoPorNombre(nodo->nombre);
			//printf("Nodo:%s BloqueArchivo:%d\n",nodo->nombre,(*bloque)->bloque);
			borrar_nodo_mongo(el_nodo);
			bloqueDisponible = buscarBloqueDisponible(el_nodo);
			crear_nodo_mongo(el_nodo);
			envio_nodo = envio_nodo_create(buffer,el_nodo->ip,el_nodo->puerto,bloqueDisponible);
			agregarBloqueEnArrayNodos(nroNodo,bloqueDisponible,(*bloque)->bloque);
			//printf("EL GRAN BLOQUE:%d",(*bloque)->bloque);
			pthread_t henviarNodos;
			sem_wait(&semHilos);
			int iThreadHilo = pthread_create(&henviarNodos, NULL,
					(void*) enviarBufferANodo, (void*) envio_nodo );
			sem_post(&semHilos);
			if (iThreadHilo) {
				fprintf(stderr,
					"Error al crear hilo - pthread_create() return code: %d\n",
					iThreadHilo);
				exit(EXIT_FAILURE);
			}


			pthread_join(henviarNodos, NULL );

			char *nombre = string_new();
			string_append(&nombre,"Bloque"); //Lo comento para poder liberar los bloques, sino no tengo como saber el numero de bloque
			string_append(&nombre,string_itoa(bloqueDisponible));

			la_copia->nombreNodo=nodo->nombre;
			la_copia->nro_bloque=nombre;

			list_add((*bloque)->listaCopias,la_copia);
			//(*bloque)->array[j].nombreNodo=nodo->nombre;
			//(*bloque)->array[j].nro_bloque=nombre;



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
	int j=0;
	llenarArrayDeNodos();
	nroBloque=0;
	while(!feof(fArchivo)){
		fread(buffer,1,TAMANIO_BLOQUE,fArchivo);
		for(j=0;j<TAMANIO_BLOQUE;j++){
			if(buffer[TAMANIO_BLOQUE-j]=='\n'){
				bufferAux = malloc(TAMANIO_BLOQUE-j+2);
				memset(bufferAux,0,TAMANIO_BLOQUE-j+2);
				memcpy(bufferAux,buffer,TAMANIO_BLOQUE-j+1);
				if(enviarBloque(bufferAux)){
					tamanio = ftell(fArchivo);
					if(tamanio!=tamanioA){
						fseek(fArchivo,-j+1,SEEK_CUR);
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
	//printf(COLOR_VERDE"TAMANIO TOTAL DEL ARCHIVO:%lu\n",tamanioTotal);
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
		//	printf("TAMANIO DE BUFFER:%d\n",tamanio);
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
					//printf("------ %d -----\n",strlen(bloque));
					memset(aux, 0, tamanio+1);

				}while (numBytesRecv <tamanio);
			//	printf("TAMANIO:%d NUMBYTESRECV:%d\n",tamanio,numBytesRecv);
				*buffer=malloc(tamanio+1);
				memset(*buffer,0,tamanio+1);
				memcpy(*buffer,bloque,tamanio);
				free(bloque);
				//printf("BLOQUE:%d TAMANIO:%lu Recibido Ok\n",nroBloque,(long unsigned)strlen(*buffer));
				return 1;
			}
		}
	}
	return 0;
}

int pedirBloque(int nroBloque, t_array_copias* copia,char** buffer){
	t_nodo * nodo;
	char** array = string_split(copia->nro_bloque,"e");
	nodo = buscarNodoPorNombre(copia->nombreNodo);
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
				t_array_copias * copia = malloc(sizeof(t_array_copias));
				copia = list_get(bloque->listaCopias,l);
				while(!pedirBloque(bloque->bloque,copia,&buffer)){
					if(l>list_size(bloque->listaCopias)) return 0;
					l++;
					copia = list_get(bloque->listaCopias,l);
				}
					if(l>list_size(bloque->listaCopias)) return 0;
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
			crear_archivo_mongo(archivo);
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
	while(list_size(lista_filesystem)!=0){
		t_filesystem* filesystemEliminar= list_get(lista_filesystem,i);
		borrar_filesystem_mongo(filesystemEliminar);
		list_remove_and_destroy_element(lista_filesystem,i,(void*)filesystem_destroy);
	}
	//free(lista_filesystem);
}

void eliminarArchivos(){
	int i = 0;
	while(list_size(lista_archivos)!=0){
		t_archivo* archEliminar = list_get(lista_archivos,i);
		borrar_archivo_mongo(archEliminar);
		list_remove_and_destroy_element(lista_archivos,i,(void*)archivo_destroy);
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
		crear_filesystem_mongo(filesystem);
		sem_wait(&semLFs);
		list_add(lista_filesystem,filesystem);
		sem_post(&semLFs);
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
		t_filesystem* filesystemDestroy = list_find(lista_filesystem,_true2);
		borrar_filesystem_mongo(filesystemDestroy);
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
					borrar_filesystem_mongo(fs);
					fs->padre = indexDestino;
					crear_filesystem_mongo(fs);
					printf("Se ha realizado el movimiento con exito!");
					return 1;
				} else {
					printf("Directorio ingresado invalido.\n");
					return 0;
				}
			} else {
				borrar_filesystem_mongo(fs);
				fs->padre = 0;
				crear_filesystem_mongo(fs);
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
				borrar_filesystem_mongo(fs);
				fs->directorio = strdup(destino);
				crear_filesystem_mongo(fs);
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

int renombrarArchivo(){

	char pathConArchivo[100];
	char* nombreArchivo = malloc(sizeof(char)*30);
	char archivoSalida[50];
	int padre;
	mostrarFilesystem();

	printf("Ingrese la ruta completa con el nombre del archivo para recuperar\n ");
	printf("Ejemplo: /home/utnso/temperatura.txt\n");
	scanf("%s",pathConArchivo);
	fflush(stdin);

	padre = validarDirectorios(pathConArchivo,&nombreArchivo);
	if(padre!=-1){
		printf("Ingrese el nuevo nombre del archivo:\n");
		scanf("%s",archivoSalida);
		fflush(stdin);
		t_archivo* archivo = buscarArchivoPorNombre(nombreArchivo,padre);
			if(archivo!=NULL){
				borrar_archivo_mongo(archivo);
				archivo->nombreArchivo= strdup(archivoSalida);
				crear_archivo_mongo(archivo);
			}

		return 1;
	} else {
		return 0;
	}
}

void liberarBloquesArchivo(t_archivo * el_archivo){

	t_bloque * el_bloque;
	t_array_copias * la_copia;
	char **array;


			int j=0;

			printf("El archivo:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);

		while(j<list_size(el_archivo->listaBloques)){
			el_bloque = list_get(el_archivo->listaBloques, j);

			while(list_size(el_bloque->listaCopias)!=0){
			la_copia = list_remove(el_bloque->listaCopias,0);
			t_nodo* el_nodo = buscarNodoPorNombre(la_copia->nombreNodo);
			array = string_split(la_copia->nro_bloque,"e");
			borrar_nodo_mongo(el_nodo);
			list_add(el_nodo->bloquesDisponibles,bloque_disponible_create(atoi(array[1])));
			crear_nodo_mongo(el_nodo);
            free(la_copia);
            }
			j++;
		}
		archivo_destroy(el_archivo);
}


int eliminarArchivo(){

	char pathConArchivo[100];
	char* nombreArchivo = malloc(sizeof(char)*30);
	int padre;
	mostrarFilesystem();

	printf("Ingrese la ruta completa con el nombre del archivo para eliminar\n ");
	printf("Ejemplo: /home/utnso/temperatura.txt\n");
	scanf("%s",pathConArchivo);
	fflush(stdin);

	padre = validarDirectorios(pathConArchivo,&nombreArchivo);
	if(padre!=-1){
		t_archivo* archivo = buscarArchivoPorNombre(nombreArchivo,padre);
			if(archivo!=NULL){
				borrar_archivo_mongo(archivo);
				liberarBloquesArchivo(archivo);
			}

		return 1;
	} else {
		return 0;
	}
}

int moverArchivo(){

	char pathConArchivo[100];
	char* nombreArchivo = malloc(sizeof(char)*30);
	int padre1;
	mostrarFilesystem();
	char directorio[50];
	int correcto = 0;
	char * path = string_new();



	printf("Ingrese la ruta completa con el nombre del archivo para mover\n ");
	printf("Ejemplo: /home/utnso/temperatura.txt\n");
	scanf("%s",pathConArchivo);
	fflush(stdin);

	padre1 = validarDirectorios(pathConArchivo,&nombreArchivo);
	if(padre1!=-1){

		int j=0,padre=0,k;
				while(!correcto){
					mostrarFilesystem();
					printf("Ingrese de un directorio a donde mover el archivo: ejemplo: home\n");
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

		t_archivo* archivo = buscarArchivoPorNombre(nombreArchivo,padre1);
			if(archivo!=NULL){
				borrar_archivo_mongo(archivo);
				archivo->padre= padre;
				crear_archivo_mongo(archivo);
			}

		return 1;
	} else {
		return 0;
	}
}

int manejoDeArchivos(){
	int seleccion;
	mostrarFilesystem();
	printf("Ingrese un comando:\n");
	printf("1: Eliminar 2: Renombrar 3: Mover\n");
	scanf("%d", &seleccion);
	switch(seleccion){
		case 1: printf("Se eligio Eliminar\n");
				return eliminarArchivo();
				break;
		case 2: printf("Se eligio Renombrar\n");
				return renombrarArchivo();
				break;
		case 3: printf("Se eligio Mover\n");
				return moverArchivo();
				break;
		default: printf("Se ha ingresado un comando incorrecto.\n");
				break;
	}
	return 0;
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

int armarArchivoMD5PorHilo(){

	int valor;

	int iThreadHilo = pthread_create(&hNodos, NULL,
					(void*) armarArchivoMD5, NULL );
	if (iThreadHilo) {
		fprintf(stderr,
		"Error al crear hilo - pthread_create() return code: %d\n",
		iThreadHilo);
		exit(EXIT_FAILURE);
	}

	pthread_join(hNodos, (void*)&valor );

	return valor;
}

int armarArchivoMD5(){
	int padre;
	FILE * fArchivo;
	char* nombre;
	//char * buffer = malloc(100);

	if(cargarArchivoMD5(&padre,&fArchivo,&nombre)){
		if(recuperarArchivoMD5(fArchivo,padre,nombre)){
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int cargarArchivoMD5(int *padre,FILE ** fArchivo,char**nombreArchivo){

	char pathConArchivo[100];
	*nombreArchivo = (char*)malloc(sizeof(char)*30);
	char* archivoSalida;

	mostrarFilesystem();

	printf("Ingrese la ruta completa con el nombre del archivo para ver el MD5\n ");
	printf("Ejemplo: /home/utnso/temperatura.txt\n");
	scanf("%s",pathConArchivo);
	fflush(stdin);

	*padre = validarDirectorios(pathConArchivo,nombreArchivo);
	if(*padre!=-1){
		archivoSalida="MD5.txt";
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


int recuperarArchivoMD5(FILE *fArchivo,int padre, char*nombre){
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
				t_array_copias * copia = malloc(sizeof(t_array_copias));
				copia = list_get(bloque->listaCopias,l);
				while(!pedirBloque(bloque->bloque,copia,&buffer)){
				if(l>list_size(bloque->listaCopias)) return 0;
				l++;
				copia = list_get(bloque->listaCopias,l);
				}
				if(l>list_size(bloque->listaCopias)) return 0;
				fwrite(buffer,sizeof(char),strlen(buffer),fArchivo);
			}
			fclose(fArchivo);
			free(buffer);
			system("md5sum MD5.txt");
			system("rm MD5.txt");
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
		t_nodo* nodoEliminar = list_get(lista_nodos, i);
		borrar_nodo_mongo(nodoEliminar);
		list_remove_and_destroy_element(lista_nodos,i++,(void*)nodo_destroy);
		//list_remove_and_destroy_element(lista_filesystem,i++,(void*)filesystem_destroy);
	}
	//free(lista_filesystem);
}

int formatearFs(){
	indexGlobal=1;
	eliminarFilesystem();
	mongoc_collection_drop(filesystemMongo,NULL);
		printf("Se elimino la estructura del filesystem\n");
		eliminarArchivos();
		mongoc_collection_drop(archivosMongo,NULL);
		printf("Se eliminaron todos los archivos\n");
		//sem_wait(&semLNodos);
		eliminarListaNodos();
		//sem_post(&semLNodos);
		mongoc_collection_drop(nodosMongo,NULL);

		printf("Se eliminaron todos los Nodos\n");
	return 1;
}

long unsigned tamanioDisponible(){
	int i=0;
	long unsigned tamanio=0;
	t_nodo* nodo;
	sem_wait(&semLNodos);
	while(i<list_size(lista_nodos)){
		nodo = list_get(lista_nodos,i);
		if(nodo->estado==1){
			tamanio = tamanio + (list_size(nodo->bloquesDisponibles)*20);
		}
		i++;
	}
	sem_post(&semLNodos);
	return tamanio;
}

long unsigned tamanioTotal(){
	int i=0;
	t_nodo* nodo;
	long unsigned tamanio=0,bloques;
	sem_wait(&semLNodos);
	while(i<list_size(lista_nodos)){
		nodo = list_get(lista_nodos,i);
		if(nodo->estado==1){
			bloques = ObtenerLu(nodo->tamanio)/TAMANIO_BLOQUE;
			tamanio = tamanio + bloques*20;
		}
		i++;
	}
	sem_post(&semLNodos);
	return tamanio;
}

long unsigned tamanioOcupado(){
	return tamanioTotal()-tamanioDisponible();
}

int operaciones_consola() {
	//system("clear");
	printf("------------Tamaño del Fs:%luMb Tamaño Ocupado:%luMb Tamaño Disponible:%luMb-------------\n",tamanioTotal(),tamanioOcupado(),tamanioDisponible());
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
		if(manejoDeArchivos()){
			log_info(logger, "Se realizo Eliminar/Renombrar/Mover Archivos\n");
		} else {
			log_info(logger, "No se realizo Eliminar/Renombrar/Mover Archivos\n");
		}
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
		if(armarArchivoMD5PorHilo()==1){
			log_info(logger, "Se realizo Solicitar el MD5 de un archivo en MDFS\n");
			} else {
				log_info(logger, "No se pudo realizar el MD5 de un archivo en MDFS\n");
						}
			break;

	case 7:
		if(manejoDeBloques()){
		log_info(logger, "Se realizo Ver/Borrar/Copiar los bloques que componen un archivo\n");
		} else{
			log_info(logger, "No se pudo realizar Ver/Borrar/Copiar los bloques que componen un archivo\n");
		}
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
	new->listaCopias  = list_create();
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
	free(self->listaCopias);
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
	new->puerto = strdup(puertoNodo);
	new->tamanio = strdup(tamanio);
	new->estado = activo;
	new->bloquesDisponibles = list_create();
	return new;
}

void nodo_destroy(t_nodo* self) {
	free(self);
}

t_array_copias* array_copias_create(char* nombre, char* bloque){
	t_array_copias* new = malloc(sizeof(t_array_copias));
	new->nombreNodo = strdup(nombre);
	new->nro_bloque = strdup(bloque);
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
    //printf("BUFFER POSTA:%d BUFFER CON PUTO STRDUP:%d\n",strlen(buffer),strlen(new->buffer));
    new->ip=strdup(ip);
    new->puerto=strdup(puerto);
    new->bloque = bloque;
    return new;
}

t_bloque_disponible *bloque_disponible_create(int bloque) {
    t_bloque_disponible *new = malloc(sizeof(t_bloque_disponible));
    new->bloque = bloque;
    new->vacio = 1;
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

void mongo_db_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	archivosMongo = mongoc_client_get_collection (client, "test", "MDFS_ARCHIVOSMONGO");
	nodosMongo = mongoc_client_get_collection (client, "test", "MDFS_NODOS");
	filesystemMongo = mongoc_client_get_collection (client, "test", "MDFS_FILESYSTEM");

}

void mongo_db_close(){
	mongoc_collection_destroy (nodosMongo);
	mongoc_collection_destroy (filesystemMongo);
	mongoc_collection_destroy (archivosMongo);
	mongoc_client_destroy (client);
}


bson_t* armar_nodo_mongo(t_nodo* el_nodo,bson_t*bloquesDisp){
	bson_t* nodo = bson_new();
	bson_append_utf8(nodo, "nombre", 6, el_nodo->nombre, strlen(el_nodo->nombre));
		bson_append_utf8(nodo, "ip", 2, el_nodo->ip, strlen(el_nodo->ip));
		bson_append_utf8(nodo, "puerto", 6, el_nodo->puerto, strlen(el_nodo->puerto));
		bson_append_utf8(nodo, "tamanio", 7, el_nodo->tamanio, strlen(el_nodo->tamanio));
		bson_append_int32(nodo, "estado", 6, el_nodo->estado);
		bson_append_array_begin(nodo, "bloquesDisponibles", 18, bloquesDisp);
		int i;
		for (i = 0; i < list_size(el_nodo->bloquesDisponibles); i++) {

			t_bloque_disponible* el_bloque = list_get(el_nodo->bloquesDisponibles, i); //Chequear si tiene que arrancar en 0 para el primer elemento


			bson_append_int32(bloquesDisp, "bloque", 6, el_bloque->bloque);

		}
		bson_append_array_end(nodo, bloquesDisp);

		return nodo;
}

bson_t* armar_nodo_mongo2(t_nodo* el_nodo,bson_t*bloquesDisp){
	bson_t* nodo = bson_new();
	bson_append_utf8(nodo, "nombre", 6, el_nodo->nombre, strlen(el_nodo->nombre));
		bson_append_utf8(nodo, "ip", 2, el_nodo->ip, strlen(el_nodo->ip));
		bson_append_utf8(nodo, "puerto", 6, el_nodo->puerto, strlen(el_nodo->puerto));
		bson_append_utf8(nodo, "tamanio", 7, el_nodo->tamanio, strlen(el_nodo->tamanio));
		bson_append_int32(nodo, "estado", 6, el_nodo->estado);

		return nodo;
}


bool crear_nodo_mongo(t_nodo* el_nodo) {
    bool res;
    bson_t *bloquesDisp = bson_new();
    bson_error_t error;

    bson_t *nodo = armar_nodo_mongo(el_nodo,bloquesDisp);


	res= mongoc_collection_insert (nodosMongo, MONGOC_INSERT_NONE, nodo, NULL, &error);
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(bloquesDisp);

	bson_destroy(nodo);

	return res;
}

bool borrar_nodo_mongo(t_nodo* el_nodo){
	bool res;
	    bson_t *bloquesDisp = bson_new();
	    bson_error_t error;
	    bson_t *nodo = armar_nodo_mongo2(el_nodo, bloquesDisp);


	    res = mongoc_collection_remove(nodosMongo,MONGOC_DELETE_SINGLE_REMOVE,nodo,NULL,&error);

	    bson_destroy(bloquesDisp);
	    bson_destroy(nodo);

	    return res;

}

int grabar_nodo_mongo(t_nodo* el_nodo_viejo, t_nodo* el_nodo_nuevo) {
    int res;


    bson_t *bloquesDispViejo = bson_new();
    bson_t *bloquesDispNuevo = bson_new();
    bson_t *nodoViejo = armar_nodo_mongo(el_nodo_viejo,bloquesDispViejo);
    bson_t *nodoNuevo = armar_nodo_mongo(el_nodo_nuevo,bloquesDispNuevo);

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
	int estado;
	char* nombre, *ip, *puerto, *tamanio;
	t_nodo *nodoMongo;
	t_bloque_disponible* el_bloque;
	int bloque;
	query  = bson_new ();


	cursor = mongoc_collection_find (nodosMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){
		        if(bson_iter_find(&iter,"nombre")){
		        	nombre=string_new();
		        	string_append(&nombre,strdup(bson_iter_utf8(&iter,NULL)));
		           	}
		        if(bson_iter_find(&iter,"ip")){
		        	ip=string_new();
		        	string_append(&ip,strdup(bson_iter_utf8(&iter,NULL)));
		        	}
		        if(bson_iter_find(&iter,"puerto")){
		        	puerto=string_new();
		        	string_append(&puerto,strdup(bson_iter_utf8(&iter,NULL)));
		        	}
		        if(bson_iter_find(&iter,"tamanio")){
		        	tamanio=string_new();
		        	string_append(&tamanio,strdup(bson_iter_utf8(&iter,NULL)));
		        }
		        if(bson_iter_find(&iter,"estado"))
		        	estado = (int)bson_iter_int32(&iter);

		        nodoMongo = nodo_create(nombre,ip,puerto,tamanio,estado);
		        nodoMongo->bloquesDisponibles=list_create();


		        if(bson_iter_find(&iter,"bloquesDisponibles")){

		        	 bson_iter_array (&iter, &array_len,&array);
		        	 bson_iter_recurse(&iter, &child);
		        	 while(bson_iter_next(&child)){
		        		//const char* clave= bson_iter_key(&child);

		        		 //if(bson_iter_find(&child,"bloque")){
		        			 bloque = (int)bson_iter_int32(&child);
		        		 //}
		        		 el_bloque = bloque_disponible_create(bloque);
		        		 list_add(nodoMongo->bloquesDisponibles,el_bloque);
		        	 }

		        }


		    }

		}
	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_nodos,nodoMongo);
	    //free(nodoMongo);
	    //list_destroy(nodoMongo->bloquesDisponibles); //Limpio la lista bloques para el proximo archivo a cargar.
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);

	return list_size(lista_nodos);
}


bson_t* armar_filesystem_mongo(t_filesystem* el_fs){
		bson_t*filesystem = bson_new();

		bson_append_int32(filesystem, "index", 5, el_fs->index);

		bson_append_utf8(filesystem, "directorio", 10, el_fs->directorio, strlen(el_fs->directorio));

		bson_append_int32(filesystem, "padre", 5, el_fs->padre);
	return filesystem;
}

bool crear_filesystem_mongo(t_filesystem* el_fs) {
    bool res;

    bson_t *filesystem = armar_filesystem_mongo(el_fs);
    bson_error_t error;

    res= mongoc_collection_insert (filesystemMongo, MONGOC_INSERT_NONE, filesystem, NULL, &error);    // ver si no hay que crear una collection especial para la lista nodo.

	bson_destroy(filesystem);

	return res;
}

bool borrar_filesystem_mongo(t_filesystem* el_fs) {
    bool res;

    bson_t *filesystem =  armar_filesystem_mongo(el_fs);
    bson_error_t error;

    res = mongoc_collection_remove (filesystemMongo,MONGOC_DELETE_SINGLE_REMOVE,filesystem,NULL,&error);

	bson_destroy(filesystem);

	return res;
}

int grabar_filesystem_mongo(t_filesystem* el_fs_viejo, t_filesystem* el_fs_nuevo) {
    int res;

    bson_t *filesystemViejo = armar_filesystem_mongo(el_fs_viejo);
    bson_t *filesystemNuevo = armar_filesystem_mongo(el_fs_nuevo);




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

	t_filesystem *archivoFilesystem;
	int index, padre;
	char* directorio;
	query  = bson_new ();

	cursor = mongoc_collection_find (filesystemMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){

		    	if(bson_iter_find(&iter,"index"))
		    		index = (int)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"directorio")){
		        	directorio=string_new();
		        	string_append(&directorio,strdup(bson_iter_utf8(&iter,NULL)));
		           	}

		        if(bson_iter_find(&iter,"padre"))
		        	padre = (int)bson_iter_int32(&iter);

		        archivoFilesystem= filesystem_create(index,directorio,padre);

		        	 }

		        }

	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_filesystem,archivoFilesystem);
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	//free(archivoFilesystem);

	return list_size(lista_filesystem);
}


bson_t* armar_archivo_mongo(t_archivo*el_archivo, bson_t *listaBloques, bson_t *copiasArray){
	bson_t* archivo = bson_new();
	bson_append_utf8(archivo, "nombreArchivo", 13, el_archivo->nombreArchivo, strlen(el_archivo->nombreArchivo));
		bson_append_int32(archivo, "padre", 5, el_archivo->padre);
		bson_append_int32(archivo, "tamanio", 7, el_archivo->tamanio);
		bson_append_int32(archivo, "estado", 6, el_archivo->estado);

		bson_append_array_begin(archivo, "listaBloques", 12, listaBloques);
		int j;
		for (j=0; j < list_size(el_archivo->listaBloques); j++) {

			t_bloque* el_bloque = list_get(el_archivo->listaBloques, j); //Chequear si tiene que arrancar en 0 para el primer elemento


			bson_append_int32((void*)listaBloques, "bloque", 6, el_bloque->bloque);
			bson_append_array_begin((void*)listaBloques, "copias", 6, copiasArray);
			int i ;
			for (i= 0; i < list_size(el_bloque->listaCopias); i++) {

				t_array_copias* la_copia = malloc(sizeof(t_array_copias));
				la_copia = list_get(el_bloque->listaCopias,i);

				bson_append_utf8((void*)copiasArray, "nombreNodo", 10, la_copia->nombreNodo, strlen(la_copia->nombreNodo));
				bson_append_utf8((void*)copiasArray, "nroBloque", 9, la_copia->nro_bloque, strlen(la_copia->nro_bloque));


				}
			bson_append_array_end((void*)listaBloques, (void*)copiasArray);
		}
		bson_append_array_end((void*)archivo, (void*)listaBloques);

		return archivo;
}

bool crear_archivo_mongo(t_archivo* el_archivo) {
    bool res;
    bson_t *listaBloques = bson_new();
    bson_t *copiasArray = bson_new();
    bson_error_t error;


    bson_t * archivo= armar_archivo_mongo(el_archivo, listaBloques, copiasArray);


    res= mongoc_collection_insert (archivosMongo, MONGOC_INSERT_NONE, archivo, NULL, &error);    // ver si no hay que crear una collection especial para la lista nodo.
    // ver si no hay que crear una collection especial para la lista nodo.

    bson_destroy(listaBloques);
	bson_destroy(archivo);
	bson_destroy(copiasArray);

	return res;
}

bool borrar_archivo_mongo(t_archivo* el_archivo) {
    bool res;

    bson_t *listaBloques = bson_new();
    bson_t *copiasArray = bson_new();
    bson_error_t error;


    bson_t *archivo = armar_archivo_mongo(el_archivo, listaBloques, copiasArray);

    res = mongoc_collection_remove (archivosMongo,MONGOC_DELETE_SINGLE_REMOVE,archivo,NULL,&error);

    bson_destroy(listaBloques);
	bson_destroy(archivo);
	bson_destroy(copiasArray);

	return res;
}

int grabar_archivo_mongo(t_archivo* el_archivo_viejo, t_archivo* el_archivo_nuevo) {
    int res;
    bson_t *listaBloquesViejo = bson_new();
    bson_t *copiasArrayViejo = bson_new();
    bson_t *listaBloquesNuevo = bson_new();
    bson_t *copiasArrayNuevo = bson_new();

    bson_t *archivoViejo = archivoViejo = armar_archivo_mongo(el_archivo_viejo,listaBloquesViejo, copiasArrayViejo);

    bson_t *archivoNuevo = armar_archivo_mongo(el_archivo_nuevo,listaBloquesNuevo, copiasArrayNuevo);



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
	t_archivo *archivoMongo;
	char* nombre;
	int estado, padre;
	long unsigned tamanio;
	t_bloque* el_bloque;
	int bloque;
	t_array_copias* la_copia;
	char * nombreNodoC;
	char * nro_bloqueC;

	query  = bson_new ();

	cursor = mongoc_collection_find (archivosMongo, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		if(bson_iter_init(&iter,doc)){
		    while(bson_iter_next(&iter)){
		        if(bson_iter_find(&iter,"nombreArchivo")){
		        	nombre=string_new();
		        	string_append(&nombre,strdup(bson_iter_utf8(&iter,NULL)));
		           	}
		        if(bson_iter_find(&iter,"padre"))
		            padre = (int)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"tamanio"))
		        	 tamanio = (long unsigned)bson_iter_int32(&iter);

		        if(bson_iter_find(&iter,"estado"))
		        	 estado = (int)bson_iter_int32(&iter);

		        archivoMongo = archivo_create(nombre,tamanio,padre,estado);
		        if(bson_iter_find(&iter,"listaBloques")){

		        	 bson_iter_array (&iter, &array_len,&array);
		        	 bson_iter_recurse(&iter, &child);
		        	 while(bson_iter_next(&child)){

		        		 //if(bson_iter_find(&child,"bloque"))
		        			 bloque = (int)bson_iter_int32(&child);
		        			 el_bloque = bloque_create(bloque);

		        		int i=0;
		        		 if(bson_iter_find(&child,"copias")){


		        			 bson_iter_array (&child, &array_len,&array);
		        			 bson_iter_recurse(&child, &child2);
		        			 while(bson_iter_next(&child2)){

		        			// if(bson_iter_find(&child2,"nombreNodo")){
		        				 nombreNodoC=string_new();
		        				 string_append(&nombreNodoC,strdup(bson_iter_utf8(&child2,NULL)));
		        			 //}
		        			 if(bson_iter_next(&child2)){
		        				 nro_bloqueC=string_new();
		        				 string_append(&nro_bloqueC,strdup(bson_iter_utf8(&child2,NULL)));
		        				 }

		        			 i++;
		        			 //string_append(&nro_bloqueC,"Bloque");
		        			 la_copia = array_copias_create(nombreNodoC,nro_bloqueC);
		        			 list_add(el_bloque->listaCopias,la_copia);
		        			 printf("CARGO EL ELEMENTO %d que es %s del %s \n",list_size(el_bloque->listaCopias),la_copia->nombreNodo,la_copia->nro_bloque);
		        			 }


		        		 }
		        		 list_add(archivoMongo->listaBloques,el_bloque);
		        	 }

		        }


		    }

		}
	    printf ("%s\n", str);
	    bson_free (str);
	    list_add(lista_archivos,archivoMongo);
	    printf("%s, %d, %lu, %d",archivoMongo->nombreArchivo,archivoMongo->padre,archivoMongo->tamanio,archivoMongo->estado);
	    RecorrerListaBloques();
	    //list_destroy(archivoMongo->listaBloques); //Limpio la lista bloques para el proximo archivo a cargar.
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	//free(archivoMongo);

	return list_size(lista_archivos);
}

void cargar_listas_mongo(){
	int larch= leer_archivo_mongo();
	if (larch==0){
	printf("La lista de archivos esta vacias\n");
	}else{
	printf("Se cargaron %d elementos a la lista de archivos\n",larch);
	}
	int lnodo= leer_nodo_mongo();
	if (lnodo==0){
	printf("La lista de nodos esta vacias\n");
	}else{
	printf("Se cargaron %d elementos a la lista de nodos\n",lnodo);
	letra =letra+lnodo;
	}



	int lfs= leer_filesystem_mongo();
	if (lfs==0){
	printf("La lista de archivos esta vacias\n");
	}else{
	printf("Se cargaron %d elementos a la lista de archivos\n",lfs);
	}


}

int verBloquesArchivo(){
	t_archivo * el_archivo;
	t_bloque * el_bloque;
	t_array_copias * la_copia;
		char pathConArchivo[100];
		char* nombreArchivo = malloc(sizeof(char)*30);
		int padre;
		mostrarFilesystem();

		printf("Ingrese la ruta completa con el nombre del archivo para ver los bloques\n ");
		printf("Ejemplo: /home/utnso/temperatura.txt\n");
		scanf("%s",pathConArchivo);
		fflush(stdin);

		padre = validarDirectorios(pathConArchivo,&nombreArchivo);
		if(padre!=-1){

			el_archivo = buscarArchivoPorNombre(nombreArchivo,padre);


			int j=0;
			int k=0;
			printf("El archivo:"COLOR_VERDE"%s\n"DEFAULT,el_archivo->nombreArchivo);

		while(j<list_size(el_archivo->listaBloques)){
			el_bloque = list_get(el_archivo->listaBloques, j);
			printf("%d :: \n",el_bloque->bloque);

			while(k<list_size(el_bloque->listaCopias)){
			la_copia = list_get(el_bloque->listaCopias,k);
			printf("Copia %d: ", (k+1));
			printf("%s--",la_copia->nombreNodo);
			printf("%s  \n",la_copia->nro_bloque);
			k++;
			}
			k=0;
			j++;
		}
		return 1;
	} else {
		return 0;
	}

}

int borrarBloquesArchivo(){
	t_archivo * el_archivo;
	t_bloque * el_bloque;
	t_array_copias * la_copia;

		char pathConArchivo[100];
		char* nombreArchivo = malloc(sizeof(char)*30);
		int padre;
		mostrarFilesystem();

		printf("Ingrese la ruta completa con el nombre del archivo para borrar los bloques\n ");
		printf("Ejemplo: /home/utnso/temperatura.txt\n");
		scanf("%s",pathConArchivo);
		fflush(stdin);

		padre = validarDirectorios(pathConArchivo,&nombreArchivo);
		if(padre!=-1){
			char bloqueIng[10];
			int bloqueBorrar;
			el_archivo = buscarArchivoPorNombre(nombreArchivo,padre);
			int cantBloques = list_size(el_archivo->listaBloques);
			printf("Ingrese el numero de bloque que quiere borrar\n ");
			printf("El archivo tiene %d bloques\n",cantBloques);
			scanf("%s",bloqueIng);
			bloqueBorrar = CharAToInt(bloqueIng);
			fflush(stdin);
			if(bloqueBorrar> cantBloques || bloqueBorrar<0){
				return 0;
			}
			el_bloque = list_get(el_archivo->listaBloques,bloqueBorrar);
			int i = 0;
			printf("%d :: \n",el_bloque->bloque);

			while(i<list_size(el_bloque->listaCopias)){
			la_copia = list_get(el_bloque->listaCopias,i);
			printf("Copia %d: ", (i+1));
			printf("%s--",la_copia->nombreNodo);
			printf("%s  \n",la_copia->nro_bloque);
			i++;
			}

			char copiaIng[10];
			int copiaBorrar;
			printf("Ingrese la copia a borrar\n");
						scanf("%s",copiaIng);
						copiaBorrar = CharAToInt(copiaIng);
						fflush(stdin);
						if(copiaBorrar> list_size(el_bloque->listaCopias) || copiaBorrar<0){
							return 0;
						}
			borrar_archivo_mongo(el_archivo);
			la_copia = list_remove(el_bloque->listaCopias,copiaBorrar);
			t_nodo* el_nodo = buscarNodoPorNombre(la_copia->nombreNodo);
			borrar_nodo_mongo(el_nodo);
			char** array = string_split(la_copia->nro_bloque,"e");
			list_add(el_nodo->bloquesDisponibles,bloque_disponible_create(atoi(array[1])));
			crear_nodo_mongo(el_nodo);
			free(la_copia);
			crear_archivo_mongo(el_archivo);
			return 1;
	} else {
		return 0;
	}

}

/*
int copiarBloquesArchivo(){
	t_archivo * el_archivo;
	t_bloque * el_bloque;

		char pathConArchivo[100];
		char* nombreArchivo = malloc(sizeof(char)*30);
		int padre;
		mostrarFilesystem();

		printf("Ingrese la ruta completa con el nombre del archivo para copiar un bloque\n ");
		printf("Ejemplo: /home/utnso/temperatura.txt\n");
		scanf("%s",pathConArchivo);
		fflush(stdin);

		padre = validarDirectorios(pathConArchivo,&nombreArchivo);
		if(padre!=-1){
			char* bloqueIng;
			int bloqueCopia;
			el_archivo = buscarArchivoPorNombre(nombreArchivo,padre);
			int cantBloques = list_size(el_archivo->listaBloques);
			printf("Ingrese el numero de bloque que quiere copiar\n ");
					printf("El archivo tiene %d bloques\n",cantBloques);
					scanf("%s",bloqueIng);
					bloqueCopia = CharAToInt(bloqueIng);
					fflush(stdin);
					if(bloqueCopia> cantBloques || bloqueCopia<0){
						return 0;
					}
			el_archivo = buscarArchivoPorNombre(nombreArchivo,padre);
			el_bloque = list_get(el_archivo->listaBloques,bloqueCopia);


		return 1;
	} else {
		return 0;
	}

}
*/



int manejoDeBloques(){
	int seleccion;
	mostrarFilesystem();
	printf("Ingrese un comando:\n");
	printf("1: Ver 2: Borrar 3: Copiar ");
	scanf("%d", &seleccion);
	switch(seleccion){
		case 1: printf("Se eligio Ver\n");
				return verBloquesArchivo();
				break;
		case 2: printf("Se eligio Borrar\n");
				return borrarBloquesArchivo();
				break;
		case 3: printf("Se eligio Copiar\n");
				//return copiarBloquesArchivo();
				break;
		default: printf("Se ha ingresado un comando incorrecto.\n");
				break;
	}
	return 0;
}
