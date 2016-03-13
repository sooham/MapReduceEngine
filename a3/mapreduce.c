#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mapreduce.h"
#include "linkedlist.h"


/**
 * Reads the filename of all files in a directory,
 * and writes them to stdout.
 * @param path the path to the directory.
 */
void walk_directory(char *path){
<<<<<<< .mine

||||||| .r3
	
=======
    char *args[] = { path };
    execv("ls", args);

    // If the above call fails exit with status 1.
    exit(1);
>>>>>>> .r4
}

<<<<<<< .mine
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
// int optind
||||||| .r3
=======
/**
 * Reads file names from stdin and distributes
 * them evenly to map workers
 */
void process_files(){
    char file_name[MAX_FILENAME];
    // Just print for now
    while(scanf("%s", &file_name) != EOF){
        printf("%s", file_name); 
    }
}
>>>>>>> .r4

<<<<<<< .mine
/**
 * error handling function
 */



||||||| .r3
=======
/**
 * Creates a walker worker and routes its
 * stdout to this process' stdin.
 */
void create_walker_worker(char *path){
    int f = fork();

    // Create the walker->master pipe
    int walker_pipe[2];
    if(pipe(mypipe)){
        fprintf(stderr, "Walker Worker: pipe failed.\n");
        return EXIT_FAILURE;
    }

    if(f == 0){
        // Child (walker worker)
        close(walker_pipe[0]);
        dup2(walker_pipe[1], stdout);
        walk_directory(path);
    }else if(f > 0){
        // Master, route child stdout to stdin
        close(walker_pipe[1]);
        dup2(walker_pipe[0], stdin);
        // Read stdin for filenames, send to map workers.
        process_files()
    }else{
        // ERROR
        fprintf(stderr, "Walker Worker: fork failed.\n");
        exit(10);
    }
}

>>>>>>> .r4
int main(){
    char *path = "/Users/jcoc611/a3/group_0476/a3/texts";
    create_walker_worker(path);
}
