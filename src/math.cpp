#include "math.hpp"
Vec3 operator+(const Vec3&a,const Vec3&b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 operator-(const Vec3&a,const Vec3&b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3 operator*(const Vec3&a,float s){return {a.x*s,a.y*s,a.z*s};}
float dot(const Vec3&a,const Vec3&b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec3 cross(const Vec3&a,const Vec3&b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
float length(const Vec3&a){return std::sqrt(dot(a,a));}
Vec3 normalize(const Vec3&a){float l=length(a);return l>0? a*(1.f/l):a;}
