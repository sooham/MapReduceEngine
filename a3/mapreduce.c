#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mapreduce.h"

/* printing to stderr conveniently in a varadic manner*/
// TODO: are we allowed to change the mapreduce header file
// if yes, put function prototype in there
void error(char *msg, int count, ...) {
    va_list vargs;
    va_start(vargs, count);

    char new_msg[strlen(msg) + 2];
    strncpy(new_msg, msg, sizeof(new_msg));
    new_msg[strlen(new_msg) + 1] = '\0';
    new_msg[strlen(new_msg)]  ='\n';

    vfprintf(stderr, new_msg, vargs);
    va_end(vargs);
}

/**
 * Read the command line args and set map reduce logistics
 * appropriately. The usage format is [-m numprocs] [-r numprocs] -d dirname.
 *
 * do not assume dirname ends with a slash TODO: did I assume this?
 */

MapReduceLogistics process(int argc, char *const *argv) {
    MapReduceLogistics res = {
        .nmapworkers = DEFAULT_NWORKERS,
        .nreduceworkers = DEFAULT_NWORKERS,
        .dirname = ""
    };

    int dflag = 0;
    int throw_error = 0;

    opterr = 0;       // do not let getopts throw error if missing argument
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
                // check the length of input string can include null term
                if (strlen(optarg) >= MAX_FILENAME) throw_error = 1;
                strncpy(res.dirname, optarg, sizeof(res.dirname));
                res.dirname[sizeof(res.dirname) - 1] = '\0';
                // TODO: concatenating end of file '/' done in list worker
                break;
            default:
                throw_error = 1;
        }
    }

    if (!dflag || res.nmapworkers <= 0 || res.nreduceworkers <= 0 || optind != argc) {
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

int main(int argc, char *argv[]){
    MapReduceLogistics out = process(argc, argv);
    printf("nmapworkers = %d, nreduceworker = %d, dirname = %s\n", out.nmapworkers, out.nreduceworkers, out.dirname);
    // create_master(out.dirname, out.nmapworkers, out.nreduceworkers);

    return 0;
}
