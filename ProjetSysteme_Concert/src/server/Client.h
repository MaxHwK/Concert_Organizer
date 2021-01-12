#ifndef DEF_SERVER_CLIENT_H
#define DEF_SERVER_CLIENT_H
#include <netinet/in.h>

typedef struct _client_ {
    struct in_addr _ip;
    char *_username;
} * Client;

/**
 * @brief  Returns the IP address of the client.
 * @param Client client to use
 * @return integer of the IP address 
 */
struct in_addr Client_getIpI(Client);

/**
 * @brief  Returns the IP address of the client.
 * @param Client client to use
 * @return string of the IP address 
 */
char *Client_getIpS(Client);

/**
 * @brief  Returns the username of the client.
 * @param Client client to use
 * @return string of the username 
 */
char *Client_getUsername(Client);

#endif