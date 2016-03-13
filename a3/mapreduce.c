#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mapreduce.h"

/* Container for map reduce logistics */
typedef struct mapReduceLogistics {
    int num_map_workers;
    int num_reduce_workers;
    char file_dir[MAX_FILENAME];
} MapReduceLogistics;


/**
 * Reads the filename of all files in a directory,
 * and writes them to stdout.
 * @param path the path to the directory.
 */
void walk_directory(char *path){
    char *args[] = { "ls", path, NULL };
    execvp("ls", args);

    // If the above call fails exit with status 1.
    exit(1);
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
// TODO: struct on stack on heap?
MapReduceLogistics process(int argc, char *const *argv) {
    MapReduceLogistics result = {
        .num_map_workers = 2,
        .file_dir = ""
    };

    opterr = 0; // don't print to stderr if wrong arguments passed
    int c;
    // m::r::d: means m and r flags are optional, d is required
    while ((c = getopt(argc, argv, "m::r::d:")) != -1) {
        switch (c) {
            case 'm':
                result.num_map_workers = strtol(optarg, NULL, 10);
                break;
            case 'r':
                result.num_reduce_workers = strtol(optarg, NULL, 10);
                break;
            case 'd':
                strncpy(result.file_dir, optarg, sizeof(result.file_dir));
                result.file_dir[sizeof(result.file_dir) - 1] = '\0';
                break;
            case '?':
            default:
                // fall through case
                printf("usage: mapreduce [-m nmapworkers] [-r nreduceworkers] -d dirname\n");
                printf("\t-m nmapworkers: number of map processes (default 2)\n");
                printf("\t-r nreduceworkers: number of reduce processes (default 2)\n");
                printf("\t-d dirname: directory of files to map reduce\n");
                exit(1);
        }
    }

    return result;
}


/**
 * Reads file names from stdin and distributes
 * them evenly to map workers.
 * @param m         The number of map workers.
 * @param in_pipes  An array of mapper->master pipes.
 * @param out_pipes An array of master->mapper pipes.
 */
void process_files(int m, int *in_pipes, int *out_pipes){
    int current_worker = 0;

    char file_name[MAX_FILENAME];
    // Just print for now
    while(scanf("%s", file_name) != EOF){
        // Send to worker
        write(
            out_pipes[current_worker],
            file_name,
            strlen(file_name)
        );
        write(out_pipes[current_worker], "\0", 1);

        // Distribute uniformly
        current_worker++;
        if(current_worker + 1 > m){
            current_worker = 0;
        }
    }

    fd_set in_pipes_set;
    FD_ZERO(&in_pipes_set);

    int closed_pipes = 0;

    for(int i = 0; i < m; i++){
        close(out_pipes[i]);

        // listen to in pipes
        FD_SET(in_pipes[i], &in_pipes_set);
    }

    char read_key[MAX_KEY];
    char read_val[MAX_VALUE];

    while(closed_pipes < m){
        select(m - closed_pipes, &in_pipes_set, NULL, NULL, NULL);
        // Process ready pipes
        for(int i = 0; i < m; i++){
            if(FD_ISSET(in_pipes[i], &in_pipes_set)){
                // Read from pipe
                if(fscanf(in_pipes[i], "%s%s", read_key, read_val) == EOF){
                    closed_pipes++;
                }else{
                    // Process <key, value> (i.e. send to reduce worker).
                    printf("Processing <%s, %s>\n", read_key, read_val);
                }
            }
        }
    }
}

/**
 * error handling function
 */

/**
 * Process a given file as a map worker.
 * @param path The path to the file.
 */
void map_digest_file(char *path){
    char chunk[READSIZE + 1];

    FILE *map_file = fopen(path, "r");

    // DESIGN DECISION:
    // Should map worker die if file reading fails?
    if(map_file == NULL){
        fprintf(stderr, "Map Worker: can't open file '%s'", path);
        exit(1);
    }
    
    size_t chunkSize;
    do {
        chunkSize = fread(chunk, sizeof(char), READSIZE, map_file);
        if(chunkSize == 0) {
            fprintf(stderr, "Map Worker: Error reading file '%s'", path);
            exit(2);
        }

        // Null terminate chunk
        chunk[chunkSize + 1] = '\0';

        // Use map function
        map(chunk, STDOUT_FILENO);
    } while(chunkSize == READSIZE);

    fclose(map_file);
}

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files(){
    char file_path[MAX_FILENAME];
    // Just print for now
    while(scanf("%s", file_path) != EOF){
        map_digest_file(file_path);
    }
}

/**
 * Creates m map workers ready for use.
 * @param  m [description]
 * @return   [description]
 */
void create_map_workers(int m){
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
        if(pipe(mapper_master_pipe)){
            fprintf(stderr, "Map Worker: mapper_paster pipe failed.\n");
            exit(20);
        }

        // Create the master->mapper pipe
        int master_mapper_pipe[2];
        if(pipe(master_mapper_pipe)){
            fprintf(stderr, "Map Worker: master_mapper pipe failed.\n");
            exit(21);
        }

        // Fork into map worker
        f = fork();

        if(f == 0){
            // Child (map worker)
            // Route STDOUT to pipe mapper->master
            close(mapper_master_pipe[0]);
            // dup2(mapper_master_pipe[1], STDOUT_FILENO);

            // Route STDIN from pipe master->mapper
            close(master_mapper_pipe[1]);
            dup2(master_mapper_pipe[0], STDIN_FILENO);

            // We don't want to spawn children from child
            break;
        }else if(f > 0){
            // Master
            // Store mapper->master pipe
            close(mapper_master_pipe[1]);
            in_pipes[i] = master_mapper_pipe[1];

            // Store master->mapper pipe
            close(master_mapper_pipe[0]);
            out_pipes[i] = master_mapper_pipe[1];
        }else{
            // ERROR
            fprintf(stderr, "Map Worker: fork failed.\n");
            exit(22);
        }
    }

    if(f == 0){
        // Break from child
        // do map stuff
        map_digest_files();
    }else{
        // Finished creating map workers
        // Read stdin for filenames, send to map workers.
        process_files(m, in_pipes, out_pipes);
    }
}

/**
 * Creates a walker worker and routes its
 * stdout to this process' stdin.
 */
int init_mapreduce(char *path, int m){
    // Create the walker->master pipe
    int walker_pipe[2];
    if(pipe(walker_pipe)){
        fprintf(stderr, "Walker Worker: pipe failed.\n");
        exit(10);
    }

    // Fork into walker worker
    int f = fork();
    if(f == 0){
        // Child (walker worker)
        close(walker_pipe[0]);
        dup2(walker_pipe[1], STDOUT_FILENO);
        walk_directory(path);
    }else if(f > 0){
        // Master, route child stdout to stdin
        close(walker_pipe[1]);
        dup2(walker_pipe[0], STDIN_FILENO);

        // Start the mapping process.
        create_map_workers(m);
    }else{
        // ERROR
        fprintf(stderr, "Walker Worker: fork failed.\n");
        exit(11);
    }

    return 0;
}

int main(){
    char *path = "/Users/jcoc611/a3/group_0476/a3/texts";
    int m = 1;
    init_mapreduce(path, m);

    return 0;
}
