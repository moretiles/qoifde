#include "rgba.h"

inline int colorsEqual (struct rgba diff){
    return diff.r == 0 && diff.g == 0 && diff.b == 0 && diff.a == 0;
}

inline int seenHash(struct rgba c){
    return (c.r * 3 + c.g * 5 + c.b * 7 + c.a * 11) % 64;
}

inline struct rgba colorsDiff (struct rgba c1, struct rgba c2) {
    struct rgba diff = { 0, 0, 0, 0 };
    diff.r = c1.r - c2.r;
    diff.g = c1.g - c2.g;
    diff.b = c1.b - c2.b;
    diff.a = c1.a - c2.a;
    return diff;
}

inline void addSeen(struct rgba color, struct rgba *colors, int pos){
    colors[pos].r = color.r;
    colors[pos].b = color.b;
    colors[pos].g = color.g;
    colors[pos].a = color.a;
}

inline int colorIndex(struct rgba color, struct rgba *colors){
    int pos = (64 + seenHash(color)) % 64;
    struct rgba testcolor = colorsDiff(color, colors[pos]);
    if (colorsEqual(testcolor)){
        return pos;
    } else {
        addSeen(color, colors, pos);
        return -1;
    }
}

