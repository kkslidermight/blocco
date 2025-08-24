#pragma once
#include "math.hpp"
struct AABB { Vec3 min; Vec3 max; };
bool intersect(const AABB&a,const AABB&b);
