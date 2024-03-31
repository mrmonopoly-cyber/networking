C_FLAGS = -Wall -Wextra -g -fsanitize=address

all: main

main: main.c address.o server.o client.o c_vector.o net_node.o
	gcc $(C_FLAGS) address.o net_node.o c_vector.o server.o client.o main.c -o main

c_vector.o: ../../c_vector/c_vector.c
	gcc $(C_FLAGS) ../../c_vector/c_vector.c -c

net_node.o: ../net_node.c
	gcc $(C_FLAGS) ../net_node.c -c

address.o: ../address.c
	gcc $(C_FLAGS) ../address.c -c

server.o: ../server.c
	gcc $(C_FLAGS) ../server.c -c

client.o: ../client.c
	gcc $(C_FLAGS) ../client.c -c

clean: 
	rm main address.o server.o client.o c_vector.o net_node.o
