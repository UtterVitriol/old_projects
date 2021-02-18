CC=gcc
CFLAGS= -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal

OBJ1 = poly
OBJ2 = test/test

DEPS1 = $(OBJ1).c $(OBJ1).h
DEPS2 = $(OBJ2).c

all: build

debug: CFLAGS += -g
debug: build

check: $(OBJ2).o
check: debug
	$(CC) $(CFLAGS) -o $(OBJ2) $(OBJ2).o -I ./ -L ./ -l$(OBJ1) -lm
	rm -f $(OBJ2).o
	./test/test

build: $(OBJ1).o
	ar -crs lib$(OBJ1).a $(OBJ1).o
	rm -f $(OBJ1).o

clean:
	rm -f lib$(OBJ1).a
	rm -f $(OBJ2)

$(OBJ1).o : $(DEPS1)
	$(CC) -c $(CFLAGS) $(OBJ1).c

$(OBJ2).o : $(DEPS2)
	$(CC) -c $(CFLAGS) $(OBJ2).c -o $(OBJ2).o
