#include "server.h"

#include <bits/types/__sigval_t.h>
#include <bits/types/siginfo_t.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../common/net_node.h"
#include "../../lib/c_vector/c_vector.h"
#include "../../lib/c_queue/c_queue.h"
#include "../../lib/c_input_check/c_check_input.h"

#define CONCRETE_SERVER(SV) (server_internal* ) SV
#define NOT_IMPLEMENTED perror("not implemented")

typedef struct connection{
    address _conn_addr;
    socket_t _conn_sock;
    pthread_t _thr;
}connection;

typedef struct server_internal{
    net_node _common;
    c_vector* _connection_vec;
    c_queue* _new_connection;
    new_client_fun _new_c_fun;
    void* new_client_fun_context;
    pthread_t _sv_thread;
    unsigned char listening:1;
}server_internal;


static inline void
free_connection(void* conn){
    free(conn);
}

static int 
cmp_socket(void* sock1, void* sock2){
    socket_t* sock1_t = (socket_t* )sock1;
    socket_t* sock2_t = (socket_t* )sock2;

    return sock1_t == sock2_t;
}

static inline int
cmp_conn(const void* conn1, const void* conn2){
    connection* c1 = (connection *)conn1;
    address* c2 = (address*)conn2;

    return !strcmp(c1->_conn_addr._addr_str, c2->_addr_str) &&
        c1->_conn_addr.port == c2->port;
}

static inline void
print_fun(const void* e){
    server_internal* ser = CONCRETE_SERVER(e);
    printf("%s", ser->_common._addr._addr_str);
}

//private tool function
static void handler_new_client(int sig, siginfo_t *info, void* context)
{
    server_internal* sv_int = CONCRETE_SERVER(info);
    if (sv_int->_new_c_fun) {
        void* new_c = NULL;
        c_queue_pop(&sv_int->_new_connection,&new_c);
        sv_int->_new_c_fun(new_c, sv_int->new_client_fun_context);
    }   
}

static void* 
new_server_thread(void* args)
{
    server_internal* sv_int = CONCRETE_SERVER(args);
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
    const union sigval sv = {
        .sival_ptr = &sv_int,
    };

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
        printf("new client ip %s\n", client_addr_str);
        new_client._conn_sock = client_socket;
        address_init(&client_addr_imp, client_addr_str, client_addr.sin_port);
        const void* ele = c_vector_push(&sv_int->_connection_vec, &new_client);
        if(!ele){
            fprintf(stderr,"failed to save client_conn\n");
            close(client_socket);
            continue;
        }
        c_queue_push(&sv_int->_new_connection, ele, c_vector_ele_size(sv_int->_connection_vec));
        sigqueue(getpid(), SIGUSR1, sv);
    }

    sv_int->listening=0;
    return NULL;
}

static void 
close_client_connection(void *ele)
{
    if(!c_check_input_pointer(ele, "connection")) return;
    connection* c = (connection* ) ele;
    close(c->_conn_sock);
}


//public
uint8_t 
server_init(server** sv, const address* addr, const uint16_t sv_capacity)
{
    if(!c_check_input_pointer(sv, "root pointer of server")) goto exit;
    if(!c_check_input_pointer(addr, "server addr")) goto exit;

    server_internal** sv_int = (server_internal** )sv;
    const char* ip_addr_str = addr->_addr_str;
    struct c_vector_input_init in_args ={
        .capacity = sv_capacity,
        .ele_size = sizeof(connection),
        .free_fun = free_connection,
        .found_f = cmp_conn,
        .print_fun = print_fun,
    };

    const socket_t sv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (sv_socket < 0) {
        fprintf(stderr, "ERROR: failing creation of socket\n");
        goto exit;
    }
    
    const struct sockaddr_in sv_struct = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(ip_addr_str),
        .sin_port = htons(addr->port),
    };

    if (bind(sv_socket, (struct sockaddr *) &sv_struct, sizeof(sv_struct)) < 0) {
        goto close_socket;
    }

    struct sigaction sg;
    sg.sa_sigaction = handler_new_client;
    sg.sa_flags = SA_SIGINFO;
    sigemptyset(&sg.sa_mask);
    if(sigaction(SIGUSR1, &sg, NULL)) goto close_socket;

    c_vector* vec_con = c_vector_init(&in_args);
    if(!c_check_input_pointer(vec_con,"vector of connections")) goto clean_vector;

    if (!*sv_int) {
        *sv_int = calloc(1, sizeof(**sv_int));
        if (!c_check_input_pointer(*sv, "ptr alloc")) goto restore_signal;
    }
    memcpy(&(*sv_int)->_common._addr, addr, sizeof(*addr));

    (*sv_int)->_sv_thread=0;
    (*sv_int)->_common._my_socket=sv_socket;
    (*sv_int)->_connection_vec = vec_con;
    (*sv_int)->_sv_thread = 0;
    (*sv_int)->_new_connection = NULL;
    (*sv_int)->_new_c_fun = NULL;

    
    return EXIT_SUCCESS;

