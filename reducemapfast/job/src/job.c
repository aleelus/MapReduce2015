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
	char *bufferArchivos=string_new();




	int tamanio=10,cantRafaga=1;
	int bytesRecibidos;
	int j=0,contadorArchivos=0;
	buffer = string_new();
	int desconexionCliente = 0;
	int g_Ejecutando = 1;

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



	bufferArchivos=procesarArchivos(bufferArchivos,contadorArchivos);

	printf("=======> Buffer a Enviar a MaRTA =======> %s \n",bufferArchivos);

	//CreoSocket();
	conectarMarta();
	EnviarDatos("2270", 10);
	EnviarDatos("213210file02.txt211file000.txt210file02.txt213resultado.txt1", 70);

	while ((!desconexionCliente) & g_Ejecutando) {
			//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.
			if (buffer != NULL )
				free(buffer);
			buffer = string_new();
			//buffer = NULL;

			//Recibimos los datos del cliente
			buffer = RecibirDatos(buffer, &bytesRecibidos,&cantRafaga,&tamanio);

			printf("BytesRecibidos:%d\n",bytesRecibidos);
			if (bytesRecibidos>0) {
				//Analisamos que peticion nos está haciendo (obtenemos el comando)
				/*emisor = ObtenerComandoMSJ(buffer);

				//Evaluamos los comandos
				switch (emisor) {
				case ES_JOB:
					implementoJob(&id,buffer,&cantRafaga,&mensaje);
					break;
				case ES_FS:
					printf("implementar atiendeFS\n");
					//implementoFS(buffer,&cantRafaga,&mensaje);
					break;
				case COMANDO:
					printf("Muestre toda la lista de Archivos:");
					RecorrerArchivos();
					break;
				case COMANDOBLOQUES:
					printf("Muestre toda la lista de Bloques:\n");
					RecorrerListaBloques(id);
					break;
				default:
					break;
				}*/
				printf("--------El BUFFER:%s\n",buffer);

				//longitudBuffer=strlen(mensaje);
				//printf("\nRespuesta: %s\n",buffer);
				// Enviamos datos al cliente.
				//EnviarDatos(socket, mensaje,longitudBuffer);
			} else
				desconexionCliente = 1;

		}

		//CerrarSocket(socket);

	//	return code;

	return 0;
}

char * procesarArchivos (char *bufferArch,int contArch){
	//2269
	//212220temperatura-2012.txt220temperatura-2013.txt213resultado.txt1

	int j=0;
	int tam=0,contDig=0,tamDigArch=0,cont=0,combiner=9;

	tamDigArch=contArch;
	while(tamDigArch>1){
		tamDigArch=tamDigArch/10;
		cont++;
	}
	string_append(&bufferArch,"2");
	string_append(&bufferArch,string_itoa(cont));
	string_append(&bufferArch,string_itoa(contArch));

	for(j=0;j<contArch;j++){
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
					*contadorArchivos=*contadorArchivos+1;//no me tomaba el *contadorArchivos++ XD
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
		memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket_Marta, bufferAux, BUFFERSIZE, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket_Marta);
		}

		digTamanio=PosicionDeBufferAInt(bufferAux,1);
		*tamanio=ObtenerTamanio(bufferAux,digTamanio);


	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char));
		memset(bufferAux, 0, *tamanio * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

		if ((*bytesRecibidos = *bytesRecibidos+recv(socket_Marta, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket_Marta);
		}
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket_Marta,
			(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}


int EnviarDatos(char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	printf("CantidadBytesAEnviar:%d\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket_Marta, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a al clientes. Socket: %d", socket_Marta);

	//Traza("ENVIO datos. socket: %d. buffer: %s", socket, (char*) buffer);

	//char * bufferLogueo = malloc(5);
	//bufferLogueo[cantidadDeBytesAEnviar] = '\0';

	//memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
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

/*#if 1 // MÉTODO MANEJO DE SOCKETS
	void CreoSocket()
		{
			int desc_socket=socket(AF_INET,SOCK_STREAM,0);
			struct sockaddr_in socketdest;
			if (desc_socket<0)
			{
				Error("No se pudo crear el socket");
			}
			else
			{
				socketdest.sin_family = AF_INET;
				socketdest.sin_port = htons(g_Puerto_Marta);
				socketdest.sin_addr.s_addr =  inet_addr(g_Ip_Marta);
				memset(&(socketdest.sin_zero), '\0', 8);
				bind(desc_socket,(struct sockaddr*)&socketdest, sizeof(struct sockaddr));
			}
		}
#endif
*/
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
