CC = gcc
OPT = -O3 -m32
#OPT = -g -m32
WARN = -Wall
CFLAGS = $(OPT) $(WARN)
#$(INC) $(LIB)

# List all your .cc files here (source files, excluding header files)
SIM_SRC = sim_cache.c

# rule for making sim_cache

sim_cache: $(SIM_OBJ)
	$(CC) -o sim_cache $(CFLAGS) $(SIM_SRC) -lm


# generic rule for converting any .c file to any .o file
 
.c.o:
	$(CC) $(CFLAGS)  -c $*.c


# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o sim_cache


# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o
