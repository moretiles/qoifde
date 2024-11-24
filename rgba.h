#include <stdint.h>

struct rgba {
    int8_t r;
    int8_t g;
    int8_t b;
    int8_t a;
};

/*
 * Checks if a struct rgba is { 0, 0, 0, 0 }
 * Only called on the difference
 */
int colorsEqual (struct rgba diff);

// Hash function that indexes into the cached colors
int seenHash(struct rgba c);

// Calculate difference between two rgba structs
struct rgba colorsDiff (struct rgba c1, struct rgba c2);

// Copy a color into colors at pos
void addSeen(struct rgba color, struct rgba *colors, int pos);

// Check if color has been cached in colors
int colorIndex(struct rgba color, struct rgba *colors);
