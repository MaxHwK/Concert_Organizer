#include <arpa/inet.h>
#include "Client.h"

char *Client_getIpS(Client client) {
    return inet_ntoa(client->_ip);
}

struct in_addr Client_getIpI(Client client) {  
    return client->_ip;  // Retourne l ip du client dans ip
}

char *Client_getUsername(Client client) {  
    return client->_username;  // Retourne le nom du client dans nom
}
