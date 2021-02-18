CC=gcc
CFLAGS= -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -O3 -std=c11

OBJ1 = intersect

DEPS1 = $(OBJ1).c $(OBJ1).h

all: build

debug: CFLAGS += -g
debug: build

build: $(OBJ1).o
	$(CC) $(CFLAGS) $(OBJ1).o -o $(OBJ1)
	rm -f $(OBJ1).o

profile: CFLAGS += -pg
profile:$(OBJ1).o
	$(CC) $(CFLAGS) $(OBJ1).o -o $(OBJ1)
	rm -f $(OBJ1).o

clean:
	rm -f $(OBJ1)
	rm -f gmon.out

check:
	./intersect test/test1.txt test/test2.txt
	./intersect -u test/test1.txt test/test2.txt
	./intersect -l test/test1.txt test/test2.txt
	./intersect -lu test/test1.txt test/test2.txt
	./intersect -i test/test1.txt test/test2.txt
	./intersect -il test/test1.txt test/test2.txt
	./intersect -ilu test/test1.txt test/test2.txt

$(OBJ1).o : $(DEPS1)
	$(CC) -c $(CFLAGS) $(OBJ1).c
