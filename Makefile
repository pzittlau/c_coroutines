CC = gcc
CFLAGS = -Wall -Wextra -g -O3 -march=native

all: coroutines_example coroutines_bench

coroutines_example: src/example.c src/coroutines.c
	$(CC) $(CFLAGS) $^ -o coroutines_example

coroutines_bench: src/bench.c src/coroutines.c
	$(CC) $(CFLAGS) $^ -o coroutines_bench

clean:
	rm -f coroutines_example coroutines_bench

run: coroutines_example
	./coroutines_example

run_bench: coroutines_bench
	./coroutines_bench
