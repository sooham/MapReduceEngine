// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "utils.h"

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
void safe_execvp(const char *file, char *const argv[]){
	if(execvp(file, argv) < 0){
		safe_fprintf(stderr, "Error executing %s\n", file);
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
ssize_t safe_read(int fildes, void *buf, size_t nbyte){
	ssize_t result = read(fildes, buf, nbyte);
	if(result < 0){
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
	if(result == 0){
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
	if(write(fildes, buf, nbyte) != nbyte){
		safe_fprintf(stderr, "Error writing to %d.\n", fildes);
		exit(1);
	}
}

/** 
 * Prints to stderr conveniently in a varadic fashion.
 *
 * @param msg       message to print.
 * @param count     total number of optional arguments provided.
 * @param ...       optional arugments
 */
void error(char *msg, int count, ...) {
    va_list vargs;
    va_start(vargs, count);

    char new_msg[strlen(msg) + 2];
    strncpy(new_msg, msg, sizeof(new_msg));
    new_msg[strlen(new_msg) + 1] = '\0';
    new_msg[strlen(new_msg)]  ='\n';

    if(vfprintf(stderr, new_msg, vargs) < 0){
    	safe_fprintf(stderr, "Error printing '%s' to stream.\n", new_msg);
    }

    va_end(vargs);
}

/**
 * Prints a formatted string to a given stream.
 * Same signature as fprintf.
 */
void safe_fprintf(FILE *stream, const char *format, ...){
	// A bit verbose for handling variable arguments
	va_list args;
	va_start(args, format);

	if(vfprintf(stream, format, args) < 0){
		if(stream == stderr) exit(2); // Prevents infinite loop of errors

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
FILE* safe_fopen(const char *path, const char *mode){
	FILE *result;
	result = fopen(path, mode);
	if(result == NULL){
		safe_fprintf(stderr, "Error opening file '%s', with mode '%d'\n", path, mode);
		exit(1);
	}
	return result;
}


/**
 * Closes a stream
 * @param stream  The stream to be closed.
 */
void safe_fclose(FILE *stream){
	if(fclose(stream) != 0){
		safe_fprintf(stderr, "Error closing a stream\n");
		exit(1);
	}
}

/**
 * Closes a given file descriptor.
 * @param file_descriptor the descriptor to be closed.
 */
void safe_close(int file_descriptor){
	if(close(file_descriptor) < 0){
		safe_fprintf(stderr, "Error closing file descriptor %d\n", file_descriptor);
		exit(1);
	}
}

/**
 * Replaces one file descriptor with another.
 */
void safe_dup2(int oldfd, int newfd){
	if(dup2(oldfd, newfd) < 0){
		safe_fprintf(stderr, "Error replacing descriptor %d with %d.\n", oldfd, newfd);
		exit(1);
	}
}

/**
 * Creates a new child process.
 * @return 0 for the child, the pid of the child for the parent.
 */
pid_t safe_fork(){
	pid_t result = fork();
	if(result < 0){
		safe_fprintf(stderr, "Error forking.\n");
		exit(1);
	}

	return result;
}

/**
 * Creates a new pipe.
 */
void safe_pipe(int filedes[2]){
	if(pipe(filedes) != 0){
		safe_fprintf(stderr, "Error piping.\n");
		exit(1);
	}
}
