#ifndef DEF_SERVER_SERVER_H
#define DEF_SERVER_SERVER_H
#include <stdbool.h>
#include <netinet/in.h>
#include "Client.h"

// -------- Definition des differents types Server -------- //

typedef struct _server_ *Server;

Server Server_create(uint16_t);

void Server_destroy(Server);

void Server_onConnectedClient(Server, void (*)(Client));

void Server_onDisconnectedClient(Server, void (*)(Client));

void Server_onMessageRecieved(Server, size_t (*)(Client, void *, size_t, void *));

bool Server_run(Server);

#endif