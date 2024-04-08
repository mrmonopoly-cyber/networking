#pragma once

#include "../common/net_node.h"
#include <stdint.h>

typedef void(*new_client_fun) (net_node* client, void* args);
typedef struct server server;

uint8_t server_init(server** sv, const address* addr, const uint16_t sv_capacity);
uint8_t server_start(server* sv);
uint8_t server_stop(const server* sv);
uint8_t server_kill(const server* sv);
void server_wait(const server* sv);
uint8_t server_free(server* sv);
uint64_t server_sizeof();
const char* server_addr_str(const server* sv);
void server_to_string(const server* sv);
const c_vector* server_get_client_list(const server* sv);
uint8_t server_set_async_new_client_action(const server* sv,new_client_fun new_c_fun, void* context);
