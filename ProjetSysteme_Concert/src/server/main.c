#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include "Server.h"
#include "../Style.h"
#include "../requests.h"

static sem_t serverDataAccess;

// -------- Decompose nom/prenom depuis le buffer -------- //

size_t deserializeId(void *buffer, char *name, char *surname) {
    uint64_t nameLen = *(uint64_t *)buffer;  // Taille du prenom
    buffer += sizeof(uint64_t);
    memcpy(name, buffer, sizeof(char) * nameLen);
    name[nameLen] = '\0';  // On remet le buffer a 0
    buffer += nameLen * sizeof(char);
    uint64_t surnameLen = *(uint64_t *)buffer;  // Taille du nom
    buffer += sizeof(uint64_t);
    memcpy(surname, buffer, sizeof(char) * surnameLen);
    surname[surnameLen] = '\0';  // On remet le buffer a 0
    buffer += surnameLen * sizeof(char);
    return 2 * sizeof(uint64_t) + sizeof(char) * (nameLen + surnameLen);
}

typedef struct _place_ {
    char *reservationNumber;
    char *name;
    char *surname;
} * Place;

typedef struct _serverData_ {
    Place *places;
} * ServerData;

static ServerData serverData;
static FILE *dataStream;

// -------- Ecriture des donnees du serveur -------- //

void saveServerData(FILE *file) {
    sem_wait(&serverDataAccess);
    fseek(file, 0, SEEK_SET);
    fwrite("SDAT", 4 * sizeof(char), 1, file);
    uint8_t _b[320];  // Utilisation du buffer pour remplir
    for (int i = 0; i < 100; ++i) {
        void *buffer = _b;  // Remplissage du buffer avant d ecrire
        size_t nameSize = serverData->places[i]->reservationNumber ? strlen(serverData->places[i]->name) : 0;  // Si non reservation alors on mets à 0
        size_t surnameSize = serverData->places[i]->reservationNumber ? strlen(serverData->places[i]->surname) : 0;  // Pareil pour le nom
        *(uint8_t *)buffer = nameSize;
        buffer += sizeof(uint8_t);
        *(uint8_t *)buffer = surnameSize;
        buffer += sizeof(uint8_t);
        if (serverData->places[i]->reservationNumber) {
            memcpy(buffer, serverData->places[i]->reservationNumber, 10 * sizeof(char));  // Si reservation alors on ecrit dans le buffer
            buffer += sizeof(char) * 10;
            memcpy(buffer, serverData->places[i]->name, nameSize * sizeof(char));  // Pareil pour ici
            buffer += nameSize * sizeof(char);
            memcpy(buffer, serverData->places[i]->surname, surnameSize * sizeof(char));  // Pareil pour ici
            buffer += surnameSize * sizeof(char);
        }
        else {
            for (int j = 0; j < 10; ++j) {  // Si non reservation alors le fichier est vide
                *(char *)buffer = 0;
                buffer += sizeof(char);
            }
        }
        fwrite(_b, (size_t)buffer - (size_t)_b, 1, file);  // Ecriture dans le fichier
    }
    fflush(file);  // Sans oublier de flush les donnees pas encore ecrites
    sem_post(&serverDataAccess);
}

