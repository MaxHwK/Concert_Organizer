#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include "Server.h"

#define SOCKET_BUFFER_SIZE 4800

struct _server_ {  // Structure
    int _fdWaitSocket;
    struct sockaddr_in _callerCoords;
    socklen_t _coordsSize;
    void (*_onConnect)(Client);
    void (*_onDisconnect)(Client);
    size_t (*_onMessage)(Client, void *, size_t, void *);
};

typedef struct {
    struct sockaddr_in callerCoords;
    int fdSocket;
    Server server;
} connection;

// -------- Demarrage des threads Client -------- //

void *clientThread(void *arg) {  // Gere la structure du Client
    connection *con = (connection *)arg;
    struct _client_ client; 
    client._ip = con->callerCoords.sin_addr;
    char user[65];
    {
        int readBytes = recv(con->fdSocket, user, 64, 0);  // Envoi du nom d utilisateur lors de la connexion
        if (readBytes >= 0) {
            user[readBytes] = '\0';
            client._username = user;
        }
        else {
            close(con->fdSocket);
            return NULL;
        }
    }
   
    if (con->server->_onConnect != NULL)  // Si une fonction existe pour l evenement connexion
        (*con->server->_onConnect)(&client);  // Alors on appelle cette fonction

    while (true) {  // Boucle principale
        uint8_t _b[SOCKET_BUFFER_SIZE];  // Buffer avec donnees reçues par le client
        void *buffer = _b;
        ssize_t readBytes = recv(con->fdSocket, buffer, SOCKET_BUFFER_SIZE, 0);
        if (readBytes > 0) {
            if (con->server->_onMessage != NULL) {  // Si l evenement de message reçu est ok
                uint8_t _r[SOCKET_BUFFER_SIZE];   // Buffer avec donnees
                void *response = _r;
                size_t respSize = 0;            
                respSize = (*con->server->_onMessage)(&client, buffer, readBytes, response);  // On appelle l'evenement
                if (respSize != 0)
                     send(con->fdSocket, response, respSize, 0);  // Reponse uniquement si taille reponse > 0
            }
        }
            else
                break;  // Deconnexion du client
    }
    close(con->fdSocket);  
    if (con->server->_onDisconnect != NULL)  // Lancement de la deconnexion
        (*con->server->_onDisconnect)(&client);
    free(con);  // Liberation de la memoire allouee
    pthread_exit(NULL);
}

void Server_destroy(Server server) {
    free(server);  // Destruction du serveur        
}        

Server Server_create(uint16_t port) {
    Server server = (Server)malloc(sizeof(struct _server_));  // Creation objet serveur
    struct sockaddr_in serverCoords;
    server->_fdWaitSocket = socket(PF_INET, SOCK_STREAM, 0);

    if (server->_fdWaitSocket < 0) {
        printf("Socket incorrect !\n");  // Affiche message d erreur
        return false;
    }

    memset(&serverCoords, 0x00, sizeof(struct sockaddr_in));  // Preparation adresse locale
    serverCoords.sin_family = PF_INET;
    serverCoords.sin_addr.s_addr = htonl(INADDR_ANY); // Differentes interfaces disponibles
    serverCoords.sin_port = htons(port);  // Interfaces disponibles

    if (bind(server->_fdWaitSocket, (struct sockaddr *)&serverCoords, sizeof(serverCoords)) == -1) {
        printf("Erreur de liaison !\n");  // Affiche message d erreur
        free(server);
        return NULL;
    }

    if (listen(server->_fdWaitSocket, 32) == -1) {
        printf("Erreur recnontre !\n");
        free(server);
        return NULL;
    }

    server->_coordsSize = sizeof(server->_callerCoords);
    return server;
}

bool Server_run(Server server) {
    while (true) {  // Boucle principale
        int fdCommunicationSocket;
        if ((fdCommunicationSocket = accept(server->_fdWaitSocket, (struct sockaddr *)&server->_callerCoords, &server->_coordsSize)) == -1) { 
            close(server->_fdWaitSocket);  // Si connexion d'un client disponible
            printf("Accepter erreur\n");  // Affiche message d erreur
            return false;
        }
        else {
            connection *con = (connection *)malloc(sizeof(connection));  // Allocation des coordonnees de connexion
            con->callerCoords = server->_callerCoords;
            con->fdSocket = fdCommunicationSocket;
            con->server = server;
            pthread_t my_thread1;
            pthread_create(&my_thread1, NULL, clientThread, con);  // Lancement du thread client
        }
    }
    close(server->_fdWaitSocket);
    return false;
}

void Server_onConnectedClient(Server server, void (*fct)(Client)) {
    server->_onConnect = fct;  // Attribution de l evenement connexion
}

void Server_onDisconnectedClient(Server server, void (*fct)(Client)) {
    server->_onDisconnect = fct;  // Attribution de l evenement deconnexion
}
void Server_onMessageRecieved(Server server, size_t (*fct)(Client, void *, size_t, void *)){
    server->_onMessage = fct;  // Attribution de l evenement message
}
