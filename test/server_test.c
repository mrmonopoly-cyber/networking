#include "../networking.h"
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void new_c_fun(net_node* client)
{
    printf("new client is: %s\n", client->_addr._addr_str);
}

void*
input_manager(void* args){
    server* sv = (server* ) args;
    
    const net_node* new_client = NULL;
    char buffer[2] = {};
    printf("press any key to close the server\n");

    // char mex[] = "hello\n";
    // char* mex_recv = calloc(1024, 1);
    char* mex_recv = NULL;
    
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        fflush(stdin);
        if (buffer[0] != 0) {
            printf("killing server\n");
            server_kill(sv);
            break;
        }
    }

    free(mex_recv);

    return NULL;
}

int main()
{
    char ip[] = "127.0.0.1";
    unsigned short port = 8080;
    address *c_ptr = NULL;
    server* new_server = NULL;
    
    if (address_init(&c_ptr,ip, port)){
        printf("failed to create address, ptr %ld\n",(unsigned long) c_ptr);
        return -1;
    }
    

    assert(c_ptr);
    printf("original ip addr and port: %s : %u\n", ip,port);
    printf("sector 1: %u\n", c_ptr->addr_sectors[0]);
    printf("sector 2: %u\n", c_ptr->addr_sectors[1]);
    printf("sector 3: %u\n", c_ptr->addr_sectors[2]);
    printf("sector 3: %u\n", c_ptr->addr_sectors[3]);
    printf("port: %u\n", c_ptr->port);

    if(server_init(&new_server, c_ptr, 5)){
        fprintf(stderr, "server init failed\n");
        goto clean_addr;
    }
    server_start(new_server);
    server_to_string(new_server);
    // printf("ip addr from struct: %s\n", server_addr_str(new_server));

    server_set_async_new_client_action(new_server, new_c_fun);
    pthread_t p;
    pthread_attr_t inf;
    pthread_attr_init(&inf);
    pthread_create(&p, &inf, input_manager, new_server);
    
    pthread_join(p, NULL);
    server_wait(new_server);
    server_free(new_server);
clean_addr:
    address_free(c_ptr);
    return 0;
}
