// ~ ~ '
// A Map Worker receives file names via standard input,
// and outputs <key, value> Pairs through standard output
// using the currently defined map() function.
// ~ ~ ,

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/**
 * Process a given file as a map worker.
 * @param path The path to the file.
 */
void map_digest_file(char *path){
    char chunk[READSIZE + 1];

    FILE *map_file = fopen(path, "r");

    // DESIGN DECISION:
    // Should map worker die if file reading fails?
    if(map_file == NULL){
        fprintf(stderr, "Map Worker: can't open file '%s'", path);
        exit(1);
    }

    size_t chunkSize;
    do {
        chunkSize = fread(chunk, sizeof(char), READSIZE, map_file);
        if(chunkSize == 0) {
            fprintf(stderr, "Map Worker: Error reading file '%s'", path);
            exit(2);
        }

        // Null terminate chunk
        chunk[chunkSize + 1] = '\0';

        // Use map function
        map(chunk, STDOUT_FILENO);
    } while(chunkSize == READSIZE);

    fclose(map_file);
    exit(0);
}

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files(){
    char file_path[MAX_FILENAME];
    // Just print for now
    while(scanf("%s", file_path) != EOF){
        map_digest_file(file_path);
    }
}