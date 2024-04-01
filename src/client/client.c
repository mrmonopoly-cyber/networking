#include "client.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

uint8_t 
client_check_input_pointer(const void* ptr, const char* ptr_name)
{
    if (!ptr) {
        fprintf(stderr, "ERROR: invalid pointer %s\n", ptr_name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int 
client_init(net_node** o_sv, const address* addr)
{
    if(client_check_input_pointer(o_sv, "result addr")) goto exit;;
    if(client_check_input_pointer(addr, "server addr")) goto exit;

    socket_t socket_sv =-1;

    if((socket_sv = socket(AF_INET, SOCK_STREAM, 0))<0){
        goto exit;
    }


    const struct sockaddr_in sv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(addr->_addr_str),
        .sin_port = htons(addr->port),
    };

    if (connect(socket_sv,(struct sockaddr* )  &sv_addr, sizeof(sv_addr))<0) {
        goto exit;
    }

    if (!(*o_sv)) {
        *o_sv = calloc(1, sizeof(net_node));
        if (client_check_input_pointer(*o_sv, "allocate pointer")) goto exit;
    }
    memcpy(&(*o_sv)->_addr, addr, sizeof(*addr));
    (*o_sv)->_my_socket = socket_sv;
    
    return EXIT_SUCCESS;

exit:
    *o_sv = NULL;
    return EXIT_FAILURE;
}
