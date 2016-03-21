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
    char *args[] = {"ls", dirname, NULL};
    safe_execvp("ls", args);

    // should not return
    perror("(list) execvp(\"ls\", args)");
    exit(1);
}
