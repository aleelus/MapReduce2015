/*o
 ============================================================================
 Name        : nodo.c
 Author      : Los Barderos
 Version     : 1.0
 Copyright   : Los Barderos - UTN FRBA 2015
 Description : Trabajo Practivo Sistemas Operativos 1C 2015
 Testing	 :
 ============================================================================
 */

#include "nodo.h"

// Crear el archivo de espacio de datos datos.bin con "truncate -s 1G datos.bin"
int main(int argv, char** argc) {
	pagina = sysconf(_SC_PAGE_SIZE);
	logger = log_create(NOMBRE_ARCHIVO_LOG, "nodo", true, LOG_LEVEL_TRACE);

	contador = 0;

	sem_init(&semaforoScript,1,1);
	sem_init(&semaforoH,1,0);
	sem_init(&semaforoGrabar,1,1);
	sem_init(&semaforoSetBloque,1,1);
	sem_init(&semaforoGetBloque,1,1);
	sem_init(&semaforoNodo,1,1);
	sem_init(&semCon,1,1);
	//long unsigned cur = 0;
	//printf("LINEA:%s\n",dameLinea("4resultado.txt",cur));
	//while(1);
	tamanioTotal = 0;

	lista_Bloques = list_create();

	//sem_init(&semaforo,1,1);
	letra = 'A';

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	//Abro el archivo de espacio de datos donde voy a leer y escribir bloques
	if(( archivoEspacioDatos = fopen(g_Archivo_Bin, "r+b") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", g_Archivo_Bin, strerror(errno));
		abort();
			}

	fseek(archivoEspacioDatos, 0L, SEEK_END);

	tamanioDataBin = 1024*1024*g_Tamanio_Bin;

	//tamanioDataBin = ftell(archivoEspacioDatos);

	printf("Tamaño de Archivo de Datos:%lu\n",tamanioDataBin);
	//Prueba getFileContent
	//char* temp_file = getFileContent("temporalPrueba.tmp");

	conexionAFs();

	//Socket para escuchar al Job, FS y otros Nodos
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

	//Prueba setBloque
	printf("Escriba el numero de bloque que quiere escribir.\n");
	int numero;
	scanf("%d", &numero);
	printf("Escriba lo que quiere grabar en el bloque.\n");
	char* datos = "Este es el bloque 2";
	setBloque(numero,datos);

	//Prueba getBloque
	printf("Escriba el numero de bloque que quiere ver.\n");
	scanf("%d", &numero);
	char * bloqueSolicitado;
		bloqueSolicitado = getBloque(numero);
		printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloqueSolicitado);
		munmap( bloqueSolicitado, TAMANIO_BLOQUE ); //Desmapeo el bloque obtenido, IMPORTANTE!

	printf("Ok\n");
	fclose(archivoEspacioDatos);
	return 0;
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

void conexionAFs(){
	int cont;
	int bytesRecibidos,cantRafaga=1,tamanio;
	char*buffer = string_new();
	char*bufferR = string_new();
	char*bufferE = string_new();
	char*aux;

	if(conectarFS(&socket_Fs,g_Ip_Fs,g_Puerto_Fs)){
		//ENVIO a FILESYSTEM(la segunda rafaga)
		//31210127.0.0.1143000
		string_append(&buffer,"31");
		aux=obtenerSubBuffer(g_Ip_Nodo);
		string_append(&buffer,aux);
		aux=obtenerSubBuffer(string_itoa(g_Puerto_Nodo));
		string_append(&buffer,aux);
		aux=obtenerSubBuffer(string_itoa(ftell(archivoEspacioDatos)));
		string_append(&buffer,aux);

		string_append(&bufferE,"3");
		cont = cuentaDigitos(strlen(buffer));
		string_append(&bufferE,string_itoa(cont));
		string_append(&bufferE,string_itoa(strlen(buffer)));

		//Primera Rafaga
		EnviarDatos(socket_Fs,bufferE, strlen(bufferE));

		bufferR = RecibirDatos(socket_Fs,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
		//Recibo respuesta de FS
		if(bufferR!=NULL){
			//Segunda Rafaga
			EnviarDatos(socket_Fs,buffer, strlen(buffer));
			bufferR = RecibirDatos(socket_Fs,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
			printf("\nNodo Conectado al Fs!");
		} else {
			printf("No se pudo conectar al FS\n");
		}
		free(buffer);
		free(bufferR);
		free(bufferE);
	}
}



int conectarFS(int * socket_Fs, char* ipFs, char* puertoFs) {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON NODO
	log_info(logger, "Intentando conectar a nodo\n");
	//conectar con Nodo
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int conexionOk = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP


	if (getaddrinfo(ipFs, puertoFs, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(logger,
				"ERROR: cargando datos de conexion socket_FS");
	}

	if ((*socket_Fs = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(logger, "ERROR: crear socket_FS");
	}
	if (connect(*socket_Fs, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(logger, "ERROR: conectar socket_FS");
	} else {
		conexionOk = 1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return conexionOk;
}


void grabarScript(char* nombreScript, char* codigoScript){
		int tamanio = strlen(codigoScript)+1;
		//Tamanio del codigo que se quiere grabar en el script
		FILE* archivoScript = fopen(nombreScript, "w");
		//Creo el script con el nombre especificado
		fwrite(codigoScript, sizeof(char), tamanio, archivoScript);
		//Grabo en el archivo el bloque
		free(codigoScript);
		//Libero el puntero
		fclose(archivoScript);
		//Cierro el script
}

int tamanio_archivo(char* nomArch){
	struct stat buf;
	stat(nomArch, &buf);
	return buf.st_size;
}

char* getBloque(int numero){
	char* bloque = string_new();
	int fd= fileno(archivoEspacioDatos);
	long unsigned offset = numero*(TAMANIO_BLOQUE/pagina);
	//printf(COLOR_VERDE"Offset:%lu\n"DEFAULT,offset);

	sem_wait(&semaforoGetBloque);
	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ, MAP_SHARED, fd, offset*pagina)) == MAP_FAILED){
			//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
			fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
			abort();
		}
	sem_post(&semaforoGetBloque);
	//printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);
	//string_append(&bloque,"\n");
	return bloque;
}

/*void setBloque(int numero, char*datos){
	if(( tamanio_archivo(g_Archivo_Bin) <= (numero*TAMANIO_BLOQUE) )){
			//Si no se pudo abrir, imprimir el error y abortar;
			printf(COLOR_VERDE"El bloque no existe en el archivo. \n"DEFAULT);
			abort();
		}
	char* bloque;
	int fd= fileno(archivoEspacioDatos);
	long unsigned offset = numero*(TAMANIO_BLOQUE/pagina);
	//sem_wait(&semaforo);

	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset*pagina)) == MAP_FAILED){
				//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
				fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
				abort();
	} else {
		memcpy(bloque, datos, strlen(datos)+1);
		munmap(bloque,TAMANIO_BLOQUE);
		printf(COLOR_VERDE"Se realizo el setBloque del bloque:%d\n"DEFAULT,numero);
	}
	//sem_post(&semaforo);

 //Copia los datos a grabar en el bloque auxiliar
	//printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);


}*/

/* char* getBloque(int numero){
	char* bloque = malloc(TAMANIO_BLOQUE);
	long int offset=numero*TAMANIO_BLOQUE;
	fseek(archivoEspacioDatos, offset, SEEK_SET);
	fread(bloque, sizeof(char), TAMANIO_BLOQUE, archivoEspacioDatos);
	return bloque;
}*/


void setBloque(int numero, char*datos){
	if(( tamanioDataBin <= (numero*TAMANIO_BLOQUE) )){
			//Si no se pudo abrir, imprimir el error y abortar;
			printf("El bloque no existe en el archivo. \n");
			abort();
		}
	long int tamanio = TAMANIO_BLOQUE; //Tamanio del bloque 20mb
	char*txtBloq = malloc(TAMANIO_BLOQUE);
	memset(txtBloq, '\0', tamanio * sizeof(char));
	//Rellena de 0 el txtBloq que se va a grabar
	memcpy(txtBloq, datos, strlen(datos)+1);
	//Copia los datos a grabar en el bloque auxiliar
	long int offset=numero*TAMANIO_BLOQUE;

	fseek(archivoEspacioDatos, offset, SEEK_SET);
	//Posicion el puntero en el bloque pedido
	sem_wait(&semaforoSetBloque);
	fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
	sem_post(&semaforoSetBloque);
	//Grabo en el archivo el bloque
	free(txtBloq);
	//Libero el puntero

}

char * armarRutaTemporal( char *nombre){
		//static char ruta[] = g_Dir_Temp;
		char* nombreT = string_new();
		//string_append(&nombreT,ruta);
		string_append(&nombreT,g_Dir_Temp);
		string_append(&nombreT,"/");
		string_append(&nombreT,nombre);
		char* fname= malloc(PATH_MAX);
		strcpy(fname,nombreT);
		return fname;
}


char * getFileContent(char* nombre){
	FILE* archivoTemporal;

	char* fname = armarRutaTemporal(nombre);
	printf("%s \n",fname);
	char* contenido= malloc(tamanio_archivo(fname));
	//Busca el archivo temporal nombre y devuelve su contenido
	if(( archivoTemporal = fopen(fname, "r+b") ) == NULL){
			//Si no se pudo abrir, imprimir el error y abortar;
			fprintf(stderr, "Error al abrir el archivo '%s': %s\n", fname, strerror(errno));
			return NULL;
		}
	fread(contenido, sizeof(char),tamanio_archivo(fname),archivoTemporal);
	printf("Filename is %s\n", fname);
	printf("Received string: %s", contenido);
	return contenido;
}

/* void ejecutarScript (char* scriptName,char* outputFilename,char* input)
{
FILE *stdin;
char *comandoScript = string_new();
char *cambioModo = string_new();
string_append(&comandoScript, "./");
string_append(&comandoScript, scriptName);
string_append(&comandoScript, " | sort");
string_append(&comandoScript, " > ");
string_append(&comandoScript, "/tmp/");
string_append(&comandoScript, outputFilename);
string_append(&cambioModo, "chmod u+x ");
string_append(&cambioModo, scriptName);
//chmod u+x script.sh
//doy permisos de ejecucion al script
system(cambioModo);
//si es modo w devuelve stdin si es r devuelve stdout uno u otro
stdin = popen (comandoScript, "w");
if (!stdin)
{
fprintf (stderr,
"incorrect parameters or too many files.\n");
//return EXIT_FAILURE;
}
fprintf(stdin, "%s\n",input);
if (pclose (stdin) != 0)
{
fprintf (stderr,
"Could not run more or other error.\n");
}
free(comandoScript);
free(cambioModo);
}*/


void permisosScript(char * nombre){
	char *cambioModo = string_new();
	string_append(&cambioModo, "chmod u+x ");
	string_append(&cambioModo, nombre);
	//chmod u+x script.sh
	//doy permisos de ejecucion al script
	system(cambioModo);
}

int enviarDatos(int socket, void *buffer) {
	int bytecount;

	if ((bytecount = send(socket, buffer, BUFFERSIZE, 0)) == -1) {
		printf("ERROR: no se pudo enviar información. \n");
		log_info(logger, "ERROR: no se pudo enviar información. \n");
	}
	log_info(logger, "ENVIO datos. socket: %d. buffer: %s", socket,
			(char*) buffer);

	return (bytecount);
}

/*int runScriptFile(char* script,char* archNom, char* input)
{
    //int outfd[2];
    //int infd[2];
	pid_t id;
    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);

    id=fork();
    if(id==0) {


        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);
        printf("Entro a FORK\n");
        Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */

/*        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

        execl(script, script, (char*)0);



    } else if(id==-1) {
    	printf("Error");
    } else {

        char buffer[50]; //definir el tamanio de este buffer

        int count;

        close fds not required by parent */
/*        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        //printf("lo que se va a escribir es %s y la longitud es %d\n", input, strlen(input));

        // Write to child’s stdin
        write(PARENT_WRITE_FD, input, (strlen(input)));

        printf("NO ENTRO\n");

        close(PARENT_WRITE_FD);

        count = 1;
        FILE* archSalida = fopen(archNom, "w");
        while(count != 0){
        	// Read from child’s stdout
        	count = read(PARENT_READ_FD, buffer, sizeof(buffer)-1);
        	if (count >= 0) {
        		buffer[count] = 0;
        		fwrite(buffer,sizeof(char),strlen(buffer),archSalida);
        		//printf("lo que se grabo %s\n", buffer);
        	} else {
        		printf("IO Error\n");
        		return 0;
        	}
        }
     }

     return 1;
}*/

int runScriptFile(char* script,char* archNom, char* input)
{

      int a[2],cont=0,status;
      pipe(a);
      char **array = string_split(script,"/");
      while(array[cont]!=NULL){
    	  cont++;
      }

    char *argv[]={ array[cont-1], "-q", 0};
    if(!fork()) {


    	printf("HIJO\n");
    	close(a[1]);

    	dup2(a[0],STDIN_FILENO);

    	close(STDOUT_FILENO);
    	FILE *fd = fopen(archNom, "w" );
        close(a[0]);
        dup(STDOUT_FILENO);
        //sem_wait(&semaforoH);
        execl(argv[0],argv[0],NULL);
        exit(0);




    } else {
    	printf("PADRE\n");

		close(a[0]);

		write(a[1],input,strlen(input));
		//sem_post(&semaforoH);
		printf("ACA TERMINA\n");

		close(a[1]);
		wait(&status);


    }

    return 1;
}



#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	log_info(logger, "Archivo de configuración: %s", PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Preguntamos y obtenemos el puerto donde esta escuchando el filesystem
		if (config_has_property(config, "PUERTO_FS")) {
			g_Puerto_Fs = config_get_string_value(config, "PUERTO_FS");
		} else {
			Error("No se pudo leer el parametro PUERTO_FS");
		}
		// Preguntamos y obtenemos la ip donde esta ejecutandose el filesystem
		if (config_has_property(config, "IP_FS")) {
			g_Ip_Fs = config_get_string_value(config,"IP_FS");
		} else{
			Error("No se pudo leer el parametro IP_FS");
		}


		// Obtenemos el nombre del archivo con los bloques
		if (config_has_property(config, "ARCHIVO_BIN")) {
			g_Archivo_Bin = config_get_string_value(config, "ARCHIVO_BIN");
		} else{
			Error("No se pudo leer el parametro ARCHIVO_BIN");
		}

		// Obtenemos el tamanio del archivo con los bloques
		if (config_has_property(config, "TAMANIO_BIN")) {
			g_Tamanio_Bin = config_get_int_value(config, "TAMANIO_BIN");
		} else{
			Error("No se pudo leer el parametro TAMANIO_BIN");
		}

		// Obtenemos el nombre del directorio temporal
		if (config_has_property(config, "DIR_TEMP")) {
			g_Dir_Temp = config_get_string_value(config, "DIR_TEMP");
		} else{
			Error("No se pudo leer el parametro DIR_TEMP");
		}

		// Obtenemos si es nodo nuevo
		if (config_has_property(config, "NODO_NUEVO")) {
			g_Nodo_Nuevo = config_get_string_value(config, "NODO_NUEVO");
		} else{
			Error("No se pudo leer el parametro NODO_NUEVO");
		}

		// Obtenemos la ip del nodo
		if (config_has_property(config, "IP_NODO")) {
			g_Ip_Nodo = config_get_string_value(config, "IP_NODO");
		} else{
			Error("No se pudo leer el parametro IP_NODO");
		}

		// Obtenemos el puerto de escucha del nodo
		if (config_has_property(config, "PUERTO_NODO")) {
			g_Puerto_Nodo = config_get_int_value(config, "PUERTO_NODO");
		} else{
			Error("No se pudo leer el parametro PUERTO_NODO");
		}

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

void SetearErrorGlobal(const char* mensaje, ...) {
	va_list arguments;
	va_start(arguments, mensaje);
	if (g_MensajeError != NULL )
		// NMR COMENTADO POR ERROR A ULTIMO MOMENTO	free(g_MensajeError);
		g_MensajeError = string_from_vformat(mensaje, arguments);
	va_end(arguments);
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
	//char* aux = malloc(1 * sizeof(char));
	//sprintf(aux, "%c", x);
	numero = strtol(aux, (char **) NULL, 10);

	if (aux != NULL )
		free(aux);
	return numero;
}



int PosicionDeBufferAInt(char* buffer, int posicion) {
	long unsigned logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return ChartToInt(buffer[posicion]);
}

int PosicionDeBufferALong(char* buffer, long unsigned posicion) {
	long unsigned logitudBuffer = 0;
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

long unsigned ObtenerTamanioLong (char *buffer , int posicion, int dig_tamanio){
	long unsigned x,aux=0;
	int digito=0;
	for(x=0;x<dig_tamanio;x++){
		digito=PosicionDeBufferALong(buffer,posicion+x);
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
		printf(COLOR_VERDE"TAMAÑO:%d\n"DEFAULT,*tamanio);


	}else if(*cantRafaga==2){
		bufferAux = realloc(bufferAux,*tamanio * sizeof(char)+1);
		memset(bufferAux, 0, *tamanio * sizeof(char)+1); //-> llenamos el bufferAux con barras ceros.

		printf("ANTES BYTESRECIBIDO:%d\n",*bytesRecibidos);
		if ((*bytesRecibidos = *bytesRecibidos+recv(socket, bufferAux, *tamanio, 0)) == -1) {
			Error("Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",socket);
		}
		printf("DESPUES BYTESRECIBIDO:%d\n",*bytesRecibidos);
	}

	if(strlen(bufferAux)<100){
		log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d", socket,(char*) bufferAux, strlen(bufferAux));
	} else {
		log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%d y mi rafaga:%d", socket,"soy grande", strlen(bufferAux),*cantRafaga);
	}
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}

int EnviarDatos(int socket, char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud
	//sleep(g_Retardo / 1000);

	int bytecount;

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información al cliente. Socket: %d", socket);

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

void AtiendeFS (t_bloque ** bloque,char *buffer){
	//SET_BLOQUE 1311210holaholaho
	//BUFFER RECIBIDO = 13110820971520
	//--- 1: soy FS 3: quiero un set bloque 2:cant digitos numero bloque 10:numero de bloque a grabar
	// 8: cant de dig de tamanio de lo que necesitamos que grabe 20971520: tamaño en bytes
		//semaforo
		estado = 1;
		char *contenidoBloq;
		//char*buffer2 = malloc(32);
		//memset(&buffer2,0,32);
		int numeroBloq,digitosCantDeDigitosBloque;
		int digitosCantDeDigitosTamanioBloq,cantDigNumBloque=0;
		long unsigned posActual=0,tam=0;

		digitosCantDeDigitosBloque=PosicionDeBufferAInt(buffer,2);
		//printf("CANT DIGITOS:%d\n",digitosCantDeDigitosBloque);
		cantDigNumBloque=ObtenerTamanio(buffer,3,digitosCantDeDigitosBloque);
		numeroBloq=ObtenerTamanio(buffer,4,cantDigNumBloque);
		//printf(COLOR_VERDE"NUMERO DE BLOQUE:%d\n"DEFAULT,numeroBloq);
		posActual=3+cantDigNumBloque+digitosCantDeDigitosBloque;
		//131218820971517
		digitosCantDeDigitosTamanioBloq = PosicionDeBufferAInt(buffer,posActual);
		//printf("Cantidad Digitos de contenido de bloque:%d\n",digitosCantDeDigitosTamanioBloq);
		tam= ObtenerTamanioLong(buffer,posActual+1,digitosCantDeDigitosTamanioBloq);
		//printf("Tamanio del contenido del bloque: %lu\n",tam);
		posActual=posActual+digitosCantDeDigitosTamanioBloq+1;
		//printf("Tamanio BLOQUE POSTA:%d\n",tamanioBloque);
		//printf("Posicion Actual%d Tamanio Bloque:%d\n",posActual,tamanioBloque);
		//buffer2 = string_substring(buffer,0,30);
		contenidoBloq=string_substring(buffer,posActual,tam);
		//contenidoBloq=DigitosNombreArchivo(buffer,&posActual);
		//printf(COLOR_VERDE"Buffer:%s\n"DEFAULT,buffer2);
		//free(buffer);
		//*bloque = bloque_create(numeroBloq,contenidoBloq);
		*bloque = (t_bloque*)malloc(sizeof(t_bloque));
		(*bloque)->numeroBloque = numeroBloq;
		(*bloque)->contenidoBloque = string_new();
		(*bloque)->tamanioBloque = strlen(contenidoBloq);
		string_append(&(*bloque)->contenidoBloque,contenidoBloq);
		list_add(lista_Bloques,*bloque);
		free(contenidoBloq);

}

void AtiendeJob (t_job ** job,char *buffer, int *cantRafaga){
	//Cadena recibida del Job
	//21270xxxxxxxxx212barriendo.sh233213resultado.txt
	//2: soy Job 1: recibo tarea 2: cant de dig del tamaño del .sh 70: tamaño del sh
	//2: cant de dig de tamaño del nombre del .sh 12: tamaño del nombre del .sh y luego el nombre
	//2: cant de dig del numero de bloque 33: numero de bloque, el nombre del arch de resultado lo sacas jajaja

	//semaforo
	estado = 1;
	char *nArchivoSH;
	int digitosCantDeDigitos=0,digitosCantDeDigitosSH;
	char * el_Bloque;
	int posActual=0;
	char * fileSH,*nArchivoResultado;

	digitosCantDeDigitosSH=PosicionDeBufferAInt(buffer,2);
	//printf("Cantidad Digitos de tamaño de contenido SH:%d\n",digitosCantDeDigitosSH);
	ObtenerTamanio(buffer,3,digitosCantDeDigitosSH);
	//printf("Tamanio del SH: %d\n",tamanioSH);
	posActual=2+digitosCantDeDigitos;

	fileSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Contenido de archivo SH:%s\n",fileSH);

	nArchivoSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo SH:%s\n",nArchivoSH);

	//printf("Posicion Actual:%d\n",posActual);
	//printf("Cantidad de digitos del numero de bloque:%d\n",digitosTamanioBloque);
	el_Bloque=DigitosNombreArchivo(buffer,&posActual);
	//printf("bloque: %s\n",el_Bloque);

	nArchivoResultado=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo de Resultado:%s\n",nArchivoResultado);

	*job = job_create(nArchivoSH,fileSH,el_Bloque,nArchivoResultado);
	*cantRafaga=1;
}

t_nodo* buscarNodoPorNombre(char* nombre,t_list* lista_nodos){
	t_nodo* el_nodo = malloc(sizeof(t_nodo));
	bool _true(void *elem){
		return ((!strcmp(((t_nodo*) elem)->nombreNodo,nombre)));
	}
	el_nodo = list_find(lista_nodos, _true);
	return el_nodo;
}

long unsigned obtenerCantidadLineas(int socket,char* bloque,long unsigned* tamanioArchivo){
	char * bufferE = string_new();
	char * bufferR = string_new();
	char * buffer = string_new();
	int cantRecibida=0,tamanio,cantRafaga=2;
	char* cantidadLineas, *tamanioArc;

	//segunda rafaga
	string_append(&buffer,"1");
	string_append(&buffer,obtenerSubBuffer(bloque));

	//primer rafaga
	string_append(&bufferE,"3");
	string_append(&bufferE,string_itoa(cuentaDigitos(strlen(buffer))));
	string_append(&bufferE,string_itoa(strlen(buffer)));

	printf("Primera Rafaga:%s\n",bufferE);

	sem_wait(&semCon);
	EnviarDatos(socket,bufferE,strlen(bufferE));


	tamanio = strlen("*");
	bufferR = RecibirDatos(socket,bufferR,&cantRecibida,&cantRafaga,&tamanio);
	int p,bandera=0;
	for(p=0;p<strlen(bufferR);p++){
		if(bufferR[p]=='*'){
			bandera=1;
		}
	}

	if(bandera == 0){
		printf("NO ASTERISCO!!!\n)");
		return 0;
	}

	//sleep(1);

	printf("Segunda Rafaga:%s\n",buffer);
	//sem_wait(&semaforoNodo);


	EnviarDatos(socket,buffer,strlen(buffer));
	sem_post(&semCon);
	bufferR = string_new();

	cantRafaga=2;
	tamanio=100;
	bufferR = RecibirDatos(socket,bufferR,&cantRecibida,&cantRafaga,&tamanio);

	//12201843434832
	int pos=0;
	cantidadLineas=DigitosNombreArchivo(bufferR,&pos);
	tamanioArc=DigitosNombreArchivo(bufferR,&pos);
	*tamanioArchivo = atoi(tamanioArc);
	return atoi(cantidadLineas);
}

long unsigned cantidadLineasArchivo(char* bloque, long unsigned* tamanio){
	FILE * fA;
	fA = fopen(bloque,"r");
	if(fA!=NULL){
		printf("ABRIO\n");
		char s2[1] = "\n";
		char * ptr;
		int i=0;

		fseek(fA,0L,SEEK_END);
		long unsigned tamanioA=ftell(fA);
		*tamanio = tamanioA;
		rewind(fA);

		char * buffer = malloc(tamanioA+1);
		memset(buffer,0,tamanioA+1);
		fread(buffer,1,tamanioA,fA);
		ptr = strtok( buffer, s2 );    // Primera llamada => Primer token
		if(ptr==NULL) return 0;
		i++;
		while( (ptr = strtok( NULL, s2 )) != NULL ){
			i++;
		}

		fclose(fA);
		return i;
	} else {
		return 0;
	}

}

long unsigned cantidadDeLineas(t_list** bloques,t_list* nodos){
	int i;
	long unsigned cantidadLineas=0,tamanioA=0;
	t_nodo* nodo;
	t_bloque_script* bloque;
	for(i=0;i<list_size(*bloques);i++){
		bloque = list_get(*bloques,i);
		printf("NOMBRE ARCHIVO BLOQUE:%s\n",bloque->bloque);
		if(!bloque->pertenece){
			nodo = buscarNodoPorNombre(bloque->nombreNodo,nodos);
			printf("SOCKET NRO : %d\n",nodo->socket);
			cantidadLineas = cantidadLineas + obtenerCantidadLineas(nodo->socket,bloque->bloque,&tamanioA);
		} else {
			cantidadLineas = cantidadLineas + cantidadLineasArchivo(bloque->bloque,&tamanioA);
		}
		printf("%d) CANTIDAD LINEAS:%lu\n",i,cantidadLineas);
		bloque->tamanio = tamanioA;
	}

	return cantidadLineas;
}

char* burbujeo(t_list*bloques,int*indice){
	t_bloque_script *bloque;

	bool _ordenarPorNodo(t_bloque_script* bloque1, t_bloque_script* bloque2){
	if(strcmp(bloque1->contenidoBloque,bloque2->contenidoBloque)<0)
		return true;
	else
		return false;
	}
	if(bloques!=NULL){
		list_sort(bloques, (void*)_ordenarPorNodo);
		*indice = 0;
		bloque = list_get(bloques,0);
		printf("MENOR : %s\n",bloque->contenidoBloque);
		if(bloque->contenidoBloque==NULL) return NULL;

		bloque->fsd = bloque->fsd + strlen(bloque->contenidoBloque)+1;
		bloque->nuevaLinea = 1;

		return bloque->contenidoBloque;
	} else {
		return NULL;
	}
}

char * dameMenor(t_list* nodos, t_list** bloques){
	int indice;
	char * menor = burbujeo(*bloques,&indice);
	t_bloque_script * bloque = list_get(*bloques,indice);
	if(bloque->fsd == bloque->tamanio){
		list_remove(*bloques,indice);
		free(bloque->contenidoBloque);
		free(bloque->bloque);
		free(bloque->nombreNodo);
		free(bloque);
	}
	return menor;
}

char * dameLinea(char * bloque, long unsigned fCur) {
	FILE * fA = fopen(bloque,"r");
	printf("ARCHIVO:%s\n",bloque);
	if(fA!=NULL){
		printf("fCUR:%lu\n",fCur);
		char s2[1] = "\n";
		char * ptr,*ptrPuto=string_new();
		long unsigned total;
		//char * array[2];
		fseek(fA,0L,SEEK_END);
		total = ftell(fA);
		printf("TOTAL:%lu\n",total);
		fseek(fA,fCur,SEEK_SET);
		char * buffer = malloc(500);
		memset(buffer,0,500);
		fread(buffer,1,500,fA);
		//ptr = strtok( buffer, s2 );    // Primera llamada => Primer token
		//int i=0;
		//if(ptr==NULL) return NULL;
		//return ptr;
		//string_append(&ptrPuto,ptr);
		//i++;
		//while( (ptr = strtok( NULL, s2 )) != NULL ){
		//	i++;
		//	if(i==10){
		//		break;
		//	}
		//}
		int i=0;
		char * aux = malloc(500);
		memset(aux,0,500);
		while(buffer[i++]!='\n'){
			aux[i-1]=buffer[i-1];
		}
		printf("AUXILIAR:%s\n",aux);
		//printf("BUFFER:%s\n",buffer);
		//char** array = string_split(buffer,s2);
		//printf("SPLIT:%s\n",array[0]);
		free(buffer);
		fclose(fA);
		return aux;
	} else {
		printf("HOLA3\n");
		return NULL;
	}

}

char* obtenerLinea(int socket,t_bloque_script *bloque){
	char * bufferE = string_new();
	char * bufferR = string_new();
	char * buffer = string_new();
	int cantRecibida=0,tamanio=10,cantRafaga=1;


	//segunda rafaga
	string_append(&buffer,"2");
	string_append(&buffer,obtenerSubBuffer(bloque->bloque));
	if(bloque->fsd == 0){
		string_append(&buffer,"110");
	} else {
		string_append(&buffer,obtenerSubBuffer(string_itoa(bloque->fsd)));
	}
	//string_append(&buffer,string_itoa(bloque->fsd));

	//primer rafaga
	string_append(&bufferE,"3");
	string_append(&bufferE,string_itoa(cuentaDigitos(strlen(buffer))));
	string_append(&bufferE,string_itoa(strlen(buffer)));

	//sem_wait(&semCon);
	EnviarDatos(socket,bufferE,strlen(bufferE));

	tamanio = strlen("*");
	cantRafaga=1;
	bufferR = RecibirDatos(socket,bufferR,&cantRecibida,&cantRafaga,&tamanio);
	int p,bandera=0;
	for(p=0;p<strlen(bufferR);p++){
		if(bufferR[p]=='*'){
			bandera=1;
		}
	}

	if(bandera == 0){
		printf("NO ASTERISCO!!!\n)");
		return 0;
	}
	EnviarDatos(socket,buffer,strlen(buffer));
	//sem_post(&semCon);
	bufferR = string_new();
	cantRafaga = 2;
	bufferR = RecibirDatos(socket,bufferR,&cantRecibida,&cantRafaga,&tamanio);



	bufferR=string_new();
	tamanio=200;
	buffer = string_new();
	string_append(&buffer,"*");
	EnviarDatos(socket,buffer,strlen(buffer));
	cantRafaga = 2;
	bufferR = RecibirDatos(socket,bufferR,&cantRecibida,&cantRafaga,&tamanio);
	return bufferR;
}

char * elMaravilloso(t_list* nodos,t_list**bloques){
	int i;
	t_bloque_script* bloque;
	t_nodo* nodo;
	int *vector = malloc (sizeof(int)*list_size(*bloques));
	char *menor = string_new();

	for(i=0;i<list_size(*bloques);i++){
		bloque = list_get(*bloques,i);
		if(bloque->nuevaLinea==1){
			if(!bloque->pertenece){
				printf("NO PERTENECE\n");
				nodo = buscarNodoPorNombre(bloque->nombreNodo,nodos);
				bloque->contenidoBloque = obtenerLinea(nodo->socket,bloque);
			} else {
				printf("PERTENECE\n");
				bloque->contenidoBloque = dameLinea(bloque->bloque,bloque->fsd);
			}
			bloque->nuevaLinea = 0;
			printf("%d) LA LINEA:%s\n",i,bloque->contenidoBloque);
		}

	}
	menor=dameMenor(nodos,bloques);

	free(vector);
	return menor;
}

void script_Reduce_Sin_Combiner(t_list**bloques,t_list* nodos,char*nombreScript,char* nombreArchivoFinal){
	long unsigned cB = cantidadDeLineas(bloques,nodos);
	printf("CB:%lu\n",cB);
	int cont=0,status;
	char * buffer = string_new();
	//t_bloque_script * bloque_script;

	char **array = string_split(nombreArchivoFinal,"/");
	while(array[cont]!=NULL){
		  cont++;
	}

	int vector[list_size(*bloques)];


	pid_t pid;
	int p[2];

	pipe( p );

	if ( (pid=fork()) == 0 )
	{

		close( p[1] );


		dup2(p[0],STDIN_FILENO);
		close(STDOUT_FILENO);
		FILE *fd = fopen(array[cont-1], "w" );
		close( p[0] );
		execl(nombreScript,nombreScript,NULL);

	}
	else
	{
	int j;
		close( p[0] );
		for(j=0;j<cB;j++){
			buffer = elMaravilloso(nodos,bloques);
			printf("LINEA:%s\n",buffer);
			write( p[1], buffer, strlen( buffer ) );
			//sleep(1);
		}
		close( p[1] );
		wait(&status);
  }

}


int AtiendeJobCombiner (t_jobComb ** job,char *buffer, int *cantRafaga){
	/*231215NodoB2202144resultado.txt2143resultado.txt2142resultado.txt2141resultado.txt2140resultado.txt21519resultado.txt21518resultado.txt21517resultado.txt21516resultado.txt21515resultado.txt21514resultado.txt21513resultado.txt21512resultado.txt21511resultado.txt21510resultado.txt2149resultado.txt2148resultado.txt2147resultado.txt2146resultado.txt2145resultado.txt15NodoB212192.168.0.1414600015NodoC1621525resultado.txt21524resultado.txt21523resultado.txt21522resultado.txt21521resultado.txt21520resultado.txt15NodoC212192.168.0.13146001230/user/juan/datos/resultado.txt3532#!/usr/bin/perl*/

	//semaforo
	estado = 1;
	t_list * nodos = list_create();
	t_list * bloques = list_create();
	t_nodo* el_nodo=malloc(sizeof(t_nodo));
	t_bloque_script * bloque_script;
	int cantNodos,digCantNodos,cantArchivos,cantDigArchivos;
	char *nombreResultado;
	int i,j,posActual=0;
	printf(COLOR_VERDE"BUFFER:%s\n"DEFAULT,buffer);
	digCantNodos=PosicionDeBufferAInt(buffer,2);

	cantNodos=ObtenerTamanio(buffer,3,digCantNodos);
	printf("Cantidad Nodos:%d\n",cantNodos);
	posActual=3+digCantNodos;

	for(i=0;i<cantNodos;i++){

		el_nodo->nombreNodo = DigitosNombreArchivo(buffer,&posActual);
		printf("Nombre Nodo:%s\n",el_nodo->nombreNodo);
		cantDigArchivos=PosicionDeBufferAInt(buffer,posActual);

		posActual= posActual + 1;

		cantArchivos=ObtenerTamanio(buffer,posActual,cantDigArchivos);
		printf("Cantidad Archivos:%d\n",cantArchivos);
		posActual = posActual + cantDigArchivos;
		el_nodo->listaArchivos = list_create();
		for(j=0;j<cantArchivos;j++){
			bloque_script = malloc(sizeof(t_bloque_script));
			nombreResultado=string_new();
			nombreResultado=DigitosNombreArchivo(buffer,&posActual);
			bloque_script->bloque = strdup(nombreResultado);
			bloque_script->nombreNodo = strdup(el_nodo->nombreNodo);
			bloque_script->fsd = 0;
			bloque_script->nuevaLinea = 1;
			list_add(el_nodo->listaArchivos,nombreResultado);
			list_add(bloques,bloque_script);
		}
		el_nodo->nombreNodo = DigitosNombreArchivo(buffer,&posActual);
		el_nodo->ipNodo=DigitosNombreArchivo(buffer,&posActual);
		el_nodo->puertoNodo=DigitosNombreArchivo(buffer,&posActual);
		int valor=0;
		if(!strcmp(g_Ip_Nodo,el_nodo->ipNodo)){
			if(!strcmp(string_itoa(g_Puerto_Nodo),el_nodo->puertoNodo)){
				valor = 1;
			}
		}
		int p;
		for(p=0;p<list_size(bloques);p++){
			bloque_script = list_get(bloques,p);
			bool _true(void *elem){
				return ((!strcmp((char*)elem,bloque_script->bloque)));
			}
			if(list_any_satisfy(el_nodo->listaArchivos,_true)){
				bloque_script->pertenece = valor;
			}
		}
		if(bloque_script->pertenece==0){
			if(!conectarFS(&el_nodo->socket,el_nodo->ipNodo,el_nodo->puertoNodo)){
				return 0;
			}
		}
		list_add(nodos,el_nodo);
	}

	char* nombreArchivoFinal=DigitosNombreArchivo(buffer,&posActual);

	char* contenidoScript=DigitosNombreArchivo(buffer,&posActual);

	char* nombreScript=DigitosNombreArchivo(buffer,&posActual);

	grabarScript(nombreScript,contenidoScript);

	permisosScript(nombreScript);
	//Doy permisos de ejecucion al script
	printf(COLOR_VERDE"CANTIDAD DE BLOQUES:%d\n"DEFAULT,list_size(bloques));
	script_Reduce_Sin_Combiner(&bloques,nodos,nombreScript,nombreArchivoFinal);
	printf(COLOR_VERDE"Termino el Reduce\n"DEFAULT);
	return 1;
}

void ordenarConSort(char * nombreArchivo,char* nombrePosta){

	char * comandoScript = string_new();
	string_append(&comandoScript, "sort");
	string_append(&comandoScript, " < ");
	//string_append(&comandoScript, "/tmp/"); Agregar Ruta Temporal
	string_append(&comandoScript, nombreArchivo);
	printf("NOMBRE DE ARCHIVO CREADO:%s\n",nombreArchivo);
	printf("NOMBRE DE ARCHIVO CREADO:%s\n",nombrePosta);
	string_append(&comandoScript, " > ");
	string_append(&comandoScript, nombrePosta);
	system(comandoScript);
	free(comandoScript);
	comandoScript = string_new();
	string_append(&comandoScript,"rm ");
	string_append(&comandoScript,nombreArchivo);
	system(comandoScript);
	free(comandoScript);
}

int procesarRutinaMap(t_job * job){
	sem_wait(&semaforoGrabar);
	grabarScript(job->nombreSH,job->contenidoSH);
	sem_post(&semaforoGrabar);
	//Creo el script y grabo el contenido.
	char ** array;

	//int numBloque = CharAToInt(job->bloque);
	array = string_split(job->bloque,"e");

	int nroBloque = atoi(array[1]);

	//printf("EL NUMERO DE BLOQUE A MAPEAR: %d \n",numBloque);
	char* contenidoBloque = malloc(TAMANIO_BLOQUE);
	contenidoBloque= getBloque(nroBloque);
	printf(COLOR_VERDE"NUMEROBLOQUE:%d\n"DEFAULT,nroBloque);
	//Obtengo el contendio del numero de bloque solicitado.
	//printf("%s",contenidoBloque);
	printf("\n\n%d\n",strlen(contenidoBloque));

	permisosScript(job->nombreSH);
	//Doy permisos de ejecucion al script

	//Ejecuto el script sobre el bloque
	//printf(COLOR_VERDE"AHORA SI\n"DEFAULT);
	char * elnombre = string_new();
	//string_append(&elnombre,"tmp");
	string_append(&elnombre,job->nombreResultado);
	int valor = runScriptFile(job->nombreSH,elnombre,contenidoBloque);

	//ordenarConSort(elnombre,job->nombreResultado);

	if (valor){
		//Si la ejecucion es correta devuelvo 1 y libero el bloque.
		if (contenidoBloque != NULL){
			//free(contenidoBloque); //rompe si dejo el free.
		}
		return 1;
	} else {
		//Si algo fallo devuelvo 0
		return 0;
	}

}

int procesarRutinaReduce(t_jobComb * job, int combiner){
	grabarScript(job->nombreSH,job->contenidoSH);
	//Creo el script y grabo el contenido.

	permisosScript(job->nombreSH);
	//Doy permisos de ejecucion al script
	char* contenidoArchivo;

	//en base al valor combiner armo el contenido de archivo de una manera u otra
	if(combiner){
	contenidoArchivo = armarArchivoCombiner(job->listaArchivos);
				}else{
					contenidoArchivo = armarArchivoSinCombiner(job->listaArchivos);

				}
	//apareo los archivos y devuelvo el contenido

	if (contenidoArchivo == NULL){
		//Error al traer el contenido del archivo
		return 0;
	}

	//Ejecuto el script sobre el contenido del archivo temporal
	if (runScriptFile(job->nombreSH,job->nombreResultado,contenidoArchivo)){
		//Si la ejecucion es correta devuelvo 1 y libero el bloque.
		if (contenidoArchivo != NULL){
			//free(contenidoArchivo); //rompe si dejo el free.
		}
		return 1;
	} else {
		//Si algo fallo devuelvo 0
		return 0;
	}

}

char* armarArchivoCombiner(t_list* listaArchivos){
	t_NodoArch* elemLista;
	t_NodoArch* elemLista2;
	char* nomArch, *resultado;
	char* archSalida="salida.txt";
	FILE * salida;
	int primerApareo =1;
	//si la lista tiene un solo elemento, no hace falta apareo, devuelvo el unico archivo
	if(list_size(listaArchivos)==1){
		elemLista = list_remove(listaArchivos, 0);
		nomArch=elemLista->nomArchT;
		return (getFileContent(nomArch));
	}
	while(!(list_is_empty(listaArchivos))){
		if(primerApareo){
			//al ser el primer apareo necesito agarrar dos elementos de la lista
			elemLista = list_remove(listaArchivos, 0);
			elemLista2 = list_remove(listaArchivos, 0);
			//apareo los dos archivos
			resultado = apareoArchivos(elemLista->nomArchT,elemLista2->nomArchT);
			//grabo el resultado en un archivo salida, que voy a usar para seguir apareando
			salida = fopen(archSalida,"w");
			fwrite(resultado, sizeof(char),strlen(resultado),salida);
			fclose(salida);
			//cambio la variable primer apareo a cero
			primerApareo =0;
		} else{
		//agarro el primer elemento
		elemLista = list_remove(listaArchivos, 0);
		//apareo con la salida del primer apareo y devuelvo char*
		resultado =apareoArchivos(elemLista->nomArchT,archSalida);
		//grabo el archivo
		salida = fopen(archSalida,"w");
		fwrite(resultado, sizeof(char),strlen(resultado),salida);
		fclose(salida);
		}
	}
	resultado= getFileContent(archSalida);
	//paso a resultado el resultado de apareo final
	remove(archSalida);
	//elimino el archivo de salida
	return resultado;
}


char* armarArchivoSinCombiner(t_list* listaArchivos){
	//Falta implementar
	return "OK";
}

char* apareoArchivos(char* archivo1, char* archivo2){
	//Aparea dos archivos y el resultado lo guarda en un tercer archivo de salida.
	char* buffer1 = malloc(BUFFERLINEA);
	char* buffer2 = malloc(BUFFERLINEA);
	FILE * arch1 = fopen(archivo1,"r+");
	FILE * arch2 = fopen(archivo2,"r");
	char* salida = string_new();

	int count1 = fread(buffer1, sizeof(char),BUFFERLINEA,arch1);
	int count2 = fread(buffer2, sizeof(char),BUFFERLINEA,arch2);
	char* codigo1 = obtenerCodigo(buffer1);
	char* codigo2 = obtenerCodigo(buffer2);

	while(count1 != 0 && count2 != 0){
		while((strcmp(codigo1,codigo2)!=0) && (count1 != 0 && count2 != 0)){

			if(esMayor(codigo1,codigo2)){
				//fwrite(buffer1, sizeof(char),BUFFERLINEA,salida);
				string_append(&salida,buffer1);
				count1 = fread(buffer1, sizeof(char),BUFFERLINEA,arch1);
				codigo1 = obtenerCodigo(buffer1);
			}else{
							//fwrite(buffer2, sizeof(char),BUFFERLINEA,salida);
				string_append(&salida,buffer2);
				count2 =fread(buffer2, sizeof(char),BUFFERLINEA,arch2);
							codigo2 = obtenerCodigo(buffer2);
			}
		}
		//fwrite(buffer1, sizeof(char),BUFFERLINEA,salida);
		//fwrite(buffer2, sizeof(char),BUFFERLINEA,salida);
		if(count1!=0)
			string_append(&salida,buffer1);
		if(count2!=0)
			string_append(&salida,buffer2);

		count1 =fread(buffer1, sizeof(char),BUFFERLINEA,arch1);
		codigo1 = obtenerCodigo(buffer1);
		count2 =fread(buffer2, sizeof(char),BUFFERLINEA,arch2);
		codigo2 = obtenerCodigo(buffer2);


	}
	while(count1!=0){
		//fwrite(buffer1, sizeof(char),BUFFERLINEA,salida);
		string_append(&salida,buffer1);
		count1 =fread(buffer1, sizeof(char),BUFFERLINEA,arch1);
	}
	while(count2!=0){
			//fwrite(buffer2, sizeof(char),BUFFERLINEA,salida);
		string_append(&salida,buffer2);
		count2 =fread(buffer2, sizeof(char),BUFFERLINEA,arch2);
		}
			fclose(arch1);
			fclose(arch2);
			//fclose(salida);


			return salida;
}

char *obtenerCodigo(char* buffer){
	//Obtiene el codigo de la linea, que es hasta donde aparece un ';'
	char * codigo = string_new();
	int size = 0;
	char* corte = ";";
	while(buffer[size] != corte[0]){
		string_append_with_format(&codigo, "%c", buffer[size]);
		size++;
	}
	return codigo;
}

int esMayor(char* primero, char* segundo){
	//Compara si el primer string es mayor que el segundo y devuelve 1 si es mayor.
	int size = 0;
	while(size < strlen(primero)){
		if(primero[size]>segundo[size]){
			return 0;
		}
		size++;
	}

	return 1;
}


void atiendeNodo(int socket, char * buffer,int * cantRafaga,char ** mensaje,int* tamanio){

	int posActual,bytesRecibidos;
	long unsigned tamanioArchivo;
	long unsigned cantidadLineas;
	char* nombreResultado = string_new();
	char * bufferE = string_new();
	char * bufferR = string_new();

	EnviarDatos(socket,"*",strlen("*"));


	*cantRafaga = 2;
	*tamanio=100;
	printf("TAMAÑO:%d\n",*tamanio);
	bufferR = RecibirDatos(socket, bufferR, &bytesRecibidos,cantRafaga,tamanio);

	int tipo_mensaje = ObtenerComandoMSJ(bufferR);

	switch(tipo_mensaje){
		case CANTIDADLINEAS:
			//31210RESULTADO.TXT
			bufferE=string_new();
			posActual = 1;
			nombreResultado = DigitosNombreArchivo(bufferR,&posActual);
			printf("ARCHIVO DE RESULTAD:%s\n",nombreResultado);
			cantidadLineas = cantidadLineasArchivo(nombreResultado,&tamanioArchivo);
			printf("Cantidad Lineas:%lu\n",cantidadLineas);
			string_append(&bufferE,obtenerSubBuffer(string_itoa(cantidadLineas)));
			string_append(&bufferE,obtenerSubBuffer(string_itoa(tamanioArchivo)));
			printf("BUFFER ENVIADO:%s\n",bufferE);
			EnviarDatos(socket,bufferE,strlen(bufferE));
			free(bufferE);
			free(bufferR);
			free(nombreResultado);
			*cantRafaga = 1;
			*mensaje = "Ok";
			break;
		case DAMELINEA:
			//32210RESULTADO.TXT210600000
			posActual = 1;
			nombreResultado = DigitosNombreArchivo(bufferR,&posActual);
			long unsigned fCur;
			fCur = atoi(DigitosNombreArchivo(bufferR,&posActual));
			char * linea = dameLinea(nombreResultado,fCur);
			//primera rafaga
			string_append(&bufferE,string_itoa(strlen(linea)));
			EnviarDatos(socket,bufferE,strlen(bufferE));
			bufferR = string_new();
			int cantRecibida;
			*cantRafaga=1;
			bufferR = RecibirDatos(socket,bufferR,&cantRecibida,cantRafaga,tamanio);

			int p,bandera=0;
			for(p=0;p<strlen(bufferR);p++){
				if(bufferR[p]=='*'){
					bandera=1;
				}
			}

			if(bandera == 0){
				printf("NO ASTERISCO!!!\n)");
			}

			buffer = string_new();
			string_append(&buffer,linea);
			EnviarDatos(socket,buffer,strlen(buffer));
			free(buffer);
			free(bufferE);
			free(bufferR);
			*mensaje = "Ok";
			/*if(linea!=NULL)
				free(linea);*/
			free(nombreResultado);
			*cantRafaga = 1;
			break;
		default:
			break;
		}

	printf("Salio del Atiende Nodo\n");
}



void implementoJob(int *id,char * buffer,int * cantRafaga,char ** mensaje,int socket,int *tamanio){
	t_job * job;
	t_jobComb * jobR;
	char * bufferR;
	bufferR = string_new();
	int bytesRecibidos;
	if(*cantRafaga == 1){
		EnviarDatos(socket, "Ok",2);
	}
	*cantRafaga = 2;
	bufferR = RecibirDatos(socket, bufferR, &bytesRecibidos,cantRafaga,tamanio);
	int tipo_mensaje = ObtenerComandoMSJ(bufferR+1);

	switch(tipo_mensaje){
		case MAPPING:
			AtiendeJob(&job,bufferR,cantRafaga);
			printf("Nombre de SH:%s\n",job->nombreSH);
			printf("Contenido de SH:%s\n",job->contenidoSH);
			printf("Bloque:%s\n",job->bloque);
			printf("Nombre de Resultado:%s\n",job->nombreResultado);


			if(procesarRutinaMap(job)){ //Proceso la rutina de map.
				//Pudo hacerla
				*mensaje="31";
			} else {
				//No pudo hacerla
				*mensaje="30";
			}
			break;

		case REDUCE_COMBINER:
			break;

		case REDUCE_SIN_COMBINER:
				if(AtiendeJobCombiner(&jobR,bufferR,cantRafaga)){
					//Pudo hacerla
					*mensaje = "31";
				} else {
					//No pudo hacerla
					*mensaje = "30";
				}
				break;
		default:
			break;
	}
	*cantRafaga = 1;
}


int obtenerNumBloque (char* buffer){
	//Buffer reci 12 210
	//semaforo
		estado = 1;
		int numeroBloque,digitosCantDeDigitosNumBloq;

		digitosCantDeDigitosNumBloq=PosicionDeBufferAInt(buffer,2);
		//printf("Cantidad Digitos de tamaño de contenido SH:%d\n",digitosCantDeDigitosSH);
		numeroBloque=ObtenerTamanio(buffer,3,digitosCantDeDigitosNumBloq);
		//printf("Tamanio del SH: %d\n",tamanioSH);

		//printf("bloque numero: %d\n",numeroBloque);

	return numeroBloque;
}


void implementoFS(char * buffer,int *cantRafaga,char** mensaje,int socket){
	//t_bloque* bloqueSet;
	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
		//printf("RAFAGA:%d\n",tipo_mensaje);
		//printf("LA RAFAGA:%d\n",*cantRafaga);
		if(*cantRafaga == 2){
			switch(tipo_mensaje){
						//GET_BLOQUE
						//BUFFER RECIBIDO = 12210 1: soy FS 2: quiero un get bloque 3:cant de dig de numero de bloque
						//						  10: numero de bloque que necesito recibir
						//BUFFER ENVIADO = 32820971520 3: soy nodo 2: va un get bloque 8: cant de dig de tamanio de
						//								bloque solicitado 20971520: tamanio de bloque
			case GET_BLOQUE:
				*cantRafaga=1;
				char* contenidoBloque, * bloqueMsj;
				int numBloq = obtenerNumBloque(buffer);
				contenidoBloque = getBloque(numBloq);
				bloqueMsj = obtenerSubBuffer(contenidoBloque);
				if(contenidoBloque !=NULL){
					//Obtuvo el bloque
					*mensaje="32";
					string_append(mensaje,bloqueMsj);
				}else{
					*mensaje="320"; //Ver como le aviso que fallo?
				}
				break;
			case SOLICITUD_DE_CONEXION:
				if(conectado == 0){
					*mensaje = "Ok!";
					conectado = 1;
				}
				break;

				//SET_BLOQUE
				//BUFFER RECIBIDO = 13210820971520  --- 1: soy FS 3: quiero un set bloque 2:cant digitos numero bloque 10:numero de bloque a grabar 8: cant de dig de tamanio
				//								    de lo que necesitamos que grabe 20971520: tamaño en bytes
				//BUFFER ENVIADO = Ok
			case SET_BLOQUE:
				printf("ES SET BLOQUE\n");



				*cantRafaga=1;
				break;
			case GET_FILE_CONTENT:
							*cantRafaga=1;
							break;
			default:
				break;
			}
			*mensaje = "Ok";
		} else {
			if (*cantRafaga==1) {
				*mensaje = "Ok!!";
				*cantRafaga = 2;
			} else {
				*mensaje = "No";
			}
		}

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
		printf("Cantidad Enviada :%lu\n",n);
	}
	*len = total; // devuelve aquí la cantidad enviada en realidad
	return n==-1?-1:0;	// devuelve -1 si hay fallo, 0 en otro caso
}


int procesarGetBloqueDeFs(char* buffer,char**mensaje,int socket){
	char * bloque;
	int nroBloque, cantDigBloque,bytesRecibidos,cantRafaga=1,tamanio,cantDigitos;
	char * bufferR = string_new();
	char * bufferE = string_new();

	cantDigBloque=PosicionDeBufferAInt(buffer,2);
	nroBloque=ObtenerTamanio(buffer,3,cantDigBloque);

	bloque = getBloque(nroBloque);

	if(bloque!=NULL){
		string_append(&bufferE,"3");
		cantDigitos = cuentaDigitos(strlen(bloque));
		string_append(&bufferE,string_itoa(cantDigitos));
		string_append(&bufferE,string_itoa(strlen(bloque)));
		EnviarDatos(socket,bufferE, strlen(bufferE));
		bufferR=RecibirDatos(socket,bufferR, &bytesRecibidos,&cantRafaga,&tamanio);
		if(strcmp(bufferR,"1")==0){
			long unsigned len=0;
			len=strlen(bloque);
			printf("LEN : %lu\n",len);
			if (sendall(socket, bloque, &len) == -1) {
				printf("ERROR AL ENVIAR\n");
			}
			//free(bloque);
			close(socket);
			*mensaje="Ok";
			return 1;
		}
	}
	//free(bloque);
	close(socket);
	*mensaje="No";
	return 0;
}


int procesarSetBloqueDeFs(char* buffer,char**mensaje,int socket){
	t_bloque* bloqueSet;

	int digTamanio=0,tamanio=0;

	digTamanio=PosicionDeBufferAInt(buffer,2);

	tamanio=ObtenerTamanio(buffer,3,digTamanio);

	//printf("TAMAÑO:%d\n",strlen(buffer));
	//printf("BUFFER:%s\n",buffer);

	*mensaje = string_new();

	string_append(mensaje,"1");

	EnviarDatos(socket, *mensaje,strlen(*mensaje));

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
	tamanioTotal = tamanioTotal + strlen(bloque);
	//printf(COLOR_VERDE"---%d---\n"DEFAULT,strlen(bloque));
	AtiendeFS(&bloqueSet,bloque);
	printf(COLOR_VERDE"---%d---\n"DEFAULT,strlen(bloqueSet->contenidoBloque));
	setBloque(bloqueSet->numeroBloque,bloqueSet->contenidoBloque);
	free(bloqueSet->contenidoBloque);
	//string_append(mensaje,"Listo");
	/*if(1){ //Hacer que setBloque devuelva algo para saber si fallo
		*mensaje = string_new();
		string_append(mensaje,"Ok"); //Se grabo correctamente
	}else{
		*mensaje = "Error!"; //Algo fallo
	}*/
//	printf("-----------\n");
	//free(bloqueSet->contenidoBloque);
	free(aux);
	//free(bloqueSet);
	free(bloque);
	return 1;

}

void RecorrerListaBloques(){
	int i;
	t_bloque* bloque = malloc(sizeof(t_bloque));
	for(i=0;i<list_size(lista_Bloques);i++){
		bloque = list_get(lista_Bloques,i);
		printf("NUMERO DE BLOQUE:%d\n",bloque->numeroBloque);
		printf("TAMAÑO DEL BLOQUE%lu\n",bloque->tamanioBloque);
	}
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;
	int id=-1;
	//cantHilos++;
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
	int trabajo;
	int cantRafaga=1,tamanio=0;
	char * mensaje=string_new();

	while ((!desconexionCliente) && g_Ejecutando) {
		//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

		printf("ENTRO AL WHILE\n");

		if (buffer != NULL )
			free(buffer);
		buffer=string_new();

		//Recibimos los datos del cliente
		buffer = RecibirDatos(socket, buffer, &bytesRecibidos,&cantRafaga,&tamanio);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			emisor = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (emisor) {
			case ES_JOB:
				implementoJob(&id,buffer,&cantRafaga,&mensaje,socket,&tamanio);
				cantRafaga=1;
				break;
			case ES_FS:
				//printf("implementar atiendeFS\n");
				//implementoFS(buffer,&cantRafaga,&mensaje,socket);
				trabajo=ObtenerComandoMSJ(buffer+1);
				if(trabajo==3){
					procesarSetBloqueDeFs(buffer,&mensaje,socket);
					cantRafaga=1;
					//desconexionCliente = 1;
					//printf(COLOR_VERDE"Afuera:%s"DEFAULT,mensaje);
				} else {
					if (trabajo==2){
						procesarGetBloqueDeFs(buffer,&mensaje,socket);
						cantRafaga=1;
						close(socket);
						return 1;
					}
				}
				//desconexionCliente=1;
				break;
			case ES_NODO:
				//printf("implementar atiendeNodo\n");

				atiendeNodo(socket,buffer,&cantRafaga,&mensaje,&tamanio);

				cantRafaga=1;
				printf(COLOR_VERDE"CERRADO EL SOCKET:%d\n"DEFAULT,socket);
				close(socket);
				return 1;
				break;
			case COMANDOBLOQUES:
				//printf("Despues vemos que hace esto\n");
				//RecorrerListaBloques();
				//printf("CANTIDAD LINEAS%lu\n",cantidadLineasArchivo("barran.txt"));
				//mensaje = "Ok";
				printf("EL CONTADOR:%d\n",contador);
				break;
			default:
				break;
			}
			printf("MENSAJE:%s\n",mensaje);
			longitudBuffer=strlen(mensaje);
			//printf("\nRespuesta: %s\n",buffer);
			// Enviamos datos al cliente.
			//if(!strcmp(mensaje,"Listo")){
			EnviarDatos(socket, mensaje,longitudBuffer);
			//}



		} else
			desconexionCliente = 1;

	}
	printf("SALIO DEL WHILE\n");

	printf(COLOR_VERDE"NA NA NA ESTO ESTA MAL\n"DEFAULT);
	printf("EL SOCKET PUTO:%d\n",socket);
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
	my_addr.sin_port = htons(g_Puerto_Nodo);
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
			socket_host, g_Puerto_Nodo);

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
