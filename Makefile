
CC=gcc
CFLAGS=-O2 -pthread -Wall -Wextra -std=c11

all: ex1-bench ex2-fairness

ex1-bench: ex1_bench.c helper.h
	$(CC) $(CFLAGS) -o $@ ex1_bench.c

ex2-fairness: ex2_fairness.c helper.h ticket_sem.h
	$(CC) $(CFLAGS) -o $@ ex2_fairness.c

clean:
	rm -f ex1-bench ex2-fairness
