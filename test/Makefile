C_FLAGS = -Wall -Wextra -g -fsanitize=address

NETWORKING_ROOT := ..

include $(NETWORKING_ROOT)/Makefile

obj_file = main.c address.o server.o client.o net_node.o c_vector.o

all: main

main:  $(obj_file)
	gcc $(C_FLAGS) $(obj_file) -o main

test_clean: 
	rm main 

clean: test_clean
