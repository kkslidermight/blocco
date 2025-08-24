#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
struct SDL_Window;
class Renderer {
public:
  Renderer(bool headless);
  ~Renderer();
  void drawFrame();
private:
  void initWindow();
  void initVulkan();
  void cleanup();
  void setupDebug();
  void createSurface();
  bool m_validationEnabled{false};
  bool m_headless{false};
  uint32_t m_frameIndex{0};
  SDL_Window* m_window{nullptr};
  VkInstance m_instance{VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
};
