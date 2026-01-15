#include <math.h>
#include <stdint.h>

#ifndef GAME_MATH_H
#define GAME_MATH_H

constexpr long double PI = 3.14159265358979323846264338327950288420L;

struct Vec2 {
    int x, y;
};

struct FVec2 {
    double x, y;
};

struct Vec3 {
    int x, y, z;
};

struct FVec3 {
    double x, y, z;
};

struct FVec2 _FVec2_normalize(struct FVec2 v) {
    double len = sqrt(v.x * v.x + v.y * v.y);
    return (struct FVec2){v.x / len, v.y / len};
}

struct FVec2 _FVec2_sum(struct FVec2 *a, struct FVec2 *b) {
    return (struct FVec2){
        a->x + b->x,
        a->y + b->y,
    };
}

struct Vec2 _Vec2_sum(struct Vec2 *a, struct Vec2 *b) {
    return (struct Vec2){
        a->x + b->x,
        a->y + b->y,
    };
}

struct FVec3 _FVec3_sum(struct FVec3 *a, struct FVec3 *b) {
    return (struct FVec3){
        a->x + b->x,
        a->y + b->y,
        a->z + b->z,
    };
}

struct Vec3 _Vec3_sum(struct Vec3 *a, struct Vec3 *b) {
    return (struct Vec3){
        a->x + b->x,
        a->y + b->y,
        a->z + b->z,
    };
}

struct FVec2 _FVec2_mult(struct FVec2 *a, struct FVec2 *b) {
    return (struct FVec2){
        a->x * b->x,
        a->y * b->y,
    };
}

struct Vec2 _Vec2_mult(struct Vec2 *a, struct Vec2 *b) {
    return (struct Vec2){
        a->x * b->x,
        a->y * b->y,
    };
}

struct FVec3 _FVec3_mult(struct FVec3 *a, struct FVec3 *b) {
    return (struct FVec3){
        a->x * b->x,
        a->y * b->y,
        a->z * b->z,
    };
}

struct Vec3 _Vec3_mult(struct Vec3 *a, struct Vec3 *b) {
    return (struct Vec3){
        a->x * b->x,
        a->y * b->y,
        a->z * b->z,
    };
}

bool _FVec2_eq(struct FVec2 *a, struct FVec2 *b) {
    return a->x == b->x && a->y == b->y;
}

bool _Vec2_eq(struct Vec2 *a, struct Vec2 *b) {
    return a->x == b->x && a->y == b->y;
}

bool _FVec3_eq(struct FVec3 *a, struct FVec3 *b) {
    return a->x == b->x && a->y == b->y && a->z == b->z;
}

bool _Vec3_eq(struct Vec3 *a, struct Vec3 *b) {
    return a->x == b->x && a->y == b->y && a->z == b->z;
}

// poor man's function overloading
// a lot of casting, just to be sure
#define vec_sum(a, b)                                                          \
    _Generic((a),                                                              \
        struct Vec2: _Vec2_sum((struct Vec2 *)&a, (struct Vec2 *)(&b)),        \
        struct Vec3: _Vec3_sum((struct Vec3 *)&a, (struct Vec3 *)(&b)),        \
        struct FVec2: _FVec2_sum((struct FVec2 *)&a, (struct FVec2 *)(&b)),    \
        struct FVec3: _FVec3_sum((struct FVec3 *)&a, (struct FVec3 *)(&b)))

#define vec_mult(a, b)                                                         \
    _Generic((a),                                                              \
        struct Vec2: _Vec2_mult((struct Vec2 *)&a, (struct Vec2 *)(&b)),       \
        struct Vec3: _Vec3_mult((struct Vec3 *)&a, (struct Vec3 *)(&b)),       \
        struct FVec2: _FVec2_mult((struct FVec2 *)&a, (struct FVec2 *)(&b)),   \
        struct FVec3: _FVec3_mult((struct FVec3 *)&a, (struct FVec3 *)(&b)))

#define vec_eq(a, b)                                                           \
    _Generic((a),                                                              \
        struct Vec2: _Vec2_eq((struct Vec2 *)&a, (struct Vec2 *)(&b)),         \
        struct Vec3: _Vec3_eq((struct Vec3 *)&a, (struct Vec3 *)(&b)),         \
        struct FVec2: _FVec2_eq((struct FVec2 *)&a, (struct FVec2 *)(&b)),     \
        struct FVec3: _FVec3_eq((struct FVec3 *)&a, (struct FVec3 *)(&b)))

#endif
