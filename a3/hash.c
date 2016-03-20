// ~ ~ '
// The number juan hash function.
// Made by Juan Camilo Osorio.
// ~ ~ ,

#include <stdlib.h>
#include <string.h>

// Some notes:
// This hash function was created by us after some very short research.
// The constants chosen are what they are because they generated
// good results after testing with thousands of keys and possible values.

/**
 * Returns a (hopefully) uniform hash of a given key.
 * @param  key the key to hash.
 * @return     the integer hash for the given key.
 */
unsigned int juanhash(const char *key){
	int hash;
	int keysize = strlen(key);

	hash = 0;
	for(int i = 1; i < keysize - 1; i++){
		hash += (unsigned int) key[i];
		hash ^= 60;
		hash = hash >> 2;
		hash *= 1056;
	}

	hash = hash << 4;
	hash += (unsigned int) key[0];

	return hash;
}

//TFIN
