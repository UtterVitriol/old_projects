IDIRS = SRC/
IDIRH = SRC/
BDIR = BIN/

CC=gcc
CFLAGS=-Wno-unused-parameter -Wall -Werror -Wextra -Wpedantic -Wwrite-strings -Wvla -Wfloat-equal

DEPS1 = $(IDIRS)battleship_offline.c $(IDIRH)battleship.h $(IDIRH)battleship_online.h
DEPS2 = $(IDIRS)battleship_online.c $(IDIRH)battleship.h $(IDIRH)battleship_online.h
DEPS3 = $(IDIRS)ascii_art.c $(IDIRH)ascii_art.h

OBJ1 = $(BDIR)battleship_offline.o
OBJ2 = $(BDIR)battleship_online.o
OBJ3 = $(BDIR)ascii_art.o

OBJ = $(OBJ1) $(OBJ2) $(OBJ3)

build: $(OBJ)
	$(CC) $(CFLAGS) -o $(BDIR)battleship $(OBJ)
	rm $(OBJ)
	
buildAll: $(OBJ)
	$(CC) $(CFLAGS) -o $(BDIR)battleship $(OBJ)

debug: $(OBJ)
	$(CC) $(CFLAGS) -g -o $(BDIR)battleship $(OBJ)

clean:
	rm $(OBJ)

cleanAll:
	rm $(OBJ) $(BDIR)battleship

$(OBJ1) : $(DEPS1)
	gcc -c $(IDIRS)battleship_offline.c -o $@

$(OBJ2) : $(DEPS2)
	gcc -c $(IDIRS)battleship_online.c -o $@

$(OBJ3): $(DEPS3)
	gcc -c $(IDIRS)ascii_art.c -o $@

