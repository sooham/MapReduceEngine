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

/**
 * Reads file names from stdin and distributes
 * them evenly to map workers.
 * @param m         The number of map workers.
 * @param in_pipes  An array of mapper->master pipes.
 * @param out_pipes An array of master->mapper pipes.
 */
void process_files(char *path, int m, int *in_pipes, int *out_pipes,
        int r, int *reduce_pipes){
    int current_worker = 0;

    char file_name[MAX_FILENAME];
    int path_length = strlen(path);

    // Read file names from walk worker
    // Send to map workers uniformly
while(scanf("%s", file_name) > 0){
        // Send to worker
        safe_write(
            out_pipes[current_worker],
            path,
            path_length * sizeof(char)
        );

        safe_write(
            out_pipes[current_worker],
            file_name,
            strlen(file_name) * sizeof(char)
        );
        safe_write(out_pipes[current_worker], "\0", sizeof(char));

        // Distribute uniformly
        current_worker++;
        if(current_worker + 1 > m){
            current_worker = 0;
        }
    }

    Pair read_pair;

    fd_set in_pipes_set;
    FD_ZERO(&in_pipes_set);

    int closed_pipes = 0;
    int max_pipe = 0;

    for(int i = 0; i < m; i++){
        safe_close(out_pipes[i]);

        // listen to in pipes
        FD_SET(in_pipes[i], &in_pipes_set);
        if(max_pipe < in_pipes[i]){
            max_pipe = in_pipes[i];
        }
    }

    // Read k/v Pairs from map worker
    // Send them to appropriate reduce worker
    // NOTE: the guarantee of uniformity when it comes to
    // distributing k/v pairs to reduce workers is
    // implicit in the hash function used. While juanhash
    // is not a perfect hash (it was built by us so it's
    // probably not too good), it performs relatively well.
    while(closed_pipes < m){
        select(max_pipe + 1, &in_pipes_set, NULL, NULL, NULL);
        // Process ready pipes
        for(int i = 0; i < m; i++){
            if(FD_ISSET(in_pipes[i], &in_pipes_set)){
                // Read from pipe
                int read_result = safe_read(in_pipes[i], &read_pair, sizeof(Pair));
                if(read_result == 0){
                    closed_pipes++;
                }else{
                    // Process <key, value> (i.e. send to reduce worker).
                    int reduce_id = juanhash(read_pair.key) % r;
                    write(reduce_pipes[reduce_id], &read_pair, sizeof(Pair));
                }
            }
        }
    }

    // Close reduce pipes
    for(int i = 0; i < r; i++){
        safe_close(reduce_pipes[i]);
    }
}


/**
 * Creates r reduce workers ready for use.
 * @param  m [description]
 * @param  r [description]
 * @return   [description]
 */
create_workers(char *path, int m, int r){
    int reduce_pipes[r];

    // Fork indicator
    int f = -1;

    // Fork n times
    // Establish pipes with each child
    // Return open pipes
    for(int i = 0; i < r; i++){
        // Create the mapper->master pipe
        int master_reduce_pipe[2];
        safe_pipe(master_reduce_pipe);

        // Fork into map worker
        f = safe_fork();

        if(f == 0){
            // Child (map worker)

            // Route STDIN from pipe master->reduce
            safe_close(master_reduce_pipe[1]);
            safe_dup2(master_reduce_pipe[0], STDIN_FILENO);

            // We don't want to spawn children from child
            break;
        }else{
            // Master

            // Store master->mapper pipe
            safe_close(master_reduce_pipe[0]);
            reduce_pipes[i] = master_reduce_pipe[1];
        }
    }

    if(f == 0){
        // Break from child
        // do reduce stuff

    }else{
        // Finished creating map workers
        // Read stdin for filenames, send to map workers.

        // Start the mapping process.
        create_map_workers(path, m, r, reduce_pipes);
    }
}

/**
 * Creates m map workers ready for use.
 * @param  m [description]
 * @return   [description]
 */
void create_map_workers(char *path, int m, int r, int *reduce_pipes){
    int in_pipes[m];
    int out_pipes[m];

    // Fork indicator
    int f = -1;

    // Fork n times
    // Establish pipes with each child
    // Return open pipes
    for(int i = 0; i < m; i++){
        // Create the mapper->master pipe
        int mapper_master_pipe[2];
        safe_pipe(mapper_master_pipe);

        // Create the master->mapper pipe
        int master_mapper_pipe[2];
        safe_pipe(master_mapper_pipe);

        // Fork into map worker
        f = safe_fork();

        if(f == 0){
            // Child (map worker)
            // Route STDOUT to pipe mapper->master
            safe_close(mapper_master_pipe[0]);
            safe_dup2(mapper_master_pipe[1], STDOUT_FILENO);

            // Route STDIN from pipe master->mapper
            safe_close(master_mapper_pipe[1]);
            safe_dup2(master_mapper_pipe[0], STDIN_FILENO);

            // We don't want to spawn children from child
            break;
        }else{
            // Master
            // Store mapper->master pipe
            safe_close(mapper_master_pipe[1]);
            in_pipes[i] = mapper_master_pipe[0];

            // Store master->mapper pipe
            safe_close(master_mapper_pipe[0]);
            out_pipes[i] = master_mapper_pipe[1];
        }
    }

    if(f == 0){
        // Break from child
        // do map stuff
        map_digest_files();
    }else{
        // Finished creating map workers
        // Read stdin for filenames, send to map workers.
        process_files(path, m, in_pipes, out_pipes, r, reduce_pipes);
    }
}

/**
 * Creates a master worker that spawns m map children
 * and r reduce children, initiating a MapReduce operation.
 * @param  path the location of the folder containing the input data files.
 * @param  m    the number of map children.
 * @param  r    the number of reduce children.
 */
int create_master(char *path, int m, int r){
    // Create the walker->master pipe
    int walker_pipe[2];
    safe_pipe(walker_pipe);

    // Fork into walker worker
    int f = safe_fork();
    if(f == 0){
        // Child (walker worker)
        safe_close(walker_pipe[0]);
        safe_dup2(walker_pipe[1], STDOUT_FILENO);

        walk_directory(path);
    }else{
        // Master, route child stdout to stdin
        safe_close(walker_pipe[1]);
        safe_dup2(walker_pipe[0], STDIN_FILENO);

        create_workers(path, m, r);
    }

    return 0;
}
