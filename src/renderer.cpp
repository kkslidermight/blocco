#include "renderer.hpp"
#include <iostream>
Renderer::Renderer(bool headless):m_headless(headless){
  // TODO: Initialize SDL3 + Vulkan, swapchain, pipelines
  std::cout << "Renderer init (headless=" << m_headless << ")\n";
}
Renderer::~Renderer(){
  std::cout << "Renderer shutdown\n";
}
void Renderer::drawFrame(){
  // TODO actual rendering
  ++m_frameIndex;
}
