C_FLAGS = -Wall -Wextra -g -fsanitize=address

networking_src_path := $(or $(NETWORKING_ROOT), .)

TREE_INTERFACE_ROOT:= $(networking_src_path)/lib/generic_tree_interface
C_VECTOR_ROOT := $(networking_src_path)/lib/c_vector


include $(C_VECTOR_ROOT)/Makefile
include $(TREE_INTERFACE_ROOT)/Makefile

server.c = $(networking_src_path)/src/server/server.c
client.c = $(networking_src_path)/src/client/client.c
address.c = $(networking_src_path)/src/common/address.c
net_node.c = $(networking_src_path)/src/common/net_node.c
c_vector.o = $(C_VECTOR_ROOT)/c_vector.o 

all: net_node.o address.o client.o server.o

net_node.o: $(net_node.c)
	gcc $(C_FLAGS) $(net_node.c) -c

address.o: $(address.c)
	gcc $(C_FLAGS) $(address.c) -c

server.o: $(server.c) 
	gcc $(C_FLAGS) $(server.c) -c

client.o: $(client.c)
	gcc $(C_FLAGS) $(client.c) -c

networking_clean: 
	rm address.o server.o client.o net_node.o 

clean: networking_clean
