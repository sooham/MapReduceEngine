// ~ ~ '
// A Walk Worker outputs to stdout the file paths
// to be read by Map Workers.
// ~ ~ ,

#include <unistd.h>
#include <stdlib.h>

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