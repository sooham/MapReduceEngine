#ifndef MAPPER_H
#define MAPPER_H

/**
 * Perform map() on the file chunk by chunk.
 */
void map_digest_file(char *file_path);

/**
 * Process all files assigned to this map worker.
 */
void map_digest_files();

#endif

