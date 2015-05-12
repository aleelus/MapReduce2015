#include "sockets.h"


//Destruir buffer
void sockets_buffer_destroy(t_socket_buffer *tbuffer){
	free(tbuffer);
};			

//Setear modo
void sockets_setMode(t_socket *socket, e_socket_mode mode){
	socket->mode = mode;
};

//Obtener modo
e_socket_mode sockets_getMode(t_socket *socket){
	return socket->mode;
};	

//Determina si un socket esta bloqueado
int	sockets_isBlocked(t_socket *socket){
	return socket->mode == SOCKETMODE_BLOCK;
};	

//Crea un socket en modo cliente
t_socket_client	*sockets_create_client(char *ip, int port){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	if( (client->socket = sockets_create(ip, port)) == NULL ){
		free(client);
		return NULL;
	}
	if (client->socket == NULL){
		return NULL;
	}
	sockets_setState(client, SOCKETSTATE_DISCONNECTED);
	return client;	
};	

//Determina si un socket esta conectado			
int	sockets_isConnected(t_socket_client *client){
	return client->state == SOCKETSTATE_CONNECTED;
};		

//Determina si 2 clientes son iguales	
int sockets_equalsClients(t_socket_client *client1, t_socket_client *client2){
	return client1->socket->desc ==  client2->socket->desc;
};	

//Determina el estado de un socket
e_socket_state sockets_getState(t_socket_client *client, e_socket_state state){
	return client->state;
};	

//Conecta un socket
int sockets_connect(t_socket_client *client, char *ip, int port){
	t_socket *serv_socket = malloc( sizeof(t_socket) );
	serv_socket->my_addr = sockets_makeaddr(ip, port);

	if ( connect(client->socket->desc, (struct sockaddr *)serv_socket->my_addr , sizeof(struct sockaddr_in) ) == -1) {
		free(serv_socket->my_addr);
		free(serv_socket);
		client->serv_socket = NULL;
		return 0;
	}
	client->serv_socket = serv_socket;
	sockets_setState(client, SOCKETSTATE_CONNECTED);
	return 1;
};	

//Envia datos
int sockets_send(t_socket_client *client, void *data, int datalen){
	int aux = send(client->socket->desc, data, datalen, 0);
	if( aux == -1 )
		sockets_setState(client, SOCKETSTATE_DISCONNECTED);
	return aux;	
};	

//Envia buffer
int sockets_sendbuffer(t_socket_client *client, t_socket_buffer *buffer){
	return sockets_send(client, buffer->data, buffer->size);
};	

//Envia una cadena
int sockets_sendString(t_socket_client *client, char *str){
	return sockets_send(client, (void*)str, strlen(str)+1);
};	

//Recibe datos
t_socket_buffer *sockets_recv(t_socket_client *client){
	t_socket_buffer *tbuffer = malloc( sizeof(t_socket_buffer) );
	int datasize = sockets_recvInBuffer(client, tbuffer);

	if( datasize <= 0 ){
		free(tbuffer);
		return NULL;
	}

	return tbuffer;	
};	

//Recive buffer
int sockets_recvInBuffer(t_socket_client *client, t_socket_buffer *buffer){
	memset(buffer->data, 0, DEFAULT_BUFFER_SIZE);
	if( !sockets_isBlocked(client->socket) ){
		fcntl(client->socket->desc, F_SETFL, O_NONBLOCK);
	}
	buffer->size = recv(client->socket->desc, buffer->data, 3, MSG_PEEK |MSG_WAITALL);
	uint16_t size_payload;
	memcpy(&size_payload,buffer->data+1,2);
	buffer->size = recv(client->socket->desc, buffer->data, size_payload + 3, MSG_WAITALL);
	if( !sockets_isBlocked(client->socket) ){
		fcntl(client->socket->desc, F_SETFL, O_NONBLOCK);
	}

	if( buffer->size == -1 )
		sockets_setState(client, SOCKETSTATE_DISCONNECTED);

	return buffer->size;
};	

