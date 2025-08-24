#include "collision.hpp"
#include <cassert>
int main(){
  AABB a{{0,0,0},{1,1,1}};
  AABB b{{0.5f,0.5f,0.5f},{2,2,2}};
  AABB c{{2,2,2},{3,3,3}};
  assert(intersect(a,b));
  assert(!intersect(a,c));
  return 0;
}
