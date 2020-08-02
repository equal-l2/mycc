CFLAGS=-std=c11 -g -static

mycc: main.c
	$(CC) -o $@ $^

test: mycc
	./test.sh

clean:
	$(RM) mycc tmp*

.PHONY: clean test
