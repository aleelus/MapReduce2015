/*
 * consola_dev.c
 *
 *  Created on: 28/04/2015
 *      Author: gabriel
 */

#include "consola.h"


int crear_socket(int socket_desc){
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

int conectar_socket(int socket_desc, struct sockaddr_in server){
	server.sin_addr.s_addr = inet_addr("74.125.235.20");
	server.sin_family = AF_INET;
	server.sin_port = htons( 80 );

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		puts("Conection Error!");
		return EXIT_FAILURE;
	}

    puts("Connected");
    return EXIT_SUCCESS;
}

//Mostrar Ayuda
void mostrarAyuda(){
	printf("Comandos posibles: \n");
	printf("1 - Formatear MDFS\n");
	printf("2 - Eliminar/Renombrar/Mover Archivos\n");
	printf("3 - Crear/Eliminar/Renombrar/Mover Directorios\n");
	printf("4 - Copiar un archivo local al MDFS\n");
	printf("5 - Copiar un archivo del MDFS al filesystem local\n");
	printf("6 - Solicitar el MD5 de un archivo en MDFS\n");
	printf("7 - Ver/Borrar/Copiar los bloques que componen un archivo\n");
	printf("8 - Agregar un nodo de datos\n");
	printf("9 - Eliminar un nodo de datos\n");
	printf("------------------------------------------------------------\n");
	printf("Seleccione un comando: \n");
}

//Mostrar Error
void mostrarError(error unError){
	switch(unError){
		case CantidadArgumentosIncorrecta: puts("CantidadArgumentosIncorrecta.");break;
		case OtroError:	 				   puts("OtroError.");break;
	};
}


//Ejecutar un comando
void ejecutarComando(int unComando){
	switch(unComando){
		case 1:	{ formatearMDFS(); break;}
		case 2: { procesarArchivo(); break;}
		case 3: { procesarDirectorio(); break;}
		case 4: { copiarLocalAlMFDS(); break;}
		case 5: { copiarMDFSalFilesystem(); break;}
		case 6: { solicitarMD5(); break;}
		case 7: { procesarBloques(); break;}
		case 8: { agregarNodo(); break;}
		case 9: { eliminarNodo(); break;}
		default: { comandoDesconocido(); break;}
	}
}

//Formatear el MDFS
void formatearMDFS(){
	println("Esta opcion debe formatear el MDFS");
};

//Procesar archivo
void procesarArchivo(){
	println("Esta opcion debe procesar un archivo");
};

//Procesar directorio
void procesarDirectorio(){
	println("Esta opcion debe procesar un directorio");
};

//Copiar archivo local al MDFS
void copiarLocalAlMFDS(){
	println("Esta opcion debe copiar un archivo local al MDFS");
};

//Copiar archivo del MDFS al Filesystem
void copiarMDFSalFilesystem(){
	println("Esta opcion debe copiar un archivo del MDFS al Filesystem");
};

//Solicitar MD5 de un archivo
void solicitarMD5(){
	println("Esta opcion debe solicitar el MD5 de un archivo");
};

//Procesar bloques
void procesarBloques(){
	println("Esta opcion debe procesar bloques");
};

//Agregar un nodo
void agregarNodo(){
	println("Esta opcion debe agregar un nodo");
};

//Eliminar un nodo
void eliminarNodo(){
	println("Esta opcion debe eliminar un nodo");
};

//Comando desconocido
void comandoDesconocido(){
	println("Esta opcion debe solicitar que ingrese un comando valido");
};