void loadServerData(FILE *file) {
    sem_wait(&serverDataAccess);
    fseek(file, 0, SEEK_END);

    if (ftell(file) < 4) {  // Verification sur le fichier
        sem_post(&serverDataAccess);
        return;
    }

    fseek(file, 0, SEEK_SET);
    uint8_t _b[310];
    fread(_b, sizeof(char) * 4, 1, file);  // Recuperation des 4 premiers octets
    _b[4] = '\0';

    if (strcmp("SDAT", (char *)_b)) {  // Verification du format du fichier
        sem_post(&serverDataAccess);
        return;
    }

    for (int i = 0; i < 100; ++i) {  // Chargement des tailles de nom/prenom + numero dossier
        fread(_b, sizeof(uint8_t), 12, file);
        void *buffer = _b;
        size_t nameSize = *(uint8_t *)buffer;
        buffer += sizeof(uint8_t);
        size_t surnameSize = *(uint8_t *)buffer;
        buffer += sizeof(uint8_t);  // Si il n y a rien dedans
        if (*(char *)buffer == 0)
            serverData->places[i]->reservationNumber = NULL;
        else {
            serverData->places[i]->reservationNumber = (char *)malloc(sizeof(char) * 11);  // Sinon, allocation du dossier
            serverData->places[i]->reservationNumber[10] = '\0';
            memcpy(serverData->places[i]->reservationNumber, buffer, sizeof(char) * 10);
            buffer = _b;
            fread(_b, sizeof(char) * (nameSize + surnameSize), 1, file);
            serverData->places[i]->name = (char *)malloc(sizeof(char) * (nameSize + 1));
            serverData->places[i]->surname = (char *)malloc(sizeof(char) * (surnameSize + 1));
            memcpy(serverData->places[i]->name, buffer, sizeof(char) * nameSize);
            buffer += sizeof(char) * nameSize;
            serverData->places[i]->name[nameSize] = '\0';
            memcpy(serverData->places[i]->surname, buffer, sizeof(char) * surnameSize);
            serverData->places[i]->surname[nameSize] = '\0';
        }
    }
    sem_post(&serverDataAccess);
}

void clientConnected(Client client) {
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client)); 
    printf(":");  // Affichage d un client connecte
    console_formatSystemForeground(" Un Nouveau Client vient de se Connecter !", CONSOLE_COLOR_BRIGHT_GREEN);
    printf("\n");
}

void clientDisconnected(Client client) {
    console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
    printf(":");  // Affichage d un client deconnecte
    console_formatSystemForeground(" Un Client vient de se Deconnecter !", CONSOLE_COLOR_BRIGHT_RED);
    printf("\n");
}

