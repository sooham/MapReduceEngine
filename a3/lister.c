/*
 Lister writes to stdout the file paths to be assigned to Mappers.
*/

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include "utils.h"

#include "mapreduce.h"

/*
 * Reads the filename of all files in a directory,
 * and writes them to stdout.
 *
 * @param dirname       directory to list
 * @exit                1 if exec fails
 */
void list(char *dirname) {
    // add '/' to end of dirname, note it doesn't matter if
    // a directory ends with many '/'
    char full_dirname[MAX_FILENAME + 1];
    strncpy(full_dirname, dirname, sizeof(full_dirname));
    full_dirname[strlen(full_dirname)] = '/';
    // no need to end with null terminator, full_dirname is bigger
    // then dirname

    char *args[] = {"ls", full_dirname, NULL};
    safe_execvp("ls", args);

    // should not return
    perror("(list) execvp(\"ls\", args)");
    exit(1);
}
