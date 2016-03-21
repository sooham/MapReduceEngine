/*
 * Mapreduce parses input command line arguments and runs MapReduce
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mapreduce.h"
#include "utils.h"
#include "master.h"

/**
 * Read the command line arguments and set MapReduce logistics
 * appropriately.
 * Usage format is mapreduce [-m numprocs] [-r numprocs] -d dirname.
 *
 * @param argc      command line argument count
 * @param argv      command line argument vector
 * @exit            if incorrect type of arguments passed
 * @return          MapReduceLogistics contains processed inputs
 */

MapReduceLogistics process(int argc, char *const *argv) {
    MapReduceLogistics res = {
        .nmapworkers = DEFAULT_NWORKERS,
        .nreduceworkers = DEFAULT_NWORKERS,
        .dirname = NULL
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
                res.dirname = malloc(sizeof(char) * (strlen(optarg) + 2));
                strncpy(res.dirname, optarg, strlen(optarg) + 2);

                // concatenating '/' if needed
                if (res.dirname[strlen(res.dirname) - 1] != '/') {
                    res.dirname[strlen(res.dirname)] = '/';
                    res.dirname[strlen(res.dirname) + 1] = '\0';
                }
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

/*
 * Main function to run MapReduce */
int main(int argc, char *argv[]) {
    MapReduceLogistics out = process(argc, argv);
    create_master(out.dirname, out.nmapworkers, out.nreduceworkers);
    // TODO: free the dynamic memory allocated in out
    return 0;
}
