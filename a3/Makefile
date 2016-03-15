# TODO: specific mapreduce targets
# default not complete
# use the GNU C Compiler
CC=gcc

# options to pass into the compiler
# I flag tells gcc to search for header files in current dir
# c flag tells gcc to generate .o file
CFLAGS= -c -I -Wall -Werror -g -std=c99

all: default

default: mapreduce.o master.o map_worker.o reduce_worker.o
	$(CC) mapreduce.o master.o map_worker.o reduce_worker.o -o mapreduce

mapreduce.o: mapreduce.c
	$(CC) $(CFLAGS) mapreduce.c

master.o: master.c
	$(CC) $(CFLAGS) master.c

map_worker.o: map_worker.c
	$(CC) $(CFLAGS) map_worker.c

reduce_worker.o: reduce_worker.c
	$(CC) $(CFLAGS) reduce_worker.c

specific_mapreduce:
	# for specific map and reduce functions provided

clean:
	rm -f *o mapreduce
