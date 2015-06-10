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

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	//Abro el archivo de espacio de datos donde voy a leer y escribir bloques
	if(( archivoEspacioDatos = fopen(g_Archivo_Bin, "r+b") ) == NULL){
		//Si no se pudo abrir, imprimir el error y abortar;
		fprintf(stderr, "Error al abrir el archivo '%s': %s\n", g_Archivo_Bin, strerror(errno));
		abort();
			}

	fseek(archivoEspacioDatos, 0L, SEEK_END);

	printf("Tamaño de Archivo de Datos:%lu\n",ftell(archivoEspacioDatos));
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
	while(tamDigArch>1){
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
	char* bloque;
	int fd= fileno(archivoEspacioDatos);
	int offset = numero*(TAMANIO_BLOQUE/pagina);
	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ, MAP_SHARED, fd, offset*pagina)) == MAP_FAILED){
			//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
			fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
			abort();
		}
	printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);

	return bloque;
}

void setBloque(int numero, char*datos){
	if(( tamanio_archivo(g_Archivo_Bin) <= (numero*TAMANIO_BLOQUE) )){
			//Si no se pudo abrir, imprimir el error y abortar;
			printf("El bloque no existe en el archivo. \n");
			abort();
		}
	char* bloque;
	int fd= fileno(archivoEspacioDatos);
	int offset = numero*(TAMANIO_BLOQUE/pagina);
	if( (bloque = mmap( NULL, TAMANIO_BLOQUE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset*pagina)) == MAP_FAILED){
				//Si no se pudo ejecutar el MMAP, imprimir el error y abortar;
				fprintf(stderr, "Error al ejecutar MMAP del archivo '%s' de tamaño: %d: %s\n", g_Archivo_Bin, TAMANIO_BLOQUE, strerror(errno));
				abort();
	}
	memcpy(bloque, datos, strlen(datos)+1);
 //Copia los datos a grabar en el bloque auxiliar
	printf ("Bloque Nro: %d\nContenido:'%s'\n", numero, bloque);


}

/* char* getBloque(int numero){
	char* bloque = malloc(TAMANIO_BLOQUE);
	long int offset=numero*TAMANIO_BLOQUE;
	fseek(archivoEspacioDatos, offset, SEEK_SET);
	fread(bloque, sizeof(char), TAMANIO_BLOQUE, archivoEspacioDatos);
	return bloque;
}*/


/* void setBloque(int numero, char*datos){
	if(( tamanio_archivo(g_Archivo_Bin) <= (numero*TAMANIO_BLOQUE) )){
			//Si no se pudo abrir, imprimir el error y abortar;
			printf("El bloque no existe en el archivo. \n");
			abort();
		}
	long int tamanio = TAMANIO_BLOQUE; //Tamanio del bloque 20mb
	char*txtBloq = malloc(TAMANIO_BLOQUE);
	memset(txtBloq, '0', tamanio * sizeof(char));
	//Rellena de 0 el txtBloq que se va a grabar
	memcpy(txtBloq, datos, strlen(datos)+1);
	//Copia los datos a grabar en el bloque auxiliar
	long int offset=numero*TAMANIO_BLOQUE;

	fseek(archivoEspacioDatos, offset, SEEK_SET);
	//Posicion el puntero en el bloque pedido
	fwrite(txtBloq, sizeof(char), tamanio, archivoEspacioDatos);
	//Grabo en el archivo el bloque
	free(txtBloq);
	//Libero el puntero


} */

char * armarRutaTemporal( char *nombre){
		//static char ruta[] = g_Dir_Temp;
		char* nombreT = string_new();
		//string_append(&nombreT,ruta);
		string_append(&nombreT,g_Dir_Temp);
		string_append(&nombreT,"/");
		string_append(&nombreT,nombre);
		char fname[PATH_MAX];
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
}
*/

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

