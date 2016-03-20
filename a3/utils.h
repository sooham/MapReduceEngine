#ifndef UTILS_H
#define UTILS_H

// Safe versions of system call functions
// (Basically sys calls with error handling).
// Very overkill but hey marks am i rite

/**
 * Replaces current process with a given executable,
 * passing it a given array of arguments.
 * @param file  The path to the executable, including its name.
 * @param argv The array of arguments to pass this executable.
 *             The first argument should be the file name.
 */
void safe_execvp(const char *file, char *const argv[]);

/**
 * Read from a file descriptor.
 * @param  fildes  The file descriptor to read from.
 * @param  buf      The buffer in which to store data.
 * @param  nbyte  The number of bytes to read.
 * @return              The bytes read.
 */
ssize_t safe_read(int fildes, void *buf, size_t nbyte);

/**
 * Reads from a file.
 * @param  ptr          The variable in which to store the data read.
 * @param  size        The size of each element to be read.
 * @param  nmemb  The number of elements to be read.
 * @param  stream   The stream to read from.
 * @return                  The number of elements actually read.
 */
size_t safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/**
 * Writes data from buffer into a file descriptor.
 * @param fildes  The file descriptor to write data into.
 * @param buf      The source of the data.
 * @param nbyte   The size of the data.
 */
void safe_write(int fildes, const void *buf, size_t nbyte);

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
void safe_dup2(int oldfd, int newfd);

/**
 * Creates a new child process.
 * @return 0 for the child, the pid of the child for the parent.
 */
pid_t safe_fork();

/**
 * Creates a new pipe.
 */
void safe_pipe(int filedes[2]);

#endif