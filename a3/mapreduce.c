#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>         // for variable args to function
#include "mapreduce.h"

/* printing to stderr conveniently in a varadic manner*/
void error(char *msg, int count, ...) {
    va_list vargs;
    va_start(vargs, count);

    char new_msg[strlen(msg) + 2];
    strncpy(new_msg, msg, strlen(msg));
    new_msg[sizeof(new_msg) - 2] = '\n';
    new_msg[sizeof(new_msg) - 1]  ='\0';

    vfprintf(stderr, new_msg, vargs);
    va_end(vargs);
}

/**
 * A function to read in the command line args and set them appropriately.
 * [-m numprocs] [-r numprocs] -d dirname
 *
 * do not assume dirname ends with a slash
 */
// int opterr - non zero, then getopt prints an error message
//
// int optopt - when getopt encounters an unknown option char or option
// char, stores it here
//
// int optind - set by getopt to the index of the next element in argv array
// to be processed after all option args processed
//
// char * optarg - set by getopt to point at the value of option arg, for
// those that accept arguments

// TODO: Init result in a simpler way using c99 partial struct initializtion
// TODO: naming members of struct MapReduceLogistics sucks, correct it
MapReduceLogistics process(int argc, char *const *argv) {
    MapReduceLogistics res = {
        .nmapworkers = 2,               // TODO: Why magic numbers?
        .nreduceworkers = 2,
        .dirname = ""
    };

    int dflag = 0;
    int throw_error = 0;

    opterr = 0;
    int output;

    while ((output = getopt(argc, argv, "m:r:d:")) != -1) {
        switch (output) {
            case 'm':
                res.nmapworkers = strtol(optarg, NULL, 10);
                break;
            case 'r':
                res.nreduceworkers = strtol(optarg, NULL, 10);
                break;
            case 'd':
                dflag = 1;
                strncpy(res.dirname, optarg, sizeof(res.dirname));
                res.dirname[sizeof(res.dirname) - 1] = '\0';
                break;
            default:
                throw_error = 1;
        }
    }

    if (!dflag || optind != argc) {
        throw_error = 1;
    }

    if (throw_error) {
        error("usage: %s [-m nmapworkers] [-r nreduceworkers] -d dirname", 1, argv[0]);
        error("\t-m nmapworkers: number of map processes (default 2)", 0);
        error("\t-r nreduceworkers: number of reduce processes (default 2)", 0);
        error("\t-d dirname: directory of files to map reduce", 0);
        exit(1);
    }

    return res;
}

int main(){
    char *path = "/Users/jcoc611/a3/group_0476/a3/texts/";
    int m = 2;
    create_master(path, m);

    return 0;
}
