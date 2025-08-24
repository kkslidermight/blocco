#include "renderer.hpp"
#include "vk_utils.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#pragma GCC diagnostic pop
#include <iostream>
#include <stdexcept>
#include <vector>

Renderer::Renderer(bool headless):m_headless(headless){
  m_validationEnabled = !m_headless; // skip validation in pure headless for now
  if(!m_headless){ initWindow(); initVulkan(); }
  std::cout << "Renderer init (headless=" << m_headless << ")" << std::endl;
}
Renderer::~Renderer(){
  cleanup();
  std::cout << "Renderer shutdown" << std::endl;
}

void Renderer::initWindow(){
  if(SDL_Init(SDL_INIT_VIDEO)!=0){
    std::string err = SDL_GetError();
    throw std::runtime_error(std::string("SDL_Init failed: ")+err);
  }
  m_window = SDL_CreateWindow("blocco", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  if(!m_window){
    throw std::runtime_error(std::string("SDL_CreateWindow failed: ")+SDL_GetError());
  }
}

void Renderer::initVulkan(){
  uint32_t extCount = 0;
  const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);
  if(!extNames || extCount==0){
    throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed");
  }
  std::vector<const char*> extensions(extNames, extNames + extCount);
  // Add debug utils if validation
  if(m_validationEnabled){
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  vkutils::InstanceConfig cfg; cfg.enableValidation = m_validationEnabled; cfg.extensions = extensions;
  m_instance = vkutils::createInstance(cfg);
  if(m_validationEnabled){
    m_debugMessenger = vkutils::createDebugMessenger(m_instance);
  }
  createSurface();
}

void Renderer::createSurface(){
  if(m_headless) return;
  if(!SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface)){
    throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
  }
}

void Renderer::cleanup(){
  if(m_surface){ vkDestroySurfaceKHR(m_instance, m_surface, nullptr); m_surface = VK_NULL_HANDLE; }
  if(m_debugMessenger){ vkutils::destroyDebugMessenger(m_instance, m_debugMessenger); m_debugMessenger = VK_NULL_HANDLE; }
  if(m_instance){ vkDestroyInstance(m_instance, nullptr); m_instance = VK_NULL_HANDLE; }
  if(m_window){ SDL_DestroyWindow(m_window); m_window=nullptr; }
  if(!m_headless){ SDL_Quit(); }
}

void Renderer::drawFrame(){
  // Nothing yet; swapchain not created. Just tick.
  ++m_frameIndex;
}
