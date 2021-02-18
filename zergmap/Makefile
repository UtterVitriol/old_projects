# Thank you, Mr. Echlin for the makefile lesson and this makefile!

CFLAGS += -Wall -Wno-unused-result -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -std=c11

CPPFLAGS +=  -D _GNU_SOURCE

LDLIBS += -lm

TARGET = zergmap

$(TARGET): $(TARGET).o decode.o graph.o min_heap.o queue.o

.PHONY: debug
debug: CFLAGS += -g
debug: $(TARGET)

.PHONY: build
build: $(DEPS)

.PHONY: profile
profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) *.o

.PHONY: check
check:
	chmod 755 test/test.bin
	./test/test.bin
