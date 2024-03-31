#pragma once
#define IP_ADDR_SECTORS 4
#define SECTOR_SIZE 4

#include "c_vector/c_vector.h"

typedef struct address{//0.0.0.0 - 255.255.255.255
    char _addr_str[IP_ADDR_SECTORS*SECTOR_SIZE];
    unsigned char addr_sectors[IP_ADDR_SECTORS];
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

