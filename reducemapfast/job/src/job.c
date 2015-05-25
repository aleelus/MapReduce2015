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
	//sem_init(&semaforoAccesoMemoria, 0, 1);
	//sem_init(&semaforoMarcosLibres, 0, 0);

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	//g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	//g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	logger = log_create(NOMBRE_ARCHIVO_LOG, "job", true, LOG_LEVEL_TRACE);
	char * buffer,*nomRes;
	char *bufferRafaga_Dos=string_new();
	char *bufferRafaga_Uno=string_new();
	char *bufferANodo = string_new();
	int cantidadRafagaMarta=1;

	int tamanio=10,cantRafaga=1;
	int bytesRecibidos;
	int j=0,contadorArchivos=0;
	buffer = string_new();
	int desconexionCliente = 0;
	int g_Ejecutando = 1;
	char *aux=string_new();
	t_job_a_nodo el_job;


	// Levantamos el archivo de configuracion.
	LevantarConfig();

	obtenerArrayArchivos(&contadorArchivos);
	nomRes=obtenerNombreResultado();
	//Agrego el nombre de Resultado al final de array_archivos
	array_archivos[contadorArchivos]=nomRes;
	//Muestro los archivos por pantalla
	printf("***********************\n");
	for(j=0;j<contadorArchivos;j++){
		printf("Archivo %d: %s\n",j,array_archivos[j]);
	}
	printf("Resultado: %s\n",array_archivos[contadorArchivos]);
	printf("***********************\n");


	bufferRafaga_Dos=procesarArchivos(bufferRafaga_Dos,contadorArchivos);
	bufferRafaga_Uno=obtenerRafaga_Uno(bufferRafaga_Uno,bufferRafaga_Dos);

	printf("=======> Buffer a Enviar a MaRTA RAFAGA 1=======> %s \n",bufferRafaga_Uno);
	printf("=======> Buffer a Enviar a MaRTA RAFAGA 2=======> %s \n",bufferRafaga_Dos);


	conectarMarta();
	EnviarDatos(bufferRafaga_Uno, strlen(bufferRafaga_Uno));
	cantidadRafagaMarta=2;

	while ((!desconexionCliente) & g_Ejecutando) {
			//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.
			if (buffer != NULL )
				free(buffer);
			buffer = string_new();

			//Recibimos los datos del cliente
			buffer = RecibirDatos(buffer, &bytesRecibidos,&cantRafaga,&tamanio);

			printf("BytesRecibidos:%d\n",bytesRecibidos);
			if (bytesRecibidos>0) {

				//printf("--------El BUFFER:%s\n",buffer);
				if(cantidadRafagaMarta==3){
					printf("Recibe Planificacion de Marta: %s\n",buffer);

					el_job=procesoJob(buffer);
					string_append(&bufferANodo,"21");
					aux=abrir_Mapper(aux);
					string_append(&bufferANodo,aux);
					free(aux);
					aux=obtenerSubBuffer(g_Mapper);
					string_append(&bufferANodo,aux);
					free(aux);
					aux=obtenerSubBuffer(el_job.archResultado);
					string_append(&bufferANodo,aux);
					free(aux);

					//Me conecto con el Nodo
					conectarNodo(el_job);
					//Le envio Nodo
					EnviarDatos(bufferANodo, strlen(bufferANodo));



					cantidadRafagaMarta=1;
				}
				else if(cantidadRafagaMarta==2){
					EnviarDatos(bufferRafaga_Dos, strlen(bufferRafaga_Dos));
					cantidadRafagaMarta=3;
					cantRafaga=3;
				}


			} else
				desconexionCliente = 1;

		}

		//CerrarSocket(socket);

	//	return code;

	return 0;
}

