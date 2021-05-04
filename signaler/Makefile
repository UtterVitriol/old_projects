CFLAGS += -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal
LDLIBS += -lpthread

TARGET = signaler

$(TARGET): $(TARGET).o signaler_utils.o

test/test_all: LDLIBS += -lcheck -lm -lrt -lsubunit -pthread
test/test_all: test/test_all.o test/test.o signaler_utils.o


.PHONY: debug
debug: CFLAGS += -g
debug: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET)
	$(RM) *.o

.PHONY: profile
profile: CFLAGS += -pg
profile: $(TARGET)

.PHONY: check
check: test/test_all
	./test/test_all