#include "net_node.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int check_input_pointer(const void* ptr, const char* ptr_name){
    if (!ptr) {
        fprintf(stderr, "ERROR: pointer %s is NULL\n", ptr_name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//public
int address_init(address** addr, const char* ip_addr, const unsigned short port)
{
    unsigned int alloc = 0;
    if (!*addr) {
        address* addr_new = calloc(1, sizeof(**addr));
        if (check_input_pointer(addr_new, "ip addr")) {
            return EXIT_FAILURE;
        }
        *addr = addr_new;
        alloc = 1;
    }
    (*addr)->port = 1;

    char sectors[IP_ADDR_SECTORS*SECTOR_SIZE] = {};
    unsigned char dig_counter = 0;
    unsigned char sector_num = 0;
    unsigned char sectors_cursor = 0;

    for (size_t i=0; i<strlen(ip_addr); i++) {
        sectors_cursor = (sector_num*SECTOR_SIZE)+dig_counter;
        if (i > IP_ADDR_SECTORS * SECTOR_SIZE) goto invalid_ip_address;

        if (ip_addr[i] == '.') {
            sectors[sectors_cursor]='\0';
            sector_num++;
            dig_counter=0;
            continue;
        }

        if (dig_counter > 2) goto invalid_ip_address;

        sectors[sectors_cursor] = ip_addr[i];
        dig_counter++;
    }
    
    unsigned short* cursor= (*addr)->addr_sectors;
    int temp = 0;
    for (int i=0; i<IP_ADDR_SECTORS; i++) {
        temp = atoi(&sectors[i*SECTOR_SIZE]);
        if (temp < 0 || temp > 255) goto invalid_ip_address;
        cursor[i] = temp;

    }
    (*addr)->port = port;
    memcpy((*addr)->_addr_str,ip_addr,strlen(ip_addr));
    
    return EXIT_SUCCESS;

invalid_ip_address:
    if (alloc) {
        free(*addr);
        *addr = NULL;
    }
    fprintf(stderr, "ERROR: given invalid ip address: %s\n", ip_addr);
    return -2;
}
