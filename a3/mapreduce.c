/*
 * Mapreduce parses input command line arguments and starts the
 * Master process to run MapReduce.
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "mapreduce.h"
#include "master.h"
#include "utils.h"

/**
 * Read the command line arguments and set MapReduce logistics
 * appropriately.
 * Usage format is "mapreduce [-m numprocs] [-r numprocs] -d dirname".
 *
 * @param argc      command line argument count
 * @param argv      command line argument vector
 * @exit            1 if incorrect type of arguments passed
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
                // we though MAX_FILENAME (32 bytes) was too short
                // a length for malloc
                safe_malloc(
                            (void **) &(res.dirname),
                            sizeof(char) * (strlen(optarg) + 2)
                            );
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

    if (!dflag || res.nmapworkers <= 0 || res.nreduceworkers <= 0 ||
     optind != argc) {
        throw_error = 1;
    }

    if (throw_error) {
        safe_fprintf(
            stderr,
            "usage: %s [-m nmapworkers] [-r nreduceworkers] -d dirname\n",
            argv[0]);
        safe_fprintf(stderr,
            "\t-m nmapworkers: number of map processes (default 2)\n"
            );
        safe_fprintf(stderr,
         "\t-r nreduceworkers: number of reduce processes (default 2)\n");
        safe_fprintf(stderr,
         "\t-d dirname: directory of files to map reduce\n");

        exit(1);
    }

    return res;
}

/*
 * Creates Master process and runs MapReduce.
 */
int main(int argc, char *argv[]) {
    MapReduceLogistics out = process(argc, argv);
    create_master(out.dirname, out.nmapworkers, out.nreduceworkers);
    free(out.dirname);
    fprintf(stderr, "%d is ending\n", getpid());
    return 0;
}

