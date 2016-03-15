# use the GNU C Compiler
CC = gcc

# options to pass into the compiler
CFLAGS = -Wall -Werror -g -std=c99

default: mapreduce.o master.o map_worker.o reduce_worker.o walk_worker.o linkedlist.o word_freq.o
	$(CC) $(CFLAGS) -o mapreduce mapreduce.o master.o map_worker.o reduce_worker.o walk_worder.o linkedlist.o word_freq.o

# general rule for compiling c files into object files
# all c files must provide an associated header file in or

mapreduce.o: mapreduce.c mapreduce.h
	$(CC) $(CFLAGS) -c mapreduce.c

master.o: master.c master.h
	$(CC) $(CFLAGS) -c master.c

map_worker.o: map_worker.c map_worker.h
	$(CC) $(CFLAGS) -c map_worker.c

reduce_worker.o: reduce_worker.c reduce_worker.h
	$(CC) $(CFLAGS) -c reduce_worker.c

walk_worker.o: walk_worker.c walk_worker.h
	$(CC) $(CFLAGS) -c walk_worker.c

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) -c linkedlist.c

word_freq.o: word_freq.c
	$(CC) $(CFLAGS) -c word_freq.c

# TODO: Complete this
#specific_mapreduce:
	# for specific map and reduce functions provided

clean:
	rm -f *.o mapreduce
