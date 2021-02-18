CC=gcc
CFLAGS= -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -O3 -std=c11

OBJ1 = maze
OBJ2 = graph
OBJ3 = queue
OBJ4 = min_heap

DEPS1 = $(OBJ1).c $(OBJ1).h $(OBJ2).h
DEPS2 = $(OBJ2).c $(OBJ2).h $(OBJ1).h $(OBJ3).h $(OBJ4).h
DEPS3 = $(OBJ3).c $(OBJ3).h
DEPS4 = $(OBJ4).c $(OBJ4).h

OBJ = $(OBJ1).o $(OBJ2).o $(OBJ3).o $(OBJ4).o

all: build

debug: CFLAGS += -g
debug: build

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OBJ1)
	rm -f *.o

profile: CFLAGS += -pg
profile:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OBJ1)
	rm -f *.o

clean:
	rm -f $(OBJ1)
	rm -f *.o
	rm -f gmon.out

check:
	./maze test/map00.txt > test/map00_test.txt
	diff test/map00_test.txt test/map00_solution.txt
	rm -f test/map00_test.txt
	./maze test/map01.txt > test/map01_test.txt
	diff test/map01_test.txt test/map01_solution.txt
	rm -f test/map01_test.txt
	./maze test/map02.txt > test/map02_test.txt
	diff test/map02_test.txt test/map02_solution.txt
	rm -f test/map02_test.txt
	./maze test/map03.txt > test/map03_test.txt
	diff test/map03_test.txt test/map03_solution.txt
	rm -f test/map03_test.txt
	./maze test/map04.tiny.txt > test/map04_test.txt
	diff test/map04_test.txt test/map04_solution.txt
	rm -f test/map04_test.txt
	./maze test/map05.huge.txt > test/map05_test.txt
	diff test/map05_test.txt test/map05_solution.txt
	rm -f test/map05_test.txt
	./maze -d test/map_d.txt > test/map_d_test.txt
	diff test/map_d_test.txt test/map_d_solution.txt
	rm -f test/map_d_test.txt
	./maze -w test/map_w.txt > test/map_w_test.txt
	diff test/map_w_test.txt test/map_w_solution.txt
	rm -f test/map_w_test.txt
	./maze -wdD test/map_wdD.txt > test/map_wdD_test.txt
	diff test/map_wdD_test.txt test/map_wdD_solution.txt
	rm -f test/map_wdD_test.txt



$(OBJ1).o : $(DEPS1)
	$(CC) -c $(CFLAGS) $(OBJ1).c

$(OBJ2).o : $(DEPS2)
	$(CC) -c $(CFLAGS) $(OBJ2).c

$(OBJ3).o : $(DEPS3)
	$(CC) -c $(CFLAGS) $(OBJ3).c

$(OBJ4).o : $(DEPS4)
	$(CC) -c $(CFLAGS) $(OBJ4).c
