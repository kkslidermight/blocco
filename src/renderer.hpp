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
  void pickPhysicalDevice();
  void createLogicalDevice();
  bool m_validationEnabled{false};
  bool m_headless{false};
  uint32_t m_frameIndex{0};
  SDL_Window* m_window{nullptr};
  VkInstance m_instance{VK_NULL_HANDLE};
  VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};
  uint32_t m_graphicsQueueFamily{UINT32_MAX};
  uint32_t m_presentQueueFamily{UINT32_MAX};
  VkQueue m_graphicsQueue{VK_NULL_HANDLE};
  VkQueue m_presentQueue{VK_NULL_HANDLE};
};
