/*
 A Map Worker receives file names via standard input,
 and outputs <key, value> Pairs through standard output
 using the currently defined map() function.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "mapreduce.h"
#include "utils.h"


/**
 * Process a given file as a map worker.
 * @param path The path to the file.
 */
void map_digest_file(char *path){
    char chunk[READSIZE + 1];

    FILE *map_file = safe_fopen(path, "r");

    size_t chunkSize;

    do {
        chunkSize = safe_fread(chunk, sizeof(char), READSIZE, map_file);

        // Null terminate chunk
        chunk[chunkSize + 1] = '\0';

        // Use map function
        map(chunk, STDOUT_FILENO);
    } while(chunkSize == READSIZE);

    safe_fclose(map_file);
}

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files(){
    char file_path[MAX_FILENAME];

    while(scanf("%s", file_path) > 0){
        map_digest_file(file_path);
    }

    exit(0);
}
