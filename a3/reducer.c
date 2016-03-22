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

#include "linkedlist.h"
#include "utils.h"

/*
 * Read Pairs from stdin and process as reduce worker.
 */
void reduce_process_pairs() {
    Pair read_pair;
    LLKeyValues *input_KV_list = NULL;
    while (safe_read(STDIN_FILENO, &read_pair, sizeof(Pair))) {
        insert_into_keys(&input_KV_list, read_pair);
    }

    // write to file [pid].out
    char filename[MAX_FILENAME] = "";
    sprintf(filename, "[%d].out", getpid());
    FILE *fout = safe_fopen(filename, "wb");

    // finished reading all the Pairs input from stdin by master
    // process them
    for (LLKeyValues *cur = input_KV_list; cur != NULL; cur = cur->next) {
        Pair result = reduce(cur->key, cur->head_value);
        safe_fwrite(&result, sizeof(Pair), 1, fout);
    }

    safe_fclose(fout);


    // TODO: free memory of linked lists
    // free_key_values_list(key_values_list);
}

