all: encode.o chksm.o decode.c
	gcc -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -o encode encode.o chksm.o
	gcc -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -o decode decode.c

debug:	encode.o chksm.o decode.c
	gcc -g -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -o decode decode.c
	gcc -g -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -o encode encode.o chksm.o


encode.o : encode.c structs.h
	gcc -c encode.c

chksm.o : chksm.c chksm.h structs.h
	gcc -c chksm.c

clean:
	rm decode encode encode.o chksm.o

