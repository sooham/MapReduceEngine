// ~ ~ '
// The number juan hash function.
// Made by Juan Camilo Osorio.
// ~ ~ ,

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <math.h>

/**
 * Returns a (hopefully) uniform hash of a given key.
 * @param  key the key to hash.
 * @return     the
 */
unsigned int juan_hash(const char *key){
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


int soohash(const char *key) {
    // h(k) = floor(m * (kA mod 1))
    // A = root(5) - 1

    // convert key to int
    int result = 0;
    for (int i = 0; i < strlen(key); i++) {
        result = result * 10 + (key[i] - '0');
    }

    double hash_result = floor(100.0 * fmod(result * (sqrt(5) - 1), 1.0));
    return (int) hash_result;
}

int main(){
	int test[101];
	int KEY_TRIES = 50000;

	for(int c = 0; c < 5000; c++){
		for(int z = 0; z < 101; z++){
			test[z] = 0;
		}

		for(int y = 0; y < KEY_TRIES; y++){
			int keysize = rand() % 64;
			char key[keysize];
			for(int k = 0; k < keysize - 1; k++){
				key[k] = (char) (rand());
			}
			key[keysize - 1] = '\0';
			int h = soohash(key) % 101;
			test[h] += 1;
		}

		float var = 0.0;
		for(int z = 0; z < 101; z++){
			int partial = (test[z] - KEY_TRIES/101.0);
			var += (partial * partial);
		}
		var /= 101.0;
		printf("VAR %.6f, for c = %i\n", var, c);
	}
	// (//)
	// char test[101];


	// while(scanf("%s", test) != EOF){
	// 	int h = juan_hash(test) % 101;
	// 	printf("h(%s) = %d\n", test, h);
	// }

}

//TFIN
