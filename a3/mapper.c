/*
 * The Map worker (mapper) receives filenames via stdin,
 * and outputs map() of <key, value> Pairs through stdout.
 */

#include <limits.h>

#include "mapreduce.h"
#include "utils.h"


/**
 * Perform map() on the file chunk by chunk.
 *
 * @param file_path         path of the file
 * @exit                    1 if error
 */
void map_digest_file(char *file_path) {
    char chunk[READSIZE + 1];

    FILE *map_file = safe_fopen(file_path, "r");

    size_t chunkSize;

    do {
        chunkSize = safe_fread(chunk, sizeof(char), READSIZE, map_file);
        chunk[chunkSize] = '\0';

        map(chunk, STDOUT_FILENO);
    } while (chunkSize == READSIZE);

    // check we reached EOF
    if (!feof(map_file)) {
        safe_fprintf(stderr, "Could not read to end of %s", file_path);
    }

    safe_fclose(map_file);
}

/**
 * Process all files assigned to this map worker.
 *
 * @exit        0 if all files processed correctly, else 1
 */
void map_digest_files() {
    // PATH_MAX is an OS defined macro
    char file_path[PATH_MAX];

    while(scanf("%s", file_path) > 0) {
        map_digest_file(file_path);
    }

    exit(0);
}

