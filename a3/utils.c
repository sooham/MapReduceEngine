/*
 * Safe versions of system calling functions
 * with error handling.
 */

#include <errno.h>
#include <stdarg.h>

#include "utils.h"


/**
 * Replaces current process with a given executable,
 * passing it a given array of arguments.
 *
 * @param file  The path to the executable, including its name.
 * @param argv The array of arguments to pass this executable.
 *             The first argument should be the file name.
 */
void safe_execvp(const char *file, char *const argv[]) {
    if (execvp(file, argv) == -1) {
        safe_fprintf(stderr, "Error executing %s\n", file);
        perror("execvp");
        exit(1);
    }
}

/**
 * Read from a file descriptor.
 * @param  fildes  The file descriptor to read from.
 * @param  buf      The buffer in which to store data.
 * @param  nbyte  The number of bytes to read.
 * @return              The bytes read.
 */
ssize_t safe_read(int fildes, void *buf, size_t nbyte) {
    ssize_t result = read(fildes, buf, nbyte);
    if (result < 0) {
        safe_fprintf(stderr, "Error reading from a file descriptor.\n");
        exit(1);
    }

    return result;
}

/**
 * Reads from a file.
 * @param  ptr          The variable in which to store the data read.
 * @param  size        The size of each element to be read.
 * @param  nmemb  The number of elements to be read.
 * @param  stream   The stream to read from.
 * @return                  The number of elements actually read.
 */
size_t safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
    size_t result = fread(ptr, size, nmemb, stream);
    if (result == 0 && ferror(stream)) {
        safe_fprintf(stderr, "Error reading from file stream.\n");
        exit(1);
    }

    return result;
}

/**
 * Writes data from buffer into a file descriptor.
 * @param fildes  The file descriptor to write data into.
 * @param buf      The source of the data.
 * @param nbyte   The size of the data.
 */
void safe_write(int fildes, const void *buf, size_t nbyte){
    if (write(fildes, buf, nbyte) != nbyte) {
        safe_fprintf(stderr, "Error writing to %d.\n", fildes);
        switch(errno) {
            case EAGAIN:
            // case EWOULDBLOCK:
                safe_fprintf(stderr,
                 "Making a blocking write to a non-blocking fd.\n");
                break;
            case EBADF:
                safe_fprintf(stderr, "Invalid fd, or not open for writing.\n");
                break;
            case EDQUOT:
                safe_fprintf(stderr,
                 "Quota of disk blocks on file system exhausted.\n");
                break;
            case EFAULT:
                safe_fprintf(stderr,
                 "Segfault, buffer is outside accessible address space.\n");
                break;
            case EFBIG:
                safe_fprintf(stderr, "Maximum file size exceeded.\n");
                break;
            case EINTR:
                safe_fprintf(stderr, "Call interrupted by signal.\n");
                break;
            case EINVAL:
                safe_fprintf(stderr, "Fd unsuitable for writing.\n");
                break;
            case EIO:
                safe_fprintf(stderr, "A low-level I/O error occurred.\n");
                break;
            case ENOSPC:
                safe_fprintf(stderr,
                 "The device containing the file referred to by fd has no room for the data.\n");
                break;
            case EPIPE:
                safe_fprintf(stderr,
                 "The reading end of this pipe has been closed.\n");
                break;
            default:
                safe_fprintf(stderr, "An unknown error occurred.\n");
        }
        exit(1);
    }
}

/**
 * Writes binary data into a file stream.
 * @param ptr    A pointer to the data to be written.
 * @param size   The size of a single element to be written.
 * @param nmemb  The number of elements to be written.
 * @param stream The stream to write to.
 */
void safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (fwrite(ptr, size, nmemb, stream) != nmemb) {
        safe_fprintf(stderr, "Error writing to file stream.\n");
        exit(1);
    }
}

/**
 * Prints a formatted string to a given stream.
 * Same signature as fprintf.
 */
void safe_fprintf(FILE *stream, const char *format, ...) {
    // A bit verbose for handling variable arguments
    va_list args;
    va_start(args, format);

    if (vfprintf(stream, format, args) < 0) {
        if (stream == stderr) exit(2); // Prevents infinite loop of errors

        safe_fprintf(stderr, "Error printing '%s' to stream.\n", format);
    }

    va_end(args);
}

/**
 * Opens a file.
 * @param  path The full path to the file
 * @param  mode The mode the file should be opened in.
 * @return      A file descriptor, assuming the procedure worked.
 */
FILE* safe_fopen(const char *path, const char *mode) {
    FILE *result;
    result = fopen(path, mode);
    if(result == NULL) {
        safe_fprintf(stderr,
         "Error opening file '%s', with mode '%s'\n", path, mode);
        exit(1);
    }
    return result;
}


/**
 * Closes a stream
 *
 * @param stream  The stream to be closed.
 */
void safe_fclose(FILE *stream) {
    if (fclose(stream) != 0) {
        safe_fprintf(stderr, "Error closing a stream\n");
        exit(1);
    }
}

/*
 * Closes a given file descriptor.
 *
 * @param file_descriptor the descriptor to be closed.
 */
void safe_close(int file_descriptor) {
    if (close(file_descriptor) != 0) {
        safe_fprintf(stderr,
                     "Error closing file descriptor %d\n", file_descriptor);
        exit(1);
    }
}

/*
 * Replaces one file descriptor with another.
 */
int safe_dup2(int oldfd, int newfd) {
    int fd = dup2(oldfd, newfd);
    if (fd == -1) {
        safe_fprintf(stderr,
                     "Error replacing descriptor %d with %d.\n", oldfd, newfd);
        exit(1);
    }
    return fd;
}

/*
 * Creates a new child process.
 *
 * @return 0 for the child, the pid of the child for the parent.
 */
pid_t safe_fork() {
    pid_t result = fork();
    if (result == -1) {
        safe_fprintf(stderr, "Error forking.\n");
        exit(1);
    }

    return result;
}

/**
 * Creates a new pipe.
 *
 * @param fd    the file descriptor for the pipe
 */
void safe_pipe(int *fd) {
    if(pipe(fd) != 0) {
        safe_fprintf(stderr, "Error piping.\n");
        exit(1);
    }
}

/**
 * Waits for one or more file descriptors to be ready.
 *
 * @param  nfds       The largest file descriptor to wait for.
 * @param  read_fds   The read file descriptors to listen.
 * @param  write_fds  The write file descriptors to listen.
 * @param  except_fds The exception file descriptors to listen.
 * @return            The number of file descriptors that are ready.
 */
int safe_select(int nfds, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds) {
    int result = select(nfds, read_fds, write_fds, except_fds, NULL);
    if (result <= 0) {
        safe_fprintf(stderr, "Error with select.\n");
        exit(1);
    }
    return result;
}
