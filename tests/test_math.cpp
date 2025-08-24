#include "math.hpp"
#include <cassert>
int main(){
  Vec3 a{1,2,3}, b{4,5,6};
  auto c = a + b;
  assert(c.x==5 && c.y==7 && c.z==9);
  assert(static_cast<int>(length(Vec3{3,4,0}))==5);
  return 0;
}
