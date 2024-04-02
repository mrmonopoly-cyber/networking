#include "net_node.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

static inline int
check_null_pointer(const void* ptr, const char* ptr_name){
    if (!ptr) {
        fprintf(stderr, "ERROR: NULL pointer: %s\n",ptr_name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int 
net_cmp(const void* net1, const void* net2){
    const net_node* net_1_t = (net_node*) net1;
    const net_node* net_2_t = (net_node*) net2;

    return  
        !strcmp(net_1_t->_addr._addr_str, net_2_t->_addr._addr_str) &&
        net_1_t->_my_socket == net_2_t->_my_socket;
}

static void 
free_net(void* e){}

static void 
print_net(const void* e) {}

c_vector* 
net_node_vector_init(const unsigned int capacity)
{
    struct c_vector_input_init in_args ={
        .capacity = capacity,
        .ele_size = sizeof(net_node),
        .print_fun = print_net,
        .found_f = net_cmp,
        .free_fun = free_net,
    };


    return c_vector_init(&in_args);
    
}

uint8_t 
net_node_send(const net_node* node, const void* data, const uint16_t data_amount)
{
    if(check_null_pointer(node, "node connection")) return EXIT_FAILURE;
    if(check_null_pointer(data, "data buffer")) return EXIT_FAILURE;
    if(!data_amount) return EXIT_SUCCESS;
    

    send(node->_my_socket, &data_amount, sizeof(data_amount), 0);
    send(node->_my_socket, data, data_amount, 0);
    

    return EXIT_SUCCESS;
}

int 
net_node_recv(const net_node* node, char** buffer, unsigned int buffer_size)
{
    if(check_null_pointer(node, "node connection")) return EXIT_FAILURE;
    if(check_null_pointer(buffer, "buffer connection")) return EXIT_FAILURE;
    
    uint16_t data_amount = -1;

    printf("receiving data amount\n");
    recv(node->_my_socket, &data_amount, sizeof(data_amount), 0);

    if (!(*buffer)) {
        printf("new buffer\n");
        *buffer = calloc(data_amount+1, sizeof(**buffer));
    }else if (data_amount > buffer_size) {
        printf("reallocin buffer\n");
        *buffer = realloc(*buffer, data_amount);
        if (check_null_pointer(*buffer, "resize of buffer\n")) return EXIT_FAILURE;
    }

    printf("saving data\n");
    recv(node->_my_socket, *buffer, data_amount, 0);

    return EXIT_SUCCESS;
}
