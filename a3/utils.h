#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_NWORKERS 2       // The number of default workers

// Container for map reduce logistics
typedef struct mapReduceLogistics {
    int nmapworkers;
    int nreduceworkers;
    char *dirname;
} MapReduceLogistics;

/**
 * malloc with error checking.
 */
void safe_malloc(void **buffer, size_t size);

/**
 * Replaces current process with a given executable,
 * passing it a given array of arguments.
 */
void safe_execvp(const char *file, char *const argv[]);

/**
 * Read from a file descriptor.
 */
ssize_t safe_read(int fildes, void *buf, size_t nbyte);

/**
 * Reads from a file.
 */
size_t safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Writes data from buffer into a file descriptor.
 */
void safe_write(int fildes, const void *buf, size_t nbyte);

/**
 * Writes binary data into a file stream.
 */
void safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Prints a formatted string to a given stream.
 * Same signature as fprintf.
 */
void safe_fprintf(FILE *stream, const char *format, ...);

/**
 * Opens a file.
 * @param  path The full path to the file
 * @param  mode The mode the file should be opened in.
 * @return      A file descriptor, assuming the procedure worked.
 */
FILE* safe_fopen(const char *path, const char *mode);


/**
 * Closes a stream
 * @param stream  The stream to be closed.
 */
void safe_fclose(FILE *stream);

/**
 * Closes a given file descriptor.
 * @param file_descriptor the descriptor to be closed.
 */
void safe_close(int file_descriptor);

/**
 * Replaces one file descriptor with another.
 */
int safe_dup2(int oldfd, int newfd);

/**
 * Creates a new child process.
 */
pid_t safe_fork();

/**
 * Creates a new pipe.
 */
void safe_pipe(int filedes[2]);

/**
 * Waits for one or more file descriptors to be ready.
 */
int safe_select(int nfds, fd_set *read_fds, fd_set *write_fds, fd_set *except_fdst);

#endif
