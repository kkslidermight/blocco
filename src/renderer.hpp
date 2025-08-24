#pragma once
#include <cstdint>
#include <vector>
#include <string>
class Renderer {
public:
  Renderer(bool headless);
  ~Renderer();
  void drawFrame();
private:
  bool m_headless{false};
  uint32_t m_frameIndex{0};
};
