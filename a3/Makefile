# default not complete
# use the GNU C Compiler
CC=gcc

# options to pass into the compiler
# c flag tells gcc to generate .o file
CFLAGS=-c -Wall -Werror -g -std=c99

all: default

default: mapreduce.o master.o map_worker.o reduce_worker.o walk_worker.o linkedlist.o word_freq.o
	$(CC) -o mapreduce mapreduce.o master.o map_worker.o reduce_worker.o walk_worder.o linkedlist.o word_freq.o

# general rule for compiling c files into object files
# all c files must provide an associated header file in or

mapreduce.o: mapreduce.c mapreduce.h
	$(CC) $(CFLAGS) mapreduce.c

master.o: master.c master.h
	$(CC) $(CFLAGS) master.c

map_worker.o: map_worker.c map_worker.h
	$(CC) $(CFLAGS) map_worker.c

reduce_worker.o: reduce_worker.c reduce_worker.h
	$(CC) $(CFLAGS) reduce_worker.c

walk_worker.o: walk_worker.c walk_worker.h
	$(CC) $(CFLAGS) walk_worker.c

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) linkedlist.c

word_freq.o: word_freq.c
	$(CC) $(CFLAGS) word_freq.c

# TODO: Complete this
#specific_mapreduce:
	# for specific map and reduce functions provided

clean:
	rm -f *.o mapreduce
