/*
 * mongodev.h
 *
 *  Created on: 30/5/2015
 *      Author: gabriel
 */

#ifndef MONGODEV_H_
#define MONGODEV_H_

#include <mongoc.h>

#define JSON_FILE	"/workspace/sisop/tp-2015-1c-los-barderos/reducemapfast/filesystem/src/filesystem.json"


mongoc_collection_t *collection;
mongoc_client_t *client;

int iniciarMongo();
int eliminarMongo();
int leerMongo();


#endif /* MONGODEV_H_ */
