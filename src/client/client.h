#include "../common/net_node.h"

typedef struct server client;

int client_init(client** sv, const address* addr);
int client_free(client* sv);
