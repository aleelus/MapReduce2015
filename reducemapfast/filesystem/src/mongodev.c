/*
 * mongodev.c
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */
#include "mongodev.h"
#include <string.h>




int iniciarMongo(){

	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;
	bson_t *query;
	mongoc_cursor_t *cursor;

	mongo_db_directorios_open();

	query = bson_new();
	BSON_APPEND_UTF8(query, "Index", "0");

	if((cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL)) == NULL){
		//No existe el primer indice, entonces la coleccion esta vacia;
		doc = bson_new ();
		bson_oid_init (&oid, NULL);
		BSON_APPEND_OID (doc, "_id", &oid);
		BSON_APPEND_INT32 (doc, "Index", 0);
		BSON_APPEND_UTF8 (doc, "Directorio", "raiz");
		BSON_APPEND_INT32 (doc, "Padre", 0);

		if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
			printf ("%s\n", error.message);
		}

		bson_destroy (doc);
		bson_destroy (query);

		//Probablemente, lo mejor seria que cargue la estructura de directorios de una..

	}

	mongo_db_close();
	return 0;

}

/*
int eliminarMongo(){

	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	mongo_db_open();


	doc = bson_new ();
	bson_oid_init (&oid, NULL);
	BSON_APPEND_OID (doc, "_id", &oid);
	BSON_APPEND_UTF8 (doc, "hello", "world");

	if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
		printf ("Insert failed: %s\n", error.message);
	}

	bson_destroy (doc);

	doc = bson_new ();
	BSON_APPEND_OID (doc, "_id", &oid);

	if (!mongoc_collection_remove (collection, MONGOC_DELETE_SINGLE_REMOVE, doc, NULL, &error)) {
		printf ("Delete failed: %s\n", error.message);
	}

	bson_destroy (doc);

	mongo_db_close();
	return 0;
}


int leerMongo(){

	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;


	mongo_db_open();

	query  = bson_new ();
	cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
	    printf ("ACA:%s\n", str);
	    bson_free (str);
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);


	return 0;
}*/


void leerJSON(t_tipoAcceso tipoAcceso){

	FILE *in;
	bson_t *query;
	mongoc_cursor_t *cursor;
	char *cadena = malloc(100);
	char *origen;
	bson_oid_t oid;
	bson_t *doc;
	bson_error_t error;

	t_archivo_json *archivo = malloc(sizeof (t_archivo_json));
	t_directorio_json *directorio = malloc(sizeof (t_directorio_json));

	switch(tipoAcceso){
		case SonArchivos: {

			mongo_db_archivos_open();

			if ((in = fopen(JSON_FILE,"rt")) == NULL ){
				break;
				}

			while(fgets(cadena, 100, in) != NULL){

				for(origen = strtok(cadena, "{,}'\n'"); origen;  origen = strtok(NULL, "{,}'\n'"))
					if (origen[0] != '\n'){
						printf("%s\n", origen);
						obtenerCampoValor(origen, archivo, NULL, tipoAcceso);
						};


				doc   = bson_new();
				query = bson_new();

				bson_oid_init (&oid, NULL);

				BSON_APPEND_UTF8(query, "nombre", archivo->nombre);

//				if((cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL)) == NULL){

					BSON_APPEND_OID (doc, "_id", &oid);
					BSON_APPEND_UTF8  (doc, "nombre", archivo->nombre);
					BSON_APPEND_INT32 (doc, "directorio", archivo->directorio);
					BSON_APPEND_INT32 (doc, "tamanio", archivo->tamanio);
					BSON_APPEND_INT32 (doc, "bloques", archivo->bloques);
					BSON_APPEND_INT32 (doc, "estado", archivo->estado);

					if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error))
						printf ("%s\n", error.message);

					bson_destroy (doc);
					bson_destroy (query);

//				};
			};

		//	free(cadena);
			fclose(in);

			mongo_db_close();
			break;
		}

		case SonDirectorios: {

			mongo_db_directorios_open();

			if ((in = fopen(JSON_FILE2,"rt")) == NULL ){
				break;
				}

			while(fgets(cadena, 100, in) != NULL){

				doc   = bson_new();
				query = bson_new();

				bson_oid_init (&oid, NULL);

				for(origen = strtok(cadena, "{,}'\n'"); origen;  origen = strtok(NULL, "{,}'\n'"))
					if (origen[0] != '\n'){
						printf("%s\n", origen);
						obtenerCampoValor(origen, NULL, directorio, tipoAcceso);
						};

//				BSON_APPEND_INT32(query, "Index", directorio->index);

//				if((cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL)) == NULL){

					BSON_APPEND_OID (doc, "_id", &oid);
					BSON_APPEND_INT32 (doc, "Index", directorio->index);
					BSON_APPEND_UTF8  (doc, "Directorio", directorio->directorio);
					BSON_APPEND_INT32 (doc, "Padre", directorio->directorio_padre);

					if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error))
						printf ("%s\n", error.message);

					bson_destroy (doc);
					bson_destroy (query);

//				};
			};

		//	free(cadena);
			fclose(in);

			mongo_db_close();
			break;
		}
	};




}


void obtenerCampoValor(const char *origen, t_archivo_json *archivo, t_directorio_json *directorio, t_tipoAcceso tipoAcceso){

	int i, j, k;
	char *campo = malloc(20);
	char *valor = malloc(20);

	for(i = 0, j = 0; origen[i] && origen[i] != ':'; i++){
		if (origen[i] != '\"')
			campo[j++] = origen[i];
	}

	campo[j] = '\0';

	for(i++, k = 0; origen[i]; i++){
		if (origen[i] != '\"')
			valor[k++] = origen[i];
	};

	valor[k] = '\0';

	printf("%s %s\n", campo, valor);

	switch(tipoAcceso){
		case SonArchivos:{
			if (strcmp(campo, "nombre") == 0){
				memset(archivo->nombre, '\0', sizeof(archivo->nombre));
				memcpy(archivo->nombre, valor, k);
			}


			if (strcmp(campo, "directorio") == 0)
				archivo->directorio = atoi(valor);

			if (strcmp(campo, "tamanio") == 0)
				archivo->tamanio = atoi(valor);

			if (strcmp(campo, "bloques") == 0)
				archivo->bloques = atoi(valor);

			if (strcmp(campo, "estado") == 0)
				archivo->estado = atoi(valor);

			free(campo);
			free(valor);
			break;

		}
		case SonDirectorios:{
			if (strcmp(campo, "directorio") == 0){
				memset(directorio->directorio, '\0', sizeof(directorio->directorio));
				memcpy(directorio->directorio, valor, k);
			}


			if (strcmp(campo, "index") == 0)
				directorio->index = atoi(valor);

			if (strcmp(campo, "directorio_padre") == 0)
				directorio->directorio_padre = atoi(valor);

			free(campo);
			free(valor);
			break;

		}

	}


}


t_archivo_json mongo_get_archivo(){
	t_archivo_json archivo;
	return archivo;
}


void mongo_db_directorios_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "MDFS_DIRECTORIOS");
}

void mongo_db_archivos_open(){
	mongoc_init ();
	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "MDFS_ARCHIVOS");
}

void mongo_db_close(){
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);
}



