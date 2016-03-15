#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* a quick hash function for dealing with reduce processes
 * takes in a key and returns an int*/
int hash(const char *key) {
    // h(k) = floor(m * (kA mod 1))
    // A = root(5) - 1

    // convert key to int
    int result = 0;
    for (int i = 0; i < strlen(key); i++) {
        result = result * 10 + (key[i] - '0');
    }

    printf("%d\n", result);

    double hash_result = floor(100 * fmod(result * (sqrt(5) - 1), 1.0));
    return hash_result;
}

int main(int argc, char *argv[]) {
    printf("%d\n\n", hash(argv[1]));
}
