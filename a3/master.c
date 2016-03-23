/*
 Master process orchestrates the execution of the Lister,
 Mapper and Reducer.
*/

#include <stdlib.h>

#include "hash.h"
#include "lister.h"
#include "mapper.h"
#include "mapreduce.h"
#include "master.h"
#include "reducer.h"
#include "utils.h"

// global variable
PipeSet master_pipes = {
    .m = 0,
    .r = 0,
    .from_mapper = NULL,
    .to_mapper = NULL,
    .to_reducer = NULL
};



/**
 * Reads filenames located at dirname from stdin (sent by lister)
 * and distributes them evenly to mappers.
 *
 * @param dirname                   directory containing the input files.
 * @exit                            1 if error
 */
void distribute_files(char *dirname) {
    int m = master_pipes.m;

    int current_worker = 0;         // index of map worker to assign

    char filename[MAX_FILENAME];    // read filename

    // Read file names from lister
    // Send to map workers uniformly
    while (scanf("%s", filename) != EOF) {
        // Send to mapper
        safe_write(
            master_pipes.to_mapper[current_worker],
            dirname,
            strlen(dirname) * sizeof(char)
        );
        safe_write(
            master_pipes.to_mapper[current_worker],
            filename,
            strlen(filename) * sizeof(char)
        );
        safe_write(master_pipes.to_mapper[current_worker], " ", sizeof(char));

        // Distribute uniformly
        current_worker++;
        if (current_worker + 1 > m) {
            current_worker = 0;
        }
    }
    // all files have been given to mappers
    // close all to mapper pipes
    for (int i = 0; i < m; i++) {
        safe_close(master_pipes.to_mapper[i]);
    }
}

/*
 * Read key value Pairs from mappers and
 * assigns by keys to reducer using hash function.
 *
 * @exit                            1 if error
 */
void route_mapped_pairs() {
    int m = master_pipes.m;
    int r = master_pipes.r;

    // read the Pairs from mappers
    Pair read_pair;                 // Pair to read
    fd_set from_mapper_set;         // using select to avoid blocking

    // we need to keep track of closed pipes by index, 1 means closed
    int num_closed_pipes = 0;
    int closed_pipes[m];
    memset(closed_pipes, 0, m);     // set all elements to 0

    do {
        // reset the fdset, excluding closed pipes
        FD_ZERO(&from_mapper_set);
        for (int i = 0; i < m; i++) {
            if (closed_pipes[i] != 1) {
                FD_SET(master_pipes.from_mapper[i], &from_mapper_set);
            }
        }

        // select the pipes ready to read
        if (safe_select(FD_SETSIZE, &from_mapper_set, NULL, NULL)) {
            // Process ready pipes
            for (int j = 0; j < m; j++) {
                if (closed_pipes[j] != 1 &&
                    FD_ISSET(master_pipes.from_mapper[j], &from_mapper_set)) {
                    // read Pair from pipe
                    int read_result = safe_read(
                                            master_pipes.from_mapper[j],
                                            &read_pair,
                                            sizeof(Pair));

                    if (read_result == 0) {
                        closed_pipes[j] = 1;
                        num_closed_pipes++;
                    } else {
                        // send to reduce worker
                        // hash function is uniform, see hash.c for more info
                        int reduce_id = hash(read_pair.key) % r;
                        safe_write(master_pipes.to_reducer[reduce_id],
                                                     &read_pair, sizeof(Pair));
                    }
                }
            }
        }
    } while (num_closed_pipes < m); // while pipes are open to read

    // all reducers have been written to
    // and all mappers have been read

    // Close mapper->master pipes.
    for (int i = 0; i < m; i++) {
        safe_close(master_pipes.from_mapper[i]);
    }

    // close reduce pipes
    for (int i = 0; i < r; i++) {
        safe_close(master_pipes.to_reducer[i]);
    }
}

/**
 * Creates m map workers ready for use.
 *
 * @exit                1 if error
 */
