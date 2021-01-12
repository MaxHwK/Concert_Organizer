#ifndef DEF_CLIENT_H
#define DEF_CLIENT_H
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

// -------- Definition des differents types Client -------- //

typedef struct _client_ *Client;

Client Client_createS(char *, char *, uint16_t);

Client Client_createI(char *, struct in_addr, uint16_t);

void Client_disconnect(Client);

void Client_destroy(Client);

ssize_t Client_send(Client, void *, size_t, void *);

#endif
