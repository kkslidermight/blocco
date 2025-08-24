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
  void createSwapchain();
  void createImageViews();
  void cleanupSwapchain();
  void createRenderPass();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
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
  VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
  std::vector<VkImage> m_swapchainImages;
  std::vector<VkImageView> m_swapchainImageViews;
  VkFormat m_swapchainFormat{VK_FORMAT_UNDEFINED};
  VkExtent2D m_swapchainExtent{0,0};
  VkRenderPass m_renderPass{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> m_framebuffers;
  VkCommandPool m_commandPool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> m_commandBuffers; // one per swapchain image
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  std::vector<VkSemaphore> m_imageAvailable;
  std::vector<VkSemaphore> m_renderFinished;
  std::vector<VkFence> m_inFlightFences;
  std::vector<VkFence> m_imagesInFlight; // size swapchain images, fence tracking
  size_t m_currentFrame{0};
};
