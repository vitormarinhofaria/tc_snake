#pragma once

struct Vec2 {
    int x;
    int y;
} typedef Vec2;

int vec2Equals(Vec2 a, Vec2 b){
    return a.x == b.x && a.y == b.y;
}

void vec2Add(Vec2* a, Vec2 b){
    a->x += b.x;
    a->y += b.y;
}

void vec2AddScalar(Vec2* a, int b){
    a->x += b;
    a->y += b;
}