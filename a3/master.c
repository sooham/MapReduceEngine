/*
 Master process orchestrates the execution of the Lister,
 Mapper and Reducer.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "master.h"
#include "mapreduce.h"
#include "utils.c"

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
        write(
            to_mapper_pipes[current_worker],
            dirname,
            strlen(dirname) * sizeof(char)
        );

        write(
            to_mapper_pipes[current_worker],
            filename,
            strlen(filename) * sizeof(char)
        );
        write(to_mapper_pipes[current_worker], "\0", sizeof(char));

        // Distribute uniformly
        current_worker = ++current_worker % m;
    }

    Pair read_pair;

    fd_set from_mapper_set;
    FD_ZERO(&from_mapper_set);

    int closed_pipes = 0;
    int max_pipe = 0;

    for (int i = 0; i < m; i++) {
        close(to_mapper_pipes[i]);

        // listen to in pipes
        FD_SET(from_mapper_pipes[i], &from_mapper_set);
        if(max_pipe < from_mapper_pipes[i]){
            max_pipe = from_mapper_pipes[i];
        }
    }

    // Read key value Pairs from map worker
    // Send them to appropriate reduce worker
    // NOTE: the guarantee of uniformity when it comes to
    // distributing key value pairs to reduce workers is
    // implicit in the hash function used. While hash
    // is not a perfect hash (it was built by us so it's
    // probably not too good), it performs relatively well.
    while (closed_pipes < m) {
        select(max_pipe + 1, &from_mapper_set, NULL, NULL, NULL);
        // Process ready pipes
        for (int i = 0; i < m; i++) {
            if (FD_ISSET(from_mapper_pipes[i], &from_mapper_set)) {
                // Read from pipe
                int read_result = read(from_mapper_pipes[i], &read_pair, sizeof(Pair));
                if (read_result == 0) {
                    closed_pipes++;
                } else if (read_result < 0) {
                    printf("Master: Error reading map, %d", errno);
                } else {
                    // Process <key, value> (i.e. send to reduce worker).
                    int reduce_id = juanhash(read_pair.key) % r;
                    write(reduce_pipes[reduce_id], &read_pair, sizeof(Pair));
                }
            }
        }
    }

    // Close reduce pipes
    for(int i = 0; i < r; i++){
        close(reduce_pipes[i]);
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
        if (pipe(master_mapper_pipe)) {
            perror("(create_mappers) pipe(master_mapper_pipe)");
            exit(1);
        }

        // Create the mapper->master pipe
        int mapper_master_pipe[2];
        if (pipe(mapper_master_pipe)) {
            perror("(create_mappers) pipe(mapper_master_pipe)");
            exit(1);
        }

        // fork into map worker
        if ((pid = fork()) == 0) {
            // mapper

            // Route stdin from pipe master->mapper
            close(master_mapper_pipe[1]);
            dup2(master_mapper_pipe[0], STDIN_FILENO);

            // Route stdout to pipe mapper->master
            close(mapper_master_pipe[0]);
            dup2(mapper_master_pipe[1], STDOUT_FILENO);

            // don't spawn children in for loop
            break;
        } else if (pid > 0) {
            // master
            // Store master->mapper pipe
            close(master_mapper_pipe[0]);
            to_mapper_pipes[i] = master_mapper_pipe[1];

            // Store mapper->master pipe
            close(mapper_master_pipe[1]);
            from_mapper_pipes[i] = mapper_master_pipe[0];
        } else {
            // ERROR
            perror("(create_mappers) fork()");
            exit(1);
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
        if(pipe(master_reducer_pipe)) {
            perror("(create_workers) pipe(master_reducer_pipe)");
            exit(1);
        }

        // Fork into reduce worker
        if((pid = fork()) == 0) {
            // reducer
            // Route stdin to pipe
            close(master_reducer_pipe[1]);
            dup2(master_reducer_pipe[0], STDIN_FILENO);
            // we are still running the for loop in the child
            break;
        } else if (pid > 0) {
            // Master
            // Store master->reducer pipe
            close(master_reducer_pipe[0]);
            to_reduce_pipes[i] = master_reducer_pipe[1];
        } else {
            // ERROR
            perror("(create_workers) fork()");
            exit(1);
        }
    }
    // all pipes to reducers have been fixed

    if (pid == 0) {
        // reducer, continuing after breaking from for loop
        // reduce reads key value pairs from stdin

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
 * @return              zero on success, and a non-zero value on error.
 */
int create_master(char *dirname, int m, int r) {
    // Create the lister->master pipe
    int lister_pipe[2];
    if(pipe(lister_pipe)){
        perror("(create_master) pipe(lister_pipe)");
        exit(1);
    }

    // Fork into lister worker
    // read from master stdin, write from lister stdout
    pid_t pid;
    if ((pid = fork()) == 0) {
        // lister
        close(lister_pipe[0]);                          // close read end
        if (dup2(lister_pipe[1], STDOUT_FILENO)) {      // map stdout to write end
            perror("(create_master) dup2(lister_pipe[1], STDOUT_FILENO)");
            exit(1);
        }
        list(dirname);
    } else if (pid > 0) {
        // master
        close(lister_pipe[1]);                         // close write end
        if(dup2(lister_pipe[0], STDIN_FILENO)) {       // map stdin to read end
            perror("(create_master) dup2(lister_pipe[0], STDIN_FILENO)");
            exit(1);
        }

        // TODO: wait for child here to see if directory is valid
        // (sync) or do something else, currently we do not
        // do shit if the dir is invalid
        // TODO: when to close the pipe? can we close dup2ed pipes?

        // make the map and reduce workers
        create_workers(dirname, m, r);
    } else {
        perror("(create_master) fork()");
        exit(1);
    }

    return 0;
}
