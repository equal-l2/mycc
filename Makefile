CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): mycc.h

test: mycc
	./test.sh

clean:
	$(RM) mycc tmp* *.o

.PHONY: clean test
