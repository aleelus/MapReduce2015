/*
 * mongodev.h
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#ifndef MONGODEV_H_
#define MONGODEV_H_

#include <mongoc.h>

#define JSON_FILE	"/home/gabriel/workspace/sisop/tp-2015-1c-los-barderos/reducemapfast/filesystem/src/filesystem.json"
#define JSON_FILE2	"/home/gabriel/workspace/sisop/tp-2015-1c-los-barderos/reducemapfast/filesystem/src/directorios.json"
#define MAXNOMBREARCHIVO    20
#define MAXNOMBREDIRECTORIO 20


typedef enum {
	SonDirectorios,
	SonArchivos,
} t_tipoAcceso;

typedef struct {
	char nombre[MAXNOMBREARCHIVO];
	long unsigned tamanio;
	int directorio;
	int bloques;
	int estado;
} t_archivo_json;

typedef struct {
	int index;
	char directorio[MAXNOMBREDIRECTORIO];
	int directorio_padre;
} t_directorio_json;


mongoc_collection_t *collection;
mongoc_client_t *client;
t_tipoAcceso tipoAcceso;

int iniciarMongo();
int eliminarMongo();
int leerMongo();


#endif /* MONGODEV_H_ */
