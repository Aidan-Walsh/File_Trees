#--------------------------------------------------------------------
# Makefile for assignment 4, part 3 implementation
# Authors: Konstantin Howard, Aidan Walsh
#--------------------------------------------------------------------

CC = gcc217

all: ft
clobber: clean
	rm -f *.o *~
clean: 
	rm -f ft

ft: dynarray.o ft_client.o ft.o node.o
	$(CC) dynarray.o ft_client.o ft.o node.o -o ft
dynarray.o: dynarray.c dynarray.h 
	$(CC) -c dynarray.c
ft_client.o: ft_client.c ft.h 
	$(CC) -c ft_client.c
ft.o: ft.c ft.h 
	$(CC) -c ft.c 
node.o: node.c node.h 
	$(CC) -c node.c 







