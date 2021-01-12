#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Client.h"

#define SOCKET_BUFFER_SIZE 4800

struct _client_ {  // Creation structure
    int _fdSocket;
};

Client Client_createI(char *username, struct in_addr ip, uint16_t port) {
    Client client = (Client)malloc(sizeof(struct _client_));
    struct sockaddr_in servCoords;
    client->_fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (client->_fdSocket < 0) {  // Test
        printf("Socket Incorrect !\n");  // Affichage message d erreur
        return NULL;
    }

    memset(&servCoords, 0x00, sizeof(struct sockaddr_in));   // Preparation du serveur
    servCoords.sin_family = PF_INET;  // Coordonnees du serveur
    servCoords.sin_addr = ip;  // Adresse du serveur
    servCoords.sin_port = htons(port);  // Differentes interfaces locales

    if (connect(client->_fdSocket, (struct sockaddr *)&servCoords, sizeof(servCoords)) == -1) {  // Test
        printf("Connexion Impossible !\n");  // Affichage message d erreur
        return NULL;
    }

    send(client->_fdSocket, username, strlen(username), 0);  // Envoi du client au serveur
    return client;
}

Client Client_createS(char *username, char *ip, uint16_t port) {
    struct in_addr address;
    inet_aton(ip, &address);
    return Client_createI(username, address, port);
}

void Client_disconnect(Client client) {
    close(client->_fdSocket);
}

void Client_destroy(Client client) {
    free(client);
}

ssize_t Client_send(Client client, void *request, size_t reqSize, void *response) {
    if (reqSize == 0) {  // Test
        printf("Taille de donnee incoherente !"); // Affichage message d erreur
        return 0;
    }

    send(client->_fdSocket, request, reqSize, 0);

    if (response != NULL) {
        ssize_t respSize = recv(client->_fdSocket, response, SOCKET_BUFFER_SIZE, 0);
        if (respSize == -1)
            close(client->_fdSocket);
        return respSize;
    }
    else
        return 0;
}
