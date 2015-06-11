/*
 * mongodev.c
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#include "mongodev.h"


int iniciarMongo(){

	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;
	bson_t *query;
	mongoc_cursor_t *cursor;


	mongoc_init ();

	if ((client = mongoc_client_new ("mongodb://localhost:27017/")) == NULL)
		return EXIT_FAILURE;

	collection = mongoc_client_get_collection (client, "test", "MDFS");

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
	}


	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);

    return 0;

}

int eliminarMongo(){
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;
	bson_oid_t oid;
	bson_t *doc;

	mongoc_init ();

	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "archivos");


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
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);

	return 0;
}

int leerMongo(){
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_t *query;
	char *str;

	mongoc_init ();

	client = mongoc_client_new ("mongodb://localhost:27017/");
	collection = mongoc_client_get_collection (client, "test", "MDFS");

	query  = bson_new ();
	cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
	    printf ("ACA:%s\n", str);
	    bson_free (str);
	}

	bson_destroy (query);
	mongoc_cursor_destroy (cursor);
	mongoc_collection_destroy (collection);
	mongoc_client_destroy (client);

	return 0;
}



