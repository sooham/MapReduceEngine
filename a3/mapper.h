#ifndef MAPPER_H
#define MAPPER_H

/**
 * Process a given file as a map worker.
 * @param path The path to the file.
 */
void map_digest_file(char *path);

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files();

#endif