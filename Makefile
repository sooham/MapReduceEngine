# use the GNU C Compiler
CC = gcc

# debug
DEBUG = -g

# compilation flags
CFLAGS = -Wall -Werror -std=c99 -c $(DEBUG)

# linker flags
LFLAGS = -Wall -Werror -std=c99 $(DEBUG)

# object files
OBJS = mapreduce.o utils.o hash.o linkedlist.o lister.o mapper.o master.o reducer.o


default: $(OBJS) word_freq.o
	$(CC) $(LFLAGS) $(OBJS) word_freq.o -o mapreduce

mapreduce.o: mapreduce.c mapreduce.h
	$(CC) $(CFLAGS) mapreduce.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) utils.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) hash.c

master.o: master.c master.h
	$(CC) $(CFLAGS) master.c

mapper.o: mapper.c mapper.h
	$(CC) $(CFLAGS) mapper.c

reducer.o: reducer.c reducer.h
	$(CC) $(CFLAGS) reducer.c

lister.o: lister.c lister.h
	$(CC) $(CFLAGS) lister.c

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) linkedlist.c

word_freq.o: word_freq.c
	$(CC) $(CFLAGS) word_freq.c

# dummy flag used for providing a specified map reduce function source file
# as command line arguments to make
# Usage: "make specific FILE=filename" (without .c extension)
specific: $(OBJS) $(FILE).o
	$(CC) $(LFLAGS) $(OBJS) $(FILE).o -o mapreduce

$(FILE).o : $(FILE).c
	$(CC) $(CFLAGS) $(FILE).c

# to clean .out files
clout:
	rm -f *.out

# dummy cleaning flag
clean: clout
	rm -rf *.o mapreduce *.swp *.dSYM

