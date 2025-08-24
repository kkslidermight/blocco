#include "engine.hpp"
#include <iostream>
int main(){
  try {
    Engine engine(true /*headless*/);
    engine.headlessCapture(120); // frames
  } catch(const std::exception& e){
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
