#ifndef LISTER_H
#define LISTER_H

/*
 * Reads the filename of all files in a directory,
 * and writes them to stdout.
 *
 * @param dirname       directory to list
 * @exit                1 if exec fails
 */
void list(char *dirname);

#endif
