/*
 A Map Worker receives file names via standard input,
 and outputs <key, value> Pairs through standard output
 using the currently defined map() function.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "mapreduce.h"
#include "utils.h"


/**
 * Process a given file as a map worker.
 * @param dirname The dirname to the file.
 */
void map_digest_file(char *dirname){
    char chunk[READSIZE + 1];

    FILE *map_file = safe_fopen(dirname, "r");

    size_t chunkSize;

    do {
        chunkSize = safe_fread(chunk, sizeof(char), READSIZE, map_file);

        // Null terminate chunk
        chunk[chunkSize] = '\0';

        // Use map function
        map(chunk, STDOUT_FILENO);
    } while(chunkSize == READSIZE);

    safe_fclose(map_file);
}

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files(){
    // note: PATH_MAX is an OS defined macro
    char file_path[PATH_MAX];

    while (scanf("%s", file_path) > 0) {
        map_digest_file(file_path);

    }

    exit(0);
}
