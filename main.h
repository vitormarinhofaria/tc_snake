#pragma once

struct Vec2 {
    int x;
    int y;
} typedef Vec2;

int vec2Equals(Vec2 a, Vec2 b){
    return a.x == b.x && b.x == b.y;
}