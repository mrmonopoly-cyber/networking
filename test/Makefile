C_FLAGS = -Wall -Wextra -g -fsanitize=address

NETWORKING_ROOT := ..

include $(NETWORKING_ROOT)/Makefile

obj_file = address.o server.o client.o net_node.o c_vector.o c_queue.o

all: server client

client:  $(obj_file) client_test.c
	gcc $(C_FLAGS) $(obj_file) client_test.c -o client

server:  $(obj_file) server_test.c
	gcc $(C_FLAGS) $(obj_file) server_test.c -o server

test_clean: 
	rm server client

clean: test_clean
