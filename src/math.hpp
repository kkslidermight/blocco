#pragma once
#include <cmath>
struct Vec3 {float x{},y{},z{};};
struct Mat4 {float m[16];};
Vec3 operator+(const Vec3&a,const Vec3&b);
Vec3 operator-(const Vec3&a,const Vec3&b);
Vec3 operator*(const Vec3&a,float s);
float dot(const Vec3&a,const Vec3&b);
Vec3 cross(const Vec3&a,const Vec3&b);
float length(const Vec3&a);
Vec3 normalize(const Vec3&a);