int runScriptFile(char* script,char* archNom, char* input)
{
    int outfd[2];
    int infd[2];

    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);

    if(!fork()) {


        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);

        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);
        execl(script, script, (char*) 0);
    } else {
        char* buffer= malloc(50); //definir el tamanio de este buffer
        int count;

        /* close fds not required by parent */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        //printf("lo que se va a escribir es %s y la longitud es %d\n", input, strlen(input));
        // Write to child’s stdin

        if (write(PARENT_WRITE_FD, input, (strlen(input)+1)) == -1){
        	return 0;
        }

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


void AtiendeJob (t_job ** job,char *buffer, int *cantRafaga){
	//Cadena recibida del Job
	//21270xxxxxxxxx212barriendo.sh233213resultado.txt
	//2: soy Job 1: recibo tarea 2: cant de dig del tamaño del .sh 70: tamaño del sh
	//2: cant de dig de tamaño del nombre del .sh 12: tamaño del nombre del .sh y luego el nombre
	//2: cant de dig del numero de bloque 33: numero de bloque, el nombre del arch de resultado lo sacas jajaja

	//semaforo
	estado = 1;
	char *nArchivoSH;
	int digitosCantDeDigitos=0,tamanioSH,digitosCantDeDigitosSH;
	char * el_Bloque;
	int posActual=0;
	char * fileSH,*nArchivoResultado;

	digitosCantDeDigitosSH=PosicionDeBufferAInt(buffer,2);
	//printf("Cantidad Digitos de tamaño de contenido SH:%d\n",digitosCantDeDigitosSH);
	tamanioSH=ObtenerTamanio(buffer,3,digitosCantDeDigitosSH);
	//printf("Tamanio del SH: %d\n",tamanioSH);
	posActual=2+digitosCantDeDigitos;

	fileSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Contenido de archivo SH:%s\n",fileSH);

	nArchivoSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo SH:%s\n",nArchivoSH);

	//printf("Posicion Actual:%d\n",posActual);
	//printf("Cantidad de digitos del numero de bloque:%d\n",digitosTamanioBloque);
	el_Bloque=DigitosNombreArchivo(buffer,&posActual);
	printf("bloque: %s\n",el_Bloque);

	nArchivoResultado=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo de Resultado:%s\n",nArchivoResultado);

	*job = job_create(nArchivoSH,fileSH,el_Bloque,nArchivoResultado);
	*cantRafaga=1;
}

void AtiendeJobSinCombiner (t_job ** job,char *buffer, int *cantRafaga){
	//Cadena recibida del Job
	// A NODO => 2312 15NodoA  13 215resultado00.txt     215resultado01.txt    1215resultado02.txt     15NodoA212192.168.1.27146000
	//         15NodoC  11 215resultado03.txt     15NodoC212192.168.1.27146000
	//semaforo
	estado = 1;
	char *nArchivoSH;
	int digitosCantDeDigitos=0,tamanioSH,digitosCantDeDigitosSH;
	char * el_Bloque;
	int posActual=0;
	char * fileSH,*nArchivoResultado;

	digitosCantDeDigitosSH=PosicionDeBufferAInt(buffer,2);
	//printf("Cantidad Digitos de tamaño de contenido SH:%d\n",digitosCantDeDigitosSH);
	tamanioSH=ObtenerTamanio(buffer,3,digitosCantDeDigitosSH);
	//printf("Tamanio del SH: %d\n",tamanioSH);
	posActual=2+digitosCantDeDigitos;

	fileSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Contenido de archivo SH:%s\n",fileSH);

	nArchivoSH=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo SH:%s\n",nArchivoSH);

	//printf("Posicion Actual:%d\n",posActual);
	//printf("Cantidad de digitos del numero de bloque:%d\n",digitosTamanioBloque);
	el_Bloque=DigitosNombreArchivo(buffer,&posActual);
	printf("bloque: %s\n",el_Bloque);

	nArchivoResultado=DigitosNombreArchivo(buffer,&posActual);
	//printf("Nombre Archivo de Resultado:%s\n",nArchivoResultado);

	*job = job_create(nArchivoSH,fileSH,el_Bloque,nArchivoResultado);
	*cantRafaga=1;
}

int procesarRutinaMap(t_job * job){
	grabarScript(job->nombreSH,job->contenidoSH);
	//Creo el script y grabo el contenido.

	int numBloque = ChartToInt(job->bloque);
	char* contenidoBloque = malloc(TAMANIO_BLOQUE);
	contenidoBloque= getBloque(numBloque);
	//Obtengo el contendio del numero de bloque solicitado.

	permisosScript(job->nombreSH);
	//Doy permisos de ejecucion al script

	//Ejecuto el script sobre el bloque
	if (runScriptFile(job->nombreSH,job->nombreResultado,contenidoBloque)){
		//Si la ejecucion es correta devuelvo 1 y libero el bloque.
		if (contenidoBloque != NULL){
			free(contenidoBloque);
		}
		return 1;
	} else {
		//Si algo fallo devuelvo 0
		return 0;
	}

}

int procesarRutinaReduceCombiner(t_job * job){
	grabarScript(job->nombreSH,job->contenidoSH);
	//Creo el script y grabo el contenido.

	permisosScript(job->nombreSH);
		//Doy permisos de ejecucion al script

	char* contenidoArchivo = getFileContent(job->bloque);
	if (contenidoArchivo == NULL){
		//Error al traer el contenido del archivo
		return 0;
	}
	//Obtengo el contendio del archivo temporal solicitado.

	//Ejecuto el script sobre el contenido del archivo temporal
	if (runScriptFile(job->nombreSH,job->nombreResultado,contenidoArchivo)){
		//Si la ejecucion es correta devuelvo 1 y libero el bloque.
		if (contenidoArchivo != NULL){
			free(contenidoArchivo);
		}
		return 1;
	} else {
		//Si algo fallo devuelvo 0
		return 0;
	}

}


int procesarRutinaReduceSinCombiner(t_jobComb * job){
	//Falta implementar
	return 1;
}

void implementoJob(int *id,char * buffer,int * cantRafaga,char ** mensaje){
	t_job * job;
	t_jobComb * jobC;
	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
	printf("RAFAGA:%d\n",tipo_mensaje);
	if(*cantRafaga == 2){
		switch(tipo_mensaje){
		case MAPPING:
			AtiendeJob(&job,buffer,cantRafaga);
			printf("Nombre de SH:%s\n",job->nombreSH);
			printf("Contenido de SH:%s\n",job->contenidoSH);
			printf("Bloque:%s\n",job->bloque);
			printf("Nombre de Resultado:%s\n",job->nombreResultado);
			if(procesarRutinaMap(job)){ //Proceso la rutina de map.
				//Pudo hacerla
				*mensaje = "31";
			} else {
				//No pudo hacerla
				*mensaje = "30";
			}
			break;
		case REDUCE_COMBINER:
					AtiendeJob(&job,buffer,cantRafaga);
					printf("Nombre de SH:%s\n",job->nombreSH);
					printf("Contenido de SH:%s\n",job->contenidoSH);
					printf("Archivo:%s\n",job->bloque);
					printf("Nombre de Resultado:%s\n",job->nombreResultado);
					if(procesarRutinaReduceCombiner(job)){ //Proceso la rutina de reduce con combiner.
						//Pudo hacerla
						*mensaje = "31";
					} else {
						//No pudo hacerla
						*mensaje = "30";
					}
					break;

		case REDUCE_SIN_COMBINER:
							AtiendeJobSinCombiner(&job,buffer,cantRafaga);
							printf("Nombre de SH:%s\n",job->nombreSH);
							printf("Contenido de SH:%s\n",job->contenidoSH);
							printf("Archivo:%s\n",job->bloque);
							printf("Nombre de Resultado:%s\n",job->nombreResultado);
							if(procesarRutinaReduceSinCombiner(jobC)){ //Proceso la rutina, reduce sin combiner.
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
	} else {
		if (*cantRafaga==1) {
			*mensaje = "Ok";
			*cantRafaga = 2;
		} else {
			*mensaje = "No";
		}
	}
}

void implementoFS(char * buffer,int *cantRafaga,char** mensaje){
	int tipo_mensaje = ObtenerComandoMSJ(buffer+1);
		printf("RAFAGA:%d\n",tipo_mensaje);
		if(*cantRafaga == 2){
			switch(tipo_mensaje){
			case GET_BLOQUE:
				*cantRafaga=1;
				break;
			case SOLICITUD_DE_CONEXION:
				if(conectado == 0){
					*mensaje = "Ok!";
					conectado = 1;
				}
				break;
			case SET_BLOQUE:
				printf("Proceso Bloque\n");
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
				implementoJob(&id,buffer,&cantRafaga,&mensaje);
				//BUFFER RECIBIDO = 23100 (EJEMPLO PRIMERA RAFAGA)
				//BUFFER ENVIADO = Ok
				//BUFFER RECIBIDO = 2
				//BUFFER ENVIADO = 311

				break;
			case ES_FS:
				printf("implementar atiendeFS\n");
				implementoFS(buffer,&cantRafaga,&mensaje);
				//Esto va en nodo.h y son define
				//PRIMERA_CONEXION 1
				//GET_BLOQUE 2
				//SET_BLOQUE 3
				//GET_FILE_CONTENT 4

				//BUFFER RECIBIDO = 11 --- 1:soy FS 1:estas disponible para conectarte?
				//BUFFER ENVIADO = 311250 --- 3: soy nodo 1: estoy vivo 1: soy nuevo 2: cant de dig de cant de
				// 							   de bloques 50: cantidad de bloques

				//SET_BLOQUE
				//BUFFER RECIBIDO = 13820971520  --- 1: soy FS 3: quiero un set bloque 8: cant de dig de tamanio
				//								    de lo que necesitamos que grabe 20971520: tamaño en bytes
				//BUFFER ENVIADO = Ok

				//GET_BLOQUE
				//BUFFER RECIBIDO = 12210 1: soy FS 2: quiero un get bloque 3:cant de dig de numero de bloque
				//						  10: numero de bloque que necesito recibir
				//BUFFER ENVIADO = 32820971520 3: soy nodo 2: va un get bloque 8: cant de dig de tamanio de
				//								bloque solicitado 20971520: tamanio de bloque

				//GET_FILE_CONTENT
				//BUFFER RECIBIDO = 14213resultado.txt 1: soy FS 4: solicito un get file content 2: cant de dig
				//					de tamanio de archivo solicitado 13:tamanio de archivo y luego el archivo
				//BUFFER ENVIADO = 34820971520 3: soy nodo 4: va un get file content solicitado 8: cant de dig
				//							   de tamanio de arch de result 20971520: tamanio de archivo

				break;
			case ES_NODO:
				printf("implementar atiendeNodo\n");
				//RecorrerArchivos();
				mensaje = "Ok";
				break;
			case COMANDOBLOQUES:
				printf("Despues vemos que hace esto\n");
				//RecorrerListaBloques();
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