//Termina socket
int sockets_destroy_client(t_socket_client *client){
	if( !sockets_isConnected(client) ) client->socket->desc = -1;
	sockets_destroy(client->socket);
	free(client);	
};	

//Crea un socket en modo server
t_socket_server *sockets_create_server(char *ip, int port){
	t_socket_server *socket = malloc( sizeof(t_socket_server) );
	if( (socket->socket = sockets_create(ip, port)) == NULL ){
		free(socket);
		return NULL;
	}
	socket->maxconnections = DEFAULT_MAX_CONNECTIONS;
	return socket;	
};	

//Maximo de conexiones
void sockets_setMaxConnections(t_socket_server *server, int connections){
	server->maxconnections = connections;
};	

//Obtener conexiones
int sockets_getMaxConnections(t_socket_server *server){
	return server->maxconnections;
};	

//Escuchar
int sockets_listen(t_socket_server *server){
	if (listen(server->socket->desc, server->maxconnections) == -1) {
		return 0;
	}
	return 1;	
};	

//Aceptar conexion
t_socket_client *sockets_accept(t_socket_server *server){
	t_socket_client* client = malloc( sizeof(t_socket_client) );
	int addrlen = sizeof(struct sockaddr_in);
	client->socket = malloc( sizeof(t_socket) );
	client->socket->my_addr = malloc( sizeof(struct sockaddr) );

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	if ((client->socket->desc = accept(server->socket->desc, (struct sockaddr *)client->socket->my_addr, (void *)&addrlen)) == -1) {
		free(client->socket->my_addr);
		free(client->socket);
		free(client);
		return NULL;
	}

	if( !sockets_isBlocked(server->socket) ){
		fcntl(server->socket->desc, F_SETFL, O_NONBLOCK);
	}

	sockets_setState(client, SOCKETSTATE_CONNECTED);
	sockets_setMode(client->socket, SOCKETMODE_BLOCK);
	return client;	
};	

//Termina socket
void sockets_destroy_server(t_socket_server *server){
	sockets_destroy(server->socket);
	free(server);	
};	

//Crear socket
t_socket *sockets_create(char* ip, int port){
	t_socket* socket_l = malloc( sizeof(t_socket) );
	socket_l->desc = socket(AF_INET, SOCK_STREAM, 0);
	int flag = 1;
	setsockopt(socket_l->desc, SOL_TCP, TCP_NODELAY, &flag, sizeof(flag));
	if( !sockets_bind(socket_l, ip, port) ){
		free(socket_l);
		return NULL;
	}
	sockets_setMode(socket_l, SOCKETMODE_BLOCK);
	return socket_l;
}

//Generar socketsaddr
void *sockets_makeaddr (char* ip, int port){
	struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	if(ip == NULL){
		addr->sin_addr.s_addr=INADDR_ANY;
	}else{
		addr->sin_addr.s_addr=inet_addr(ip);
	}
	addr->sin_port = htons(port);
    return addr;	
};

//Setea estado
void sockets_setState(t_socket_client *client, e_socket_state state){
	client->state = state;
};

//Libera socket
void sockets_destroy(t_socket* socket){
	if( socket->desc > 0 ){
		close(socket->desc);
	}
	free(socket->my_addr);
	free(socket);
};

//Bindea socket
int sockets_bind(t_socket* socket, char* ip, int port){
	int yes=1;
	socket->my_addr = sockets_makeaddr(ip,port);

	if (setsockopt(socket->desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		free(socket->my_addr);
		socket->my_addr = NULL;
		return 0;
	}

	if (bind(socket->desc, (struct sockaddr *)socket->my_addr, sizeof(struct sockaddr_in)) == -1) {
		free(socket->my_addr);
		socket->my_addr = NULL;
		return 0;
	}
	return 1;
};


//Escribir en cliente
int sockets_write(t_socket_client *client, void *data, int datalen){
	return write(client->socket->desc, data, datalen);
};