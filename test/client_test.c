#include <stdlib.h>
#include <string.h>

#include "../networking.h"


int main(void)
{
    net_node* client = NULL;
    address addr_sv;
    address* addr_sv_ptr = &addr_sv;

    address_init(&addr_sv_ptr, "127.0.0.1", 8080);
    client_init(&client, addr_sv_ptr);

    char mex[1024] = "Hello\n";
    net_node_send(client, &mex, strlen(mex));
    while (1) {}
    return EXIT_SUCCESS;
}