void create_mappers() {
    int m = master_pipes.m;
    int r = master_pipes.r;
    master_pipes.to_mapper = malloc(sizeof(int) * m);
    master_pipes.from_mapper = malloc(sizeof(int) * m);

    // Fork indicator
    pid_t pid;

    // Fork m times
    // connect two pipes with each child
    // one master->mapper pipe to transfer filenames
    // one mapper->master pipe to transfer mapped key value Pairs
    for (int i = 0; i < m; i++) {
        // Create the master->mapper pipe
        int to_mapper_pipe[2];
        safe_pipe(to_mapper_pipe);

        // Create the mapper->master pipe
        int from_mapper_pipe[2];
        safe_pipe(from_mapper_pipe);

        // fork into map worker
        pid = safe_fork();
        if (pid == 0) {
            // mapper
            safe_close(to_mapper_pipe[WRITE_END]);

            // route stdin from pipe master->mapper
            safe_dup2(to_mapper_pipe[READ_END], STDIN_FILENO);
            safe_close(to_mapper_pipe[READ_END]);

            safe_close(from_mapper_pipe[READ_END]);

            // route stdout to pipe mapper->master
            safe_dup2(from_mapper_pipe[WRITE_END], STDOUT_FILENO);
            safe_close(from_mapper_pipe[WRITE_END]);

            // i-1 pipes to sibling mappers exist in this child, close them
            for (int j = 0; j < i; j++) {
                safe_close(master_pipes.to_mapper[j]);
                safe_close(master_pipes.from_mapper[j]);
            }

            // close all the pipes to reducers in mapper child process
            for (int j = 0; j < r; j++) {
                safe_close(master_pipes.to_reducer[j]);
            }

            // don't spawn children for child
            break;
        } else {
            // master
            // Store master->mapper pipe
            safe_close(to_mapper_pipe[READ_END]);
            master_pipes.to_mapper[i] = to_mapper_pipe[WRITE_END];

            // Store mapper->master pipe
            safe_close(from_mapper_pipe[WRITE_END]);
            master_pipes.from_mapper[i] = from_mapper_pipe[READ_END];
        }
    }
    // Finished creating map workers

    if (pid == 0) {
        // continuing after break from for loop
        // mapper blocks trying to read the Pairs from its stdin
        map_digest_files();

        // free malloced memory in child process
        free(master_pipes.from_mapper);
        free(master_pipes.to_mapper);
        free(master_pipes.to_reducer);
    }
}



/*
 * Creates m mappers and r reducers ready to use.
 *
 * @param  dirname      directory containing the input files.
 * @exit                1 if error
 */
void create_workers(char *dirname) {
    int r = master_pipes.r;
    master_pipes.to_reducer = malloc(sizeof(int) * r);

    // fork indicator
    pid_t pid;

    // fork r times for reducers
    // make one master->reducer pipe to provide mapped keys
    for (int i = 0; i < r; i++) {
        // Create the master->reducer pipe
        int to_reducer_pipe[2];
        safe_pipe(to_reducer_pipe);

        // Fork into reduce worker
        pid = safe_fork();
        if (pid == 0) {
            // reducer

            safe_close(to_reducer_pipe[WRITE_END]);

            // change stdin to read end of pipe
            safe_dup2(to_reducer_pipe[READ_END], STDIN_FILENO);
            safe_close(to_reducer_pipe[READ_END]);

            // i-1 pipes to sibling reducers exist in this child, close them
            for (int j = 0; j < i; j++) {
                safe_close(master_pipes.to_reducer[j]);
            }

            // break because child PC is still in for loop
            break;
        } else if (pid > 0) {
            // master
            // Store master->reducer pipe
            safe_close(to_reducer_pipe[READ_END]);
            master_pipes.to_reducer[i] = to_reducer_pipe[WRITE_END];
        }
    }

    // all pipes to reducers have been connected

    if (pid == 0) {
        // reducer, continuing after breaking from for loop
        // reduce blocked trying to read key value Pairs from stdin given
        // by master
        reduce_process_pairs();
        free(master_pipes.to_reducer);

    } else {
        // master

        // finished creating reduce workers
        // reducer children are blocked trying to read
        // create map workers
        create_mappers();
        // map workers are blocked trying to read
        // Read stdin for filenames, write to map workers
        distribute_files(dirname);
        // read and write mapped Pairs
        route_mapped_pairs();

        while(wait(NULL) > 0) {
            // waits for all children of master process to terminate
        }

        // free malloced memory
        free(master_pipes.from_mapper);
        free(master_pipes.to_mapper);
        free(master_pipes.to_reducer);
    }
}

/**
 * Creates a master worker that spawns m map children
 * and r reduce children, initiating MapReduce.
 *
 * @param  dirname      directory containing the input files.
 * @param  m            number of map children.
 * @param  r            number of reduce children.
 * @exit                1 if error
 * @return              zero on success, and a non-zero value on error.
 */
int create_master(char *dirname, int m, int r) {

    // master_pipes is a global variable of type PipeSet
    master_pipes.m = m;
    master_pipes.r = r;

    // Create the lister->master pipe
    int lister_pipe[2];
    safe_pipe(lister_pipe);

    // Fork into lister worker
    // Communication:  lister writes to stdout, master reads stdin
    pid_t pid = safe_fork();
    if (pid > 0) {
        // lister
        safe_close(lister_pipe[READ_END]);

        // change stdout to write end of pipe
        safe_dup2(lister_pipe[WRITE_END], STDOUT_FILENO);
        safe_close(lister_pipe[WRITE_END]);

        list(dirname);
    } else {
        // master
        safe_close(lister_pipe[WRITE_END]);

        // change stdin to read end of pipe
        safe_dup2(lister_pipe[READ_END], STDIN_FILENO);
        safe_close(lister_pipe[READ_END]);

        // no need to wait for lister child, program counter
        // for that process will start at fork point

        // create map and reduce workers
        create_workers(dirname);
    }

    return 0;
}