size_t message(Client client, void *data, size_t dataSize, void *buffer) {
    uint8_t requestType = *(uint8_t *)data;  // Requete type est dans le 1er octet
    data += sizeof(uint8_t);   
    switch (requestType) {  // Depend du type de la requete
        case LIST_REQUEST: {
            char name[100], surname[100];     
            deserializeId(data, name, surname);  // Recuperation nom et prenom
            void *quantityPtr = buffer;
            buffer += sizeof(uint8_t);
            uint8_t quantity = 0;
            sem_wait(&serverDataAccess); 

            for (int i = 0; i < 100; i++)   
                if (serverData->places[i]->reservationNumber != NULL)  // Recherche places attribuees
                
            if (!strcmp(serverData->places[i]->name, name) && !strcmp(serverData->places[i]->surname, surname)) { 
                *(uint8_t *)buffer = i;  // Stock du numero de place
                buffer += sizeof(uint8_t);
                memcpy(buffer, serverData->places[i]->reservationNumber, sizeof(char) * 10);  // Stock du numero de dossier
                buffer += 10 * sizeof(char);
                ++quantity;
            }
        
            sem_post(&serverDataAccess);
            *(uint8_t *)quantityPtr = quantity; // On met quantite avec reponses
            console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
            printf(": Un client est en train de regarder ces places réservées, ");
            console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_GREY, quantity);
            printf(" place réservée pour le moment.\n");  
            return sizeof(uint8_t) + (sizeof(char) * 10 + sizeof(uint8_t)) * quantity;  // Renvoi taille reponse
        }
        break;

        case NAME_REQUEST: {
            char name[100], surname[100]; 
            uint8_t nameSize = *(uint8_t *)data;  // Tailles du nom et prenom
            data += sizeof(uint8_t);
            uint8_t surnameSize = *(uint8_t *)data;
            data += sizeof(uint8_t);
            memcpy(name, data, sizeof(char) * nameSize);  // Recuperation du prenom
            data += sizeof(char) * nameSize;
            name[nameSize] = '\0';  // On remet le buffer a 0
            memcpy(surname, data, sizeof(char) * surnameSize);  // Recuperation du nom
            surname[surnameSize] = '\0';  // On remet le buffer a 0
            console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
            printf(": Client Connecté sous le nom : ");
            console_formatMode("%s %s", CONSOLE_FLAG_BLINK, name, surname);
            printf("\n");
            return 0;
        }
        break;     

        case AVAILABILITY_REQUEST: {
            char name[100], surname[100];
            deserializeId(data, name, surname);  // Recuperation nom et prenom
            sem_wait(&serverDataAccess);
            for (int i = 0; i < 13; i++) {  // La reponse est de 13 octets
                uint8_t currState = 0;  // Octet en cours
                for (int j = 0; j < 8; j++) {   
                    int currID = i * 8 + j;  // Calcul de l id de chaque place
                    if (currID < 100) {  // Petite verification du nombre de bits        
                        if (j > 0)    
                            currState >>= 1;  // On decale alors tout vers la droite
                        currState |= (serverData->places[currID]->reservationNumber == NULL) << 7;
                    }
                    else {         
                        currState >>= 4;  // Si on arrive au bout
                        break;
                    }
                }
                *(uint8_t *)buffer = currState;  // Ecriture de l octet
                buffer += sizeof(uint8_t);
            }
            sem_post(&serverDataAccess);
            console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
            printf(": Un client est en train de regarder les places restantes.\n");
            return 13 * sizeof(uint8_t);
        }
        break;

        case REMOVE_REQUEST: {
            char name[100], surname[100];
            char code[11];  // Numero de dossier
            code[10] = '\0'; 
            data += deserializeId(data, name, surname);  // Recuperation du nom et prenom    
            memcpy(code, data, 10 * sizeof(char));  // Recuperation du numero de dossier  
            bool found = false;     
            *(uint8_t *)buffer = 0;  // On renvoit 0      
            sem_wait(&serverDataAccess);
        
            for (int i = 0; i < 100; i++)  // Pour les 100 places disponibles
            
            if (serverData->places[i]->reservationNumber != NULL) {  // Si la place n est pas reserve  
                if (!strcmp(code, serverData->places[i]->reservationNumber)) {  // Si le numero correspond
                    found = true;
                    if (!strcmp(name, serverData->places[i]->name) && !strcmp(surname, serverData->places[i]->surname)) {  // Si nom et prenom correspondent        
                        *(uint8_t *)buffer = 1;  // On renvoit 1    
                        free(serverData->places[i]->reservationNumber);  // On supprime alors la reservation
                        free(serverData->places[i]->name);
                        free(serverData->places[i]->surname);
                        serverData->places[i]->reservationNumber = NULL;
                    }
                    break;
                }
            }

            sem_post(&serverDataAccess);
            saveServerData(dataStream);
            console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
            printf(": Annulation de la place avec le dossier n°");
            console_formatMode(code, CONSOLE_FLAG_BOLD);
            printf(" ");

            if (*(uint8_t *)buffer)
                console_formatSystemForeground("autorisée", CONSOLE_COLOR_BRIGHT_GREY);
            else {
                console_formatSystemForeground("refusée", CONSOLE_COLOR_BRIGHT_GREY);
                printf(", car : ");
                if (found)
                    console_formatSystemForeground("Mauvais client !", CONSOLE_COLOR_BRIGHT_RED);
                else
                    console_formatSystemForeground("Dossier inconnu !", CONSOLE_COLOR_BRIGHT_RED);
            }
            printf("\n");
            return sizeof(uint8_t);
        }
        break;

        case PLACE_REQUEST: {
            char name[100], surname[100];
            uint8_t place; 
            data += deserializeId(data, name, surname);  // Recuperation nom et prenom
            place = *(uint8_t *)data;

            if (place > 99)     
                *(uint8_t *)buffer = 0;  // Si la place est incorrecte
            else {
                sem_wait(&serverDataAccess); 
                if (serverData->places[place]->reservationNumber == NULL) {
                    char code[11];  // Numero de dossier
                    code[10] = '\0';

                    for (int i = 0; i < 10; i++) 
                        code[i] = '0' + (rand() % 10);  // Dossier genere

                    serverData->places[place]->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
                    serverData->places[place]->surname = (char *)malloc((strlen(surname) + 1) * sizeof(char));
                    serverData->places[place]->reservationNumber = (char *)malloc(11 * sizeof(char));
                
                    memcpy(serverData->places[place]->name, name, (strlen(name) + 1) * sizeof(char));
                    memcpy(serverData->places[place]->surname, surname, (strlen(surname) + 1) * sizeof(char));
                    memcpy(serverData->places[place]->reservationNumber, code, 11 * sizeof(char));
                    *(uint8_t *)buffer = 1;  // Aucune erreur
                }
                else
                    *(uint8_t *)buffer = 0;  // Dossier existant
                sem_post(&serverDataAccess);
            }

            if (*(uint8_t *)buffer)
                saveServerData(dataStream);
    
            console_formatSystemForegroundMode("%s@%s", CONSOLE_COLOR_BRIGHT_GREY, CONSOLE_FLAG_BOLD, Client_getUsername(client), Client_getIpS(client));
            printf(": Un client vient de réserver la place n°");
            console_formatSystemForeground("%d", CONSOLE_COLOR_BRIGHT_GREY, place + 1);
            printf(" ");

            if (*(uint8_t *)buffer) {
                console_formatSystemForeground("avec succès", CONSOLE_COLOR_BRIGHT_GREY);
                printf(", dossier ouvert avec le n°");
                sem_wait(&serverDataAccess);
                console_formatMode(serverData->places[place]->reservationNumber, CONSOLE_FLAG_BLINK);
                sem_post(&serverDataAccess);
            }
            else {
                console_formatSystemForeground("refusée", CONSOLE_COLOR_BRIGHT_RED);
                printf(", car : ");
                if (place > 99)
                    console_formatSystemForeground("numéro de place incorrect", CONSOLE_COLOR_BRIGHT_YELLOW);
                else
                    console_formatSystemForeground("place déjà occupée", CONSOLE_COLOR_BRIGHT_YELLOW);
            }
            printf("\n");
            return sizeof(uint8_t);
        }
        break;
    }
    return 0;
}

