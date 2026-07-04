#include "../framework.h"

// Classic high-performance DJB2 string hashing algorithm.
// Computes a deterministic 32-bit unique ID based strictly on title and author fields.
unsigned int PR_GenerateAppHash(const char *title, const char *author) {
    unsigned int hash = 5381;
    int c;

    // Stream the title characters through the hash matrix
    if (title != NULL) {
        while ((c = *title++)) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
    }

    // Continuously layer the author characters into the same hash stream
    if (author != NULL) {
        while ((c = *author++)) {
            hash = ((hash << 5) + hash) + c;
        }
    }

    return hash;
}
