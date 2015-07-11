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
#define MAXJSON				100		//Largo de una cadena JSON
#define MAXCAMPO			20		//Largo para un campo
#define MAXVALOR			20		//Largo para un valor


typedef enum {
	SonDirectorios,
	SonArchivos,
} t_tipoAcceso;

typedef struct {
	char nombre[MAXNOMBREARCHIVO];
	int tamanio;
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

//Persistencia nodo archivos y filesystem
mongoc_collection_t *nodosMongo;
mongoc_collection_t *archivosMongo;
mongoc_collection_t *filesystemMongo;

int iniciarMongo();
//int eliminarMongo();
//int leerMongo();
t_archivo_json mongo_get_archivo();
void mongo_db_directorios_open();
void mongo_db_archivos_open();
void mongo_db_close();
void obtenerCampoValor(const char *, t_archivo_json *, t_directorio_json *, t_tipoAcceso);
void leerJSON(t_tipoAcceso);
t_archivo_json *leerDirectorioMongo();
t_archivo_json *leerArchivoMongo();
void modificarIndice();
void eliminarArchivoMongo();

#endif /* MONGODEV_H_ */
