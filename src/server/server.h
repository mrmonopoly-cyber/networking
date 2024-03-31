#pragma once

#include "../common/net_node.h"

typedef struct server server;

int server_init(server** sv, const address* addr, const unsigned int sv_capacity);
int server_start(server* sv);
int server_stop(const server* sv);
int server_kill(const server* sv);
void server_wait(const server* sv);
int server_free(server* sv);
int server_send(const server* sv, const address* addr);
int server_recv(const server* sv, unsigned char* buffer, unsigned int* buffer_size);
unsigned long server_sizeof();
const char* server_addr_str(const server* sv);
void server_to_string(const server* sv);
const c_vector* server_get_client_list(const server* sv);
