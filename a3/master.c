/*
 Master process orchestrates the execution of the Lister,
 Mapper and Reducer.
*/

// TODO: wait for all children to exit

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mapreduce.h"
#include "master.h"
#include "hash.h"
#include "mapper.h"
#include "lister.h"
#include "reducer.h"
#include "utils.h"

/**
 * Reads file names located at dirname from stdin and distributes
 * them evenly to mappers.
 *
 * @param dirname       directory to read files from
 * @param m             number of map workers
 * @param r             number of reduce workers
 * @param in_pipes      array of mapper->master pipes.
 * @param out_pipes     array of master->mapper pipes.
 * @reduce_pipes        array of master->reducer pipes
 */
void process_files(char *dirname, int m, int r, int *from_mapper_pipes, int *to_mapper_pipes, int *reduce_pipes) {
    int current_worker = 0;

    char filename[MAX_FILENAME];

    // Read file names from lister
    // Send to map workers uniformly
    while (scanf("%s", filename) != EOF){
        // Send to worker
        safe_write(
            to_mapper_pipes[current_worker],
            dirname,
            strlen(dirname) * sizeof(char)
        );
        safe_write(
            to_mapper_pipes[current_worker],
            filename,
            strlen(filename) * sizeof(char)
        );
        safe_write(to_mapper_pipes[current_worker], " ", sizeof(char));

        // Distribute uniformly
        current_worker++;
        if(current_worker + 1 > m) {
            current_worker = 0;
        }
    }
    // all files written to mappers

    Pair read_pair;

    fd_set from_mapper_set;
    FD_ZERO(&from_mapper_set);

    int closed_pipe_count = 0;

    for (int i = 0; i < m; i++) {
        safe_close(to_mapper_pipes[i]);

        // listen to in pipes
        FD_SET(from_mapper_pipes[i], &from_mapper_set);
    }

    // Read key value Pairs from map worker
    // Send them to appropriate reduce worker
    // NOTE: the guarantee of uniformity when it comes to
    // distributing key value pairs to reduce workers is
    // implicit in the hash function used. While hash
    // is not a perfect hash (it was built by us so it's
    // probably not too good), it performs relatively well.
    int closed_pipes[m];
    for(int i = 0; i < m; i++){
        closed_pipes[i] = 0;
    }
    while (closed_pipe_count < m) {
        int nready_fds = safe_select(FD_SETSIZE, &from_mapper_set, NULL, NULL, NULL);
        
        if (nready_fds > 0) {
            // Process ready pipes
            for (int i = 0; i < m; i++) {
                if(closed_pipes[i] == 1) continue;
                if(FD_ISSET(from_mapper_pipes[i], &from_mapper_set)) {
                    // Read from pipe
                    int read_result = safe_read(from_mapper_pipes[i], &read_pair, sizeof(Pair));
                    if (read_result <= 0) {
                        closed_pipes[i] = 1;
                        closed_pipe_count++;
                    } else {
                        // Process <key, value> (i.e. send to reduce worker).
                        int reduce_id = juanhash(read_pair.key) % r;
                        safe_write(reduce_pipes[reduce_id], &read_pair, sizeof(Pair));
                    }
                }
            }

            FD_ZERO(&from_mapper_set);
            for (int z = 0; z < m; z++) {
                if(closed_pipes[z] == 1) continue;
                // listen to in pipes
                FD_SET(from_mapper_pipes[z], &from_mapper_set);
            }
        }else{
            safe_fprintf(stderr, "No file descriptors available after select.\n");
        }
    }

    // Close mapper->master pipes.
    for(int i = 0; i < m; i++){
        safe_close(from_mapper_pipes[i]);
    }

    // Close reduce pipes
    for (int i = 0; i < r; i++) {
        safe_close(reduce_pipes[i]);
    }

    // wait for all children of master process to terminate
    // TODO: confirm correctness
    while(wait(NULL) > 0) {
    }
}

/**
 * Creates m map workers ready for use.
 * @param path
 * @param  m [description]
 * @param  r [description]
 * @param  reduce_pipes [description]
 * @return   [description]
 */
