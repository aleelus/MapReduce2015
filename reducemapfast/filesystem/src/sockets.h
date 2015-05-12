
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/tcp.h>


#define DEFAULT_BUFFER_SIZE		2048	//Tamanio de buffer por defecto
#define DEFAULT_MAX_CONNECTIONS	100		//Tamanio de conexiones por defecto
#define SELECT_USEC_TIMEOUT		500		//Timeout
#define SOCKET_MAX_BUFFER		100		//Tamanio maximo de datos a recibir

typedef enum {
	SOCKETSTATE_CONNECTED,
	SOCKETSTATE_DISCONNECTED,
} e_socket_state;

typedef enum {
	SOCKETMODE_NONBLOCK = 1,
	SOCKETMODE_BLOCK    = 2,
} e_socket_mode;

typedef struct {
	int desc;
	struct sockaddr_in *my_addr;
	e_socket_mode mode;
} t_socket;

typedef struct {
	t_socket *socket;
	t_socket *serv_socket;
	e_socket_state	state;
} t_socket_client;

typedef struct {
	t_socket *socket;
	int maxconnections;
} t_socket_server;

typedef struct {
	char data[DEFAULT_BUFFER_SIZE];
	int size;
} t_socket_buffer;


void 		  	sockets_buffer_destroy(t_socket_buffer *tbuffer);							//Destruir buffer
void 			*sockets_makeaddr (char* ip, int port);										//Generar socketaddr
void 		  	sockets_setMode(t_socket *socket, e_socket_mode mode);						//Setear modo
e_socket_mode 	sockets_getMode(t_socket *socket);											//Obtener modo
int			  	sockets_isBlocked(t_socket *socket);										//Determina si un socket esta bloqueado
t_socket_client	*sockets_create_client(char *ip, int port);									//Crea un socket en modo cliente
int				sockets_isConnected(t_socket_client *client);								//Determina si un socket esta conectado
int  			sockets_equalsClients(t_socket_client *client1, t_socket_client *client2);	//Determina si 2 clientes son iguales

e_socket_state	sockets_getState(t_socket_client *client, e_socket_state state);			//Determina el estado de un socket
int 			sockets_connect(t_socket_client *client, char *ip, int port);				//Conecta un socket
int 			sockets_send(t_socket_client *client, void *data, int datalen);				//Envia datos
int 			sockets_sendbuffer(t_socket_client *client, t_socket_buffer *buffer);		//Envia buffer
int  			sockets_sendString(t_socket_client *client, char *str);						//Envia una cadena
t_socket_buffer *sockets_recv(t_socket_client *client);										//Recibe datos
int 			sockets_recvInBuffer(t_socket_client *client, t_socket_buffer *buffer);		//Recive buffer
int 			sockets_destroy_client(t_socket_client *client);							//Termina socket

t_socket_server *sockets_create_server(char *ip, int port);									//Crea un socket en modo server
void			sockets_setMaxConnections(t_socket_server *server, int connections);		//Maximo de conexiones
int 			sockets_getMaxConnections(t_socket_server *server);							//Obtener conexiones
int 			sockets_listen(t_socket_server *server);									//Escuchar
t_socket_client *sockets_accept(t_socket_server *server);									//Aceptar conexion
void 			sockets_destroy_server(t_socket_server *server);							//Termina socket
t_socket 		*sockets_create(char* ip, int port);										//Crear socket

void 			sockets_setState(t_socket_client *client, e_socket_state state);			//Setea estado
void 			sockets_destroy(t_socket* sckt);											//Libera socket
int 			sockets_bind(t_socket* sckt, char* ip, int port);							//Bindea socket
int 			sockets_write(t_socket_client *client, void *data, int datalen);			//Escribir en cliente
