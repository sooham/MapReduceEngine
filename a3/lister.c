/*
 * Lister writes to stdout the files contained in given directory dirname.
 * The output is read by master.
 */

// #include <stdio.h>

#include "utils.h"

/*
 * Writes to stdout all filenames in a directory.
 *
 * @param dirname       directory to list
 * @exit                1 if exec fails
 */
void list(char *dirname) {
    char *argv[] = {"ls", dirname, NULL};
    safe_execvp("ls", argv);
}

