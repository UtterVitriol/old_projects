CFLAGS += -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal
LDLIBS += -lpthread

all: listener dispatcher

test/test_all: LDLIBS += -lcheck -lm -lrt -lsubunit -pthread
test/test_all: test/test_all.o test/test_dispatcher.o test/test_listener.o 


.PHONY: debug
debug: CFLAGS += -g
debug: all

.PHONY: clean
clean:
	$(RM) dispatcher listener *.o

.PHONY: profile
profile: CFLAGS += -pg
profile: all

.PHONY: check
check: test/test_all
	./test/test_all