void conectarNodo(t_job_a_nodo el_job){

	int socket_nodo;
	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON UN NODO
	log_info(logger, "Intentando conectar a %s\n",el_job.nodo);

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(el_job.ip, el_job.puerto, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_nodo");
	}

	if ((socket_nodo = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_nodo");
	}
	if (connect(socket_nodo, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_nodo");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas


}

t_job_a_nodo procesoJob (char *buffer){

	//415 NodoA   19 127.0.0.1    14 6000      18 Bloque30      213 resultado.txt

	t_job_a_nodo el_job;
	int pos=1;


	el_job.nodo=DigitosNombreArchivo(buffer,&pos);
	el_job.ip=DigitosNombreArchivo(buffer,&pos);
	el_job.puerto=DigitosNombreArchivo(buffer,&pos);
	el_job.bloque=DigitosNombreArchivo(buffer,&pos);
	el_job.archResultado=DigitosNombreArchivo(buffer,&pos);

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
	while(tam>1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioNombre));
	string_append(&aux,nombre);

	return aux;
}

char* abrir_Mapper(char *aux){

	FILE *f;
	int tamanioArchivo=0;
	float tam=0;
	int cont=0;
	char *aux2=string_new();

	f= fopen(g_Mapper,"rb");
	fseek(f,0,SEEK_END);
	tamanioArchivo=ftell(f);
	rewind(f);
	aux2=(char*)malloc(sizeof(char)*tamanioArchivo);
	memset(aux2, 0, tamanioArchivo * sizeof(char));
	fread(aux2,1,tamanioArchivo,f);

	tam=tamanioArchivo;
	while(tam>1){
		tam=tam/10;
		cont++;
	}
	string_append(&aux,string_itoa(cont));
	string_append(&aux,string_itoa(tamanioArchivo));
	string_append(&aux,aux2);

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

char* obtenerRafaga_Uno(char *buffer,char* bufferAux){
	int tamanio=0,cont=0;
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
	int contDig=0,tamDigArch=0,cont=0,combiner=9;
	float tam=0;

	tamDigArch=contArch;
	while(tamDigArch>1){
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
		while(tam>1){
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
char* obtenerNombreResultado(){
	char **array;
	int cont=0;
	array =(char**) malloc (strlen(g_Resultado));
	array=string_split(g_Resultado,"/");
	while (array[cont]!=NULL){
				cont++;
	}
	return array[cont-1];
}
void obtenerArrayArchivos(int *contadorArchivos){

	char **array;
	int cont=0,j=0,i=0;

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
		while (array[cont]!=NULL){
				for(j=0;j<strlen(array[cont]);j++){
					if(array[cont][j]=='.'){
							array_archivos[i]=array[cont];
							i++;
					}
				}
				cont++;
		}

}



int ObtenerTamanio (char *buffer , int dig_tamanio){
	int x,digito,aux=0;
	for(x=0;x<dig_tamanio;x++){
		digito=PosicionDeBufferAInt(buffer,2+x);
		aux=aux*10+digito;
	}
	return aux;
}

char* RecibirDatos(char *buffer, int *bytesRecibidos,int *cantRafaga,int *tamanio) {
	*bytesRecibidos = 0;
	char *bufferAux= malloc(1);
	int digTamanio;
	if (buffer != NULL ) {
		free(buffer);
	}

	if(*cantRafaga==1){
		bufferAux = realloc(bufferAux,BUFFERSIZE * sizeof(char));
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket_Marta, bufferAux, BUFFERSIZE, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde Marta. Socket: %d",socket_Marta);
		}

		digTamanio=PosicionDeBufferAInt(bufferAux,1);
		*tamanio=ObtenerTamanio(bufferAux,digTamanio);



	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char));
		memset(bufferAux, 0, *tamanio * sizeof(char)); //-> llenamos el bufferAux con ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket_Marta, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket_Marta);
		}

	}else{

		bufferAux = realloc(bufferAux,100* sizeof(char));
		memset(bufferAux, 0, 100 * sizeof(char)); //-> llenamos el bufferAux con ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket_Marta, bufferAux, 100, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket_Marta);
		}
		*cantRafaga=1;

	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket_Marta,
			(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo.
}


int EnviarDatos(char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket_Marta, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a al clientes. Socket: %d", socket_Marta);

	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket_Marta,
			(char*) buffer);

	return bytecount;
}


void conectarMarta() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON MARTA
	log_info(logger, "Intentando conectar a Marta\n");
	//char * puerto = "7000";
	//conectar con Marta
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(g_Ip_Marta, g_Puerto_Marta, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_Marta");
	}

	if ((socket_Marta = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_Marta");
	}
	if (connect(socket_Marta, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_Marta");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

}



#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Obtenemos la ip de Marta
		if (config_has_property(config, "IP_MARTA")) {
			g_Ip_Marta = config_get_string_value(config, "IP_MARTA");
			//printf("IP:%s",g_Ip_Marta);
		} else
			Error("No se pudo leer el parametro IP_MARTA");

		// Obtenemos el puerto de escucha de Marta
		if (config_has_property(config, "PUERTO_MARTA")) {
			g_Puerto_Marta = config_get_string_value(config,"PUERTO_MARTA");
			//printf("IP:%d",g_Puerto_Marta);
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
