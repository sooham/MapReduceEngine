/*
 Lister writes to stdout the file paths to be assigned to Mappers.
*/

#include <stdio.h>

#include "utils.h"

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
    // TODO: Used safe_execvp do we need error checking below?

    // should not return
    perror("(list) execvp(\"ls\", args)");
    exit(1);
}
