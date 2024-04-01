#pragma once

#include "../common/net_node.h"
#include <stdint.h>

typedef struct server server;

uint8_t server_init(server** sv, const address* addr, const uint16_t sv_capacity);
uint8_t server_start(server* sv);
uint8_t server_stop(const server* sv);
uint8_t server_kill(const server* sv);
void server_wait(const server* sv);
uint8_t server_free(server* sv);
uint8_t server_recv(const server* sv, void* buffer, uint16_t* buffer_size);
uint64_t server_sizeof();
const char* server_addr_str(const server* sv);
void server_to_string(const server* sv);
const c_vector* server_get_client_list(const server* sv);
const net_node* server_async_wait_new_connection(const server* sv);
