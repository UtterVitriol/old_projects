CC=gcc
CFLAGS= -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -O3 -std=c11

OBJ1 = ticker

DEPS1 = $(OBJ1).c $(OBJ1).h

all: build

debug: CFLAGS += -g
debug: build

build: $(OBJ1).o
	$(CC) $(CFLAGS) $(OBJ1).o -o $(OBJ1) -lm
	rm -f $(OBJ1).o

profile: CFLAGS += -pg
profile:$(OBJ1).o
	$(CC) $(CFLAGS) $(OBJ1).o -o $(OBJ1) -lm
	rm -f $(OBJ1).o

clean:
	rm -f $(OBJ1)
	rm -f gmon.out

check:
	./ticker test/test_seed.txt < test/test_update.txt

$(OBJ1).o : $(DEPS1)
	$(CC) -c $(CFLAGS) $(OBJ1).c
