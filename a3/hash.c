/*
 * A rudimentary hash we developed in-house to assign to reducers.
 * Made by Juan Camilo Osorio.
 */

#include <string.h>

// Some notes:
// This hash function was created by us after some brief research.
// The constants are chosen because they generated minimal collisions
// while maintaining uniformity when tested with
// thousands of different input keys.

/*
 * Returns a hash value for a given key. Uniformly distributes keys.
 *
 * @param key       the key to hash
 * @return          integer hash for the given key.
 */
unsigned int hash(const char *key) {
    int hash;
    int keysize = strlen(key);

    hash = 0;
    for (int i = 1; i < keysize - 1; i++) {
        hash += (unsigned int) key[i];
        hash ^= 60;
        hash = hash >> 2;
        hash *= 1056;
    }

    hash = hash << 4;
    hash += (unsigned int) key[0];

    return hash;
}