void create_mappers(char *dirname, int m, int r, int *reduce_pipes){
    int to_mapper_pipes[m];
    int from_mapper_pipes[m];

    // Fork indicator
    pid_t pid;

    // Fork m times
    // connect pipes with each child
    // one master->mapper pipe to transfer filenames
    // one mapper->master pipe to transfer intermediate key value pairs
    for (int i = 0; i < m; i++) {
        // Create the master->mapper pipe
        int master_mapper_pipe[2];
        safe_pipe(master_mapper_pipe);

        // Create the mapper->master pipe
        int mapper_master_pipe[2];
        safe_pipe(mapper_master_pipe);

        // fork into map worker
        pid = safe_fork();
        if (pid == 0) {
            // mapper child
            for (int j = 0; j < i; j++) {
                safe_close(to_mapper_pipes[j]);
                safe_close(from_mapper_pipes[j]);
            }

            // close all the pipes to reducers in mapper child process
            for (int j = 0; j < r; j++) {
                safe_close(reduce_pipes[j]);
            }

            // Route stdin from pipe master->mapper
            safe_close(master_mapper_pipe[1]);
            safe_dup2(master_mapper_pipe[0], STDIN_FILENO);

            // Route stdout to pipe mapper->master
            safe_close(mapper_master_pipe[0]);
            safe_dup2(mapper_master_pipe[1], STDOUT_FILENO);

            // don't spawn children in for loop
            break;
        } else {
            // master
            // Store master->mapper pipe
            safe_close(master_mapper_pipe[0]);
            to_mapper_pipes[i] = master_mapper_pipe[1];

            // Store mapper->master pipe
            safe_close(mapper_master_pipe[1]);
            from_mapper_pipes[i] = mapper_master_pipe[0];
        }
    }

    if (pid == 0) {
        // Break from child
        // mapper reads the file keys from its stdin
        map_digest_files();
    } else {
        // master
        // Finished creating map workers
        // Read stdin for filenames, send to map workers
        process_files(dirname, m, r, from_mapper_pipes, to_mapper_pipes, reduce_pipes);
    }
}



/**
 * Creates m mappers and r reducers ready to use.
 *
 * @param dirname           directory containing input files
 * @param  m                number of mappers
 * @param  r                number of reducers
 * @return TODO
 */
void create_workers(char *dirname, int m, int r) {
    int to_reduce_pipes[r];

    // fork indicator
    pid_t pid;

    // fork r times for reducers
    // make one master->reducer pipe to provide keys
    for (int i = 0; i < r; i++) {
        // Create the master->reducer pipe
        int master_reducer_pipe[2];
        safe_pipe(master_reducer_pipe);

        // Fork into reduce worker
        pid = safe_fork();
        if (pid == 0) {
            // reducer

            // Close all other pipes
            for(int z = 0; z < i; z++){
                safe_close(to_reduce_pipes[z]);
            }

            // Route stdin to pipe
            safe_close(master_reducer_pipe[1]);
            safe_dup2(master_reducer_pipe[0], STDIN_FILENO);
            safe_close(master_reducer_pipe[0]);

            // we are still running the for loop in the child
            break;
        } else if (pid > 0) {
            // Master
            // Store master->reducer pipe
            safe_close(master_reducer_pipe[0]);
            to_reduce_pipes[i] = master_reducer_pipe[1];
        }
    }
    // all pipes to reducers have been fixed

    if (pid == 0) {
        // reducer, continuing after breaking from for loop
        // reduce reads key value pairs from stdin
        reduce_process_pairs();

    } else {
        // master
        // finished creating reduce workers
        // Read stdin for filenames from lister
        // send to map workers.
        create_mappers(dirname, m, r, to_reduce_pipes);
    }
}

/**
 * Creates a master worker that spawns m map children
 * and r reduce children, initiating MapReduce.
 *
 * @param  dirname      directory containing the input files.
 * @param  m            number of map children.
 * @param  r            number of reduce children.
 * @exit                if system calling functions fail
 * @return              zero on success, and a non-zero value on error.
 */
int create_master(char *dirname, int m, int r) {
    // Create the lister->master pipe
    int lister_pipe[2];
    safe_pipe(lister_pipe);

    // Fork into lister worker
    // Communication: master reads stdin, lister writes to stdout
    pid_t pid = safe_fork();
    if (pid == 0) {
        // lister
        safe_close(lister_pipe[0]);                    // close read end
        safe_dup2(lister_pipe[1], STDOUT_FILENO);      // map stdout to write end
        safe_close(lister_pipe[1]);                    // we don't need this anymore

        list(dirname);
    } else {
        // master
        safe_close(lister_pipe[1]);                 // close write end
        safe_dup2(lister_pipe[0], STDIN_FILENO);    // map stdin to read end
        safe_close(lister_pipe[0]);                 // we don't need this anymore

        // TODO: wait for child here to see if directory is valid
        // (sync) or do something else, currently we do not
        // do shit if the dir is invalid

        // make the map and reduce workers
        create_workers(dirname, m, r);
    }

    return 0;
}
