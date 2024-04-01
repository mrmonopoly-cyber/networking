#pragma once
#include <stdint.h>
#define IP_ADDR_SECTORS 4
#define SECTOR_SIZE 4

#include "../../lib/c_vector/c_vector.h"

typedef struct address{//0.0.0.0 - 255.255.255.255
    char _addr_str[IP_ADDR_SECTORS*SECTOR_SIZE];
    union  {
        unsigned char addr_sectors[IP_ADDR_SECTORS];
        unsigned int addr_compact;  
    };
    unsigned short port;
}address;

typedef int socket_t;
typedef struct net_node{
    address _addr;
    socket_t _my_socket;
}net_node;

//connection
int address_init(address** addr, const char* ip_addr, const unsigned short port);
#define address_free(A) free(A);

//net_node
c_vector* net_node_vector_init(const unsigned int capacity);
uint8_t net_node_send(const net_node* addr, const void* data, const uint16_t data_amount);
