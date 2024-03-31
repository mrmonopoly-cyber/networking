#include "server.h"

#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../common/net_node.h"
#include "../common/c_vector/c_vector.h"

typedef struct server_internal{
    net_node _common;
    c_vector* _connection_vec;
    pthread_t _sv_thread;
    unsigned char listening:1;
}server_internal;

typedef struct connection{
    address _conn_addr;
    socket_t _conn_sock;
    pthread_t _thr;
}connection;


static inline int
check_null_pointer(const void* ptr, const char* ptr_name){
    if (!ptr) {
        fprintf(stderr, "ERROR: NULL pointer: %s\n",ptr_name);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static inline void
free_connection(void* conn){
    free(conn);
}

static inline int
cmp_conn(const void* conn1, const void* conn2){
    connection* c1 = (connection *)conn1;
    connection* c2 = (connection *)conn2;

    return c1->_conn_sock == c2->_conn_sock && 
        c1->_conn_addr.addr_sectors == c2->_conn_addr.addr_sectors &&
        c1->_conn_addr.port == c2->_conn_addr.port;
}

static inline void
print_fun(const void* e){
    server_internal* ser = (server_internal *) e;
    printf("%s", ser->_common._addr._addr_str);
}

static void* 
new_server_thread(void* args)
{
    server_internal* sv_int = (server_internal* )args;
    unsigned int _my_socket = sv_int->_common._my_socket;
    struct sockaddr_in client_addr;
    unsigned int client_addr_size = sizeof(client_addr);
    int client_socket = -1;
    char client_addr_str[IP_ADDR_SECTORS*SECTOR_SIZE]= {0};
    connection new_client ={
        ._thr = 0,
        ._conn_sock = -1,
    };
    address* client_addr_imp = &new_client._conn_addr;

    listen(_my_socket, c_vector_capacity(sv_int->_connection_vec));

    sv_int->listening=1;
    while (sv_int->listening) {
        printf("waiting for connections\n");
        client_socket = accept(_my_socket,(struct sockaddr*) &client_addr, &client_addr_size);
        if (client_socket<0) {
            fprintf(stderr, "ERROR: failed to accept a client, skipping connection\n");
            continue;
        }
        if(!inet_ntop(AF_INET, &client_addr, client_addr_str, sizeof(client_addr_str))){
            fprintf(stderr, "ERROR: failed to convert ip address of client, skipping connection\n");
            close(client_socket);
            continue;
        }
        new_client._conn_sock = client_socket;
        address_init(&client_addr_imp, client_addr_str, client_addr.sin_port);
        int err = c_vector_push(&sv_int->_connection_vec, &new_client);
        if(err <0){
            fprintf(stderr,"failed to save client_conn: erroro %d\n",err);
            close(client_socket);
            continue;
        }


    }

    sv_int->listening=0;
    return NULL;
}

static void close_client_connection(void *ele)
{
    if(check_null_pointer(ele, "connection")) return;
    connection* c = (connection* ) ele;
    close(c->_conn_sock);
}

//public
int server_init(server** sv, const address* addr, const unsigned int sv_capacity)
{
    server_internal** sv_int = (server_internal** )sv;
    unsigned int alloc = 0;
    const char* ip_addr_str = addr->_addr_str;
    struct c_vector_input_init in_args ={
        .capacity = sv_capacity,
        .ele_size = sizeof(connection),
        .free_fun = free_connection,
        .found_f = cmp_conn,
        .print_fun = print_fun,
    };

    if(check_null_pointer(sv, "root pointer of server")){
        goto exit;
    }
    if (!*sv) {
        *sv = calloc(1, sizeof(**sv_int));
        if (check_null_pointer(*sv, "ptr alloc")) {
            goto exit;
        }
        alloc=1;
    }
    memcpy(&(*sv_int)->_common._addr, addr, sizeof(*addr));
    const socket_t sv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (sv_socket < 0) {
        fprintf(stderr, "ERROR: failing creation of socket\n");
        goto free_mem;
    }
    
    const struct sockaddr_in sv_struct = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(ip_addr_str),
        .sin_port = htons(addr->port),
    };

    if (bind(sv_socket, (struct sockaddr *) &sv_struct, sizeof(sv_struct)) < 0) {
        goto close_socket;
    }

    c_vector* vec_con = c_vector_init(&in_args);
    if(check_null_pointer(vec_con,"vector of connections")){
        goto close_socket;
    }

    (*sv_int)->_sv_thread=0;
    (*sv_int)->_common._my_socket=sv_socket;
    (*sv_int)->_connection_vec = vec_con;
    (*sv_int)->_sv_thread = 0;
    
    return EXIT_SUCCESS;

close_socket:
    close(sv_socket);
free_mem:
    if (alloc) {
        free(*sv);
        *sv = NULL;
    }
exit:
    return EXIT_FAILURE;
}

int server_start(server* sv)
{
    server_internal* sv_int = (server_internal *) sv;
    pthread_attr_t t_args;
    if (!sv_int->_sv_thread) {
        pthread_attr_init(&t_args);
        if(pthread_create(&sv_int->_sv_thread, &t_args, *new_server_thread, sv_int)){
            fprintf(stderr, "ERROR: failed to start the server\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    
    if(!pthread_kill(sv_int->_sv_thread, SIGCONT)){
        fprintf(stderr, "ERROR: failed to resume the sever\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
int server_stop(const server* sv)
{
    server_internal* sv_int = (server_internal *) sv;
    if (!sv_int->_sv_thread) {
        fprintf(stderr, "ERROR: failed to stop the server, server has not yet started\n");
        return EXIT_FAILURE;
    }
    sv_int->listening=0;
    return EXIT_SUCCESS;
}

int server_kill(const server* sv)
{
    server_internal* sv_int = (server_internal* ) sv;
    if(check_null_pointer(sv_int, "server")) return EXIT_FAILURE;
    
    if (!sv_int->_sv_thread) {
        fprintf(stderr, "ERROR: server not yet stared, cannot kill it\n");
        return EXIT_FAILURE;
    }
    
    void* ele = NULL;
    unsigned int n_client = c_vector_length(sv_int->_connection_vec);
    printf("closing %d client connections\n", n_client);
    for (unsigned int i =0;i< n_client; i++) {
        ele = c_vector_get_at_index(sv_int->_connection_vec, i);
        close_client_connection(ele);
    }

    close(sv_int->_common._my_socket);
    pthread_kill(sv_int->_sv_thread, SIGTERM);
    server_wait(sv);

    return EXIT_SUCCESS;
}

void server_wait(const server* sv)
{
    if(check_null_pointer(sv, "server")) return;
    
    server_internal* sv_int = (server_internal* ) sv;
    if (!sv_int->_sv_thread) {
        fprintf(stderr, "error server not started\n");
        return;
    }

    pthread_join(sv_int->_sv_thread, NULL);
    return;
}

int server_send(const server* sv, const address* addr);
int server_recv(const server* sv, unsigned char* buffer, unsigned int* buffer_size);
int server_free(server* sv)
{
    server_internal* sv_int = (server_internal* )sv;
    server_stop(sv);
    c_vector_free(sv_int->_connection_vec);
    free(sv);

    return EXIT_SUCCESS;
}

unsigned long server_sizeof(){
    return sizeof(server_internal);
}

const char* server_addr_str(const server* sv){
    if(check_null_pointer(sv, "server")){
        return NULL;
    }
    server_internal* sv_int = (server_internal* ) sv;
    return sv_int->_common._addr._addr_str;
}

void server_to_string(const server* sv)
{
    server_internal* sv_int = (server_internal* )sv;
    printf("common:\n");
    printf("socket: %d, address: %s\n",sv_int->_common._my_socket, sv_int->_common._addr._addr_str);
    printf("vector: ");
    c_vector_to_string(sv_int->_connection_vec);
    printf("thread: %ld\n", sv_int->_sv_thread);
    if (sv_int->listening) {
        printf("listening!\n");
    }else {
        printf("not listening!\n");
    }
}

const c_vector* server_get_client_list(const server* sv)
{
    if(check_null_pointer(sv, "server"))return NULL;

    server_internal* sv_int = (server_internal* )sv;
    unsigned int clien_num = c_vector_length(sv_int->_connection_vec);
    c_vector* result = net_node_vector_init(clien_num);
    void* ele = NULL;

    for (unsigned int i=0; i<clien_num; i++) {
        ele = c_vector_get_at_index(sv_int->_connection_vec, i);
        c_vector_push(&result, ele);
    }

    return result;
}
