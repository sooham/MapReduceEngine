/*
 Lister writes to stdout the file paths to be read by Mappers.
*/

// #include <stdlib.h>
// #include <unistd.h>
#include "utils.h"

/**
 * Reads the filename of all files in a directory,
 * and writes them to stdout.
 *
 * @dirname path the path to the directory.
 */
void walk_directory(char *dirname){
    char *args[] = {"ls", dirname, NULL};
    safe_execvp("ls", args);
    // TODO: Is ls located here in cdf?
    // TODO: Does the directory exist?

    // TODO: Improve error handling here
    // If the above call fails exit with status 1.
    exit(1);
}