clean_vector:
    c_vector_free(vec_con);
restore_signal:
    if(!sigaction(SIGUSR1, NULL, NULL)) goto close_socket;
close_socket:
    close(sv_socket);
exit:
    return EXIT_FAILURE;
}

uint8_t server_start(server* sv)
{
    if(!c_check_input_pointer(sv, "server pointer")) return EXIT_FAILURE;

    server_internal* sv_int = CONCRETE_SERVER(sv);
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

uint8_t server_stop(const server* sv)
{
    if(!c_check_input_pointer(sv, "server pointer")) return EXIT_FAILURE;

    server_internal* sv_int = CONCRETE_SERVER(sv);
    if (!sv_int->_sv_thread) {
        fprintf(stderr, "ERROR: failed to stop the server, server has not yet started\n");
        return EXIT_FAILURE;
    }
    sv_int->listening=0;
    return EXIT_SUCCESS;
}

uint8_t server_kill(const server* sv)
{
    if(!c_check_input_pointer(sv, "server pointer")) return EXIT_FAILURE;
    server_internal* sv_int = CONCRETE_SERVER(sv);
    
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
    if(!c_check_input_pointer(sv, "server pointer")) return;
    
    server_internal* sv_int = CONCRETE_SERVER(sv);
    if (!sv_int->_sv_thread) {
        fprintf(stderr, "error server not started\n");
        return;
    }

    pthread_join(sv_int->_sv_thread, NULL);
    return;
}

uint8_t server_free(server* sv)
{
    if(!c_check_input_pointer(sv, "server pointer")) return EXIT_FAILURE;

    server_internal* sv_int = CONCRETE_SERVER(sv);
    server_stop(sv);
    c_vector_free(sv_int->_connection_vec);
    free(sv);

    return EXIT_SUCCESS;
}

unsigned long server_sizeof(){
    return sizeof(server_internal);
}

const char* server_addr_str(const server* sv){
    if(!c_check_input_pointer(sv, "server pointer")){
        return NULL;
    }
    server_internal* sv_int = CONCRETE_SERVER(sv);
    return sv_int->_common._addr._addr_str;
}

void server_to_string(const server* sv)
{
    if(!c_check_input_pointer(sv, "server pointer")) return ;

    server_internal* sv_int = CONCRETE_SERVER(sv);
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
    if(!c_check_input_pointer(sv, "server pointer")) return NULL;

    server_internal* sv_int = CONCRETE_SERVER(sv);
    unsigned int clien_num = c_vector_length(sv_int->_connection_vec);
    c_vector* result = net_node_vector_init(clien_num);
    void* ele = NULL;

    for (unsigned int i=0; i<clien_num; i++) {
        ele = c_vector_get_at_index(sv_int->_connection_vec, i);
        c_vector_push(&result, ele);
    }

    return result;
}

uint8_t server_set_async_new_client_action(const server* sv,new_client_fun new_c_fun, void* context)
{
    if((!c_check_input_pointer(sv, "server pointer")) || 
            (!c_check_input_pointer(new_c_fun, "new client function"))) return EXIT_FAILURE;

    server_internal* sv_int = CONCRETE_SERVER(sv);
    sv_int->_new_c_fun = new_c_fun;
    sv_int->new_client_fun_context = context;

    return EXIT_SUCCESS;
}
