#include "net_node.h"
#include <string.h>

static int net_cmp(const void* net1, const void* net2){
    const net_node* net_1_t = (net_node*) net1;
    const net_node* net_2_t = (net_node*) net2;

    return  
        !strcmp(net_1_t->_addr._addr_str, net_2_t->_addr._addr_str) &&
        net_1_t->_my_socket == net_2_t->_my_socket;
}

static void free_net(void* e){}

static void print_net(const void* e) {}

c_vector* net_node_vector_init(const unsigned int capacity)
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
