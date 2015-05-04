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

	logger = log_create(NOMBRE_ARCHIVO_LOG, "msp", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	//CreoSocket();
	printf("Ok\n");

	return 0;
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
			g_Puerto_Marta = config_get_int_value(config,"PUERTO_MARTA");
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

#if 1 // MÉTODO MANEJO DE SOCKETS
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
