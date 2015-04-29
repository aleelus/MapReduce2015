/*
 * consola.h
 *
 *  Created on: 28/04/2015
 *      Author: gabriel
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_



typedef enum{
	CantidadArgumentosIncorrecta,
	NoEsUnVolumen,
	NoSePudoAbrirIn,
	NoSePudoAbrirOut1,
	NoSePudoAbrirOut2,
	ErrorEnLectura,
	ErrorEnEscritura,
	OtroError,
} Error;


#define IP 						"127.0.0.1"
#define PUERTO					"6667"
#define PACKAGESIZE				1024


// IPv4 AF_INET sockets:
struct sockaddr_in {
    short            sin_family;
    unsigned short   sin_port;
    struct in_addr   sin_addr;
    char             sin_zero[8];
};

struct in_addr {
    unsigned long s_addr;
};

struct sockaddr {
    unsigned short    sa_family;
    char              sa_data[14];
};



int crear_socket(int *);
int conectar_socket(int *, struct sockaddr_in*);


#endif /* CONSOLA_H_ */
