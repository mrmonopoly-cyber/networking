#include "../common/net_node.h"

typedef struct server client;

int client_init(client** sv, const address* addr);
int client_free(client* sv);
int client_send(const client* sv, const void* buffer, const unsigned int buffer_size);
int client_recv(const client* sv, void* buffer, unsigned int* buffer_size);
