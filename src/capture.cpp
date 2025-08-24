#include "capture.hpp"
#include <fstream>
void Capture::savePNGDummy(const std::string& path){ std::ofstream f(path, std::ios::binary); if(f) f << "PNG"; }