int main(int argc, char **argv, char **args) {
    srand(time(NULL));
    sem_init(&serverDataAccess, PTHREAD_PROCESS_SHARED, 1);  
    Place _pl[100];  // Creation des donnees
    struct _serverData_ _serv;
    serverData = &_serv;
    serverData->places = _pl;

    for (int i = 0; i < 100; i++) {  // Allocation pour toutes les places
        serverData->places[i] = (Place)malloc(sizeof(struct _place_));
        serverData->places[i]->reservationNumber = NULL;
    }

    if (argc < 2) {
        console_formatSystemForeground("Usage : server <port> <fichier de sauvegarde (optionel)>", CONSOLE_COLOR_BRIGHT_RED);
        printf("\n");
        return 1;
    }

    char *file = "sauvegarde.sdat";
    if (argc > 2)
        file = argv[2];
    struct stat buffer;

    if (stat(file, &buffer) == 0) {
        dataStream = fopen(file, "r+b");
        loadServerData(dataStream);
    }
    else
        dataStream = fopen(file, "w+b");

    uint16_t port = atoi(argv[1]);
    Server server = Server_create(port);

    if (!server)
        return 0;
    
    Server_onConnectedClient(server, &clientConnected);
    Server_onDisconnectedClient(server, &clientDisconnected);
    Server_onMessageRecieved(server, &message);
    console_formatSystemForeground("--- Serveur de Reservation de Billets pour le Concert de Maxence et Dorian disponible au port %d ---", CONSOLE_COLOR_BRIGHT_BLUE, port);
    printf("\n");
    Server_run(server);
    return 0;
}