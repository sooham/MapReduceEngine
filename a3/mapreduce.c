#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
 * them evenly to map workers
 */
void process_files(int pipe){
    char file_name[MAX_FILENAME];
    // Just print for now
    while(scanf("%s", file_name) != EOF){
        printf("%s\n", file_name);
    }
}

/**
 * error handling function
 */



/**
 * Creates a walker worker and routes its
 * stdout to this process' stdin.
 */
int create_walker_worker(char *path){
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

        // Read stdin for filenames, send to map workers.
        process_files();
    }else{
        // ERROR
        fprintf(stderr, "Walker Worker: fork failed.\n");
        exit(11);
    }

    return 0;
}

int main(){
    char *path = "/Users/jcoc611/a3/group_0476/a3/texts";
    create_walker_worker(path);

    return 0;
}
