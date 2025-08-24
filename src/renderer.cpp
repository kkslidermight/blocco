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
#include <set>
#include <algorithm>

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
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapchain();
  createImageViews();
}

void Renderer::createSurface(){
  if(m_headless) return;
  if(!SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface)){
    throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
  }
}

void Renderer::cleanup(){
  cleanupSwapchain();
  if(m_device){ vkDestroyDevice(m_device, nullptr); m_device = VK_NULL_HANDLE; }
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

void Renderer::pickPhysicalDevice(){
  uint32_t count=0;
  vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
  if(count==0) throw std::runtime_error("No Vulkan physical devices found");
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(m_instance, &count, devices.data());
  for(auto d: devices){
    // Check queue families
    uint32_t qCount=0; vkGetPhysicalDeviceQueueFamilyProperties(d, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qprops(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(d, &qCount, qprops.data());
    uint32_t graphicsIdx = UINT32_MAX;
    uint32_t presentIdx = UINT32_MAX;
    for(uint32_t i=0;i<qCount;++i){
      if(qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsIdx = i;
      VkBool32 presentSupport = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(d, i, m_surface, &presentSupport);
      if(presentSupport) presentIdx = i;
    }
    if(graphicsIdx==UINT32_MAX || presentIdx==UINT32_MAX) continue;
    // Check device extension support
    uint32_t extCount=0; vkEnumerateDeviceExtensionProperties(d, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> exts(extCount);
    vkEnumerateDeviceExtensionProperties(d, nullptr, &extCount, exts.data());
    std::set<std::string> required;
    for(auto e: vkutils::deviceExtensions()) required.insert(e);
    for(const auto &e : exts){ required.erase(e.extensionName); }
    if(!required.empty()) continue;
    // Suitable
    m_physicalDevice = d;
    m_graphicsQueueFamily = graphicsIdx;
    m_presentQueueFamily = presentIdx;
    break;
  }
  if(m_physicalDevice==VK_NULL_HANDLE) throw std::runtime_error("No suitable GPU found");
}

void Renderer::createLogicalDevice(){
  if(m_physicalDevice==VK_NULL_HANDLE) return;
  float priority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueInfos;
  std::set<uint32_t> uniqueFamilies = {m_graphicsQueueFamily, m_presentQueueFamily};
  for(uint32_t fam : uniqueFamilies){
    VkDeviceQueueCreateInfo q{}; q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; q.queueFamilyIndex=fam; q.queueCount=1; q.pQueuePriorities=&priority; queueInfos.push_back(q);
  }
  VkPhysicalDeviceFeatures features{}; // default
  VkDeviceCreateInfo ci{}; ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  ci.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
  ci.pQueueCreateInfos = queueInfos.data();
  ci.pEnabledFeatures = &features;
  auto devExts = vkutils::deviceExtensions();
  ci.enabledExtensionCount = static_cast<uint32_t>(devExts.size());
  ci.ppEnabledExtensionNames = devExts.data();
  std::vector<const char*> layers;
  if(m_validationEnabled && vkutils::checkValidationLayerSupport()){
    layers = vkutils::validationLayers();
    ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames = layers.data();
  }
  if(vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device) != VK_SUCCESS){
    throw std::runtime_error("Failed to create logical device");
  }
  vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, m_presentQueueFamily, 0, &m_presentQueue);
}

void Renderer::createSwapchain(){
  if(m_headless) return;
  // Query support details
  VkSurfaceCapabilitiesKHR caps{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &caps);
  uint32_t formatCount=0; vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());
  uint32_t presentModeCount=0; vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

  // Choose format (prefer SRGB non-linear BGRA8/ R8G8B8A8)
  VkSurfaceFormatKHR chosenFormat = formats[0];
  for(const auto& f: formats){
    if((f.format == VK_FORMAT_B8G8R8A8_SRGB || f.format == VK_FORMAT_R8G8B8A8_SRGB) && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
      chosenFormat = f; break;
    }
  }
  // Choose present mode (MAILBOX -> IMMEDIATE -> FIFO)
  VkPresentModeKHR chosenPresent = VK_PRESENT_MODE_FIFO_KHR; // guaranteed
  for(auto pm: presentModes){ if(pm == VK_PRESENT_MODE_MAILBOX_KHR){ chosenPresent = pm; break; } }
  if(chosenPresent != VK_PRESENT_MODE_MAILBOX_KHR){
    for(auto pm: presentModes){ if(pm == VK_PRESENT_MODE_IMMEDIATE_KHR){ chosenPresent = pm; break; } }
  }
  // Extent
  VkExtent2D extent;
  if(caps.currentExtent.width != UINT32_MAX){
    extent = caps.currentExtent;
  } else {
    int w=0,h=0; SDL_GetWindowSize(m_window, &w, &h);
    extent.width = std::clamp<uint32_t>(static_cast<uint32_t>(w), caps.minImageExtent.width, caps.maxImageExtent.width);
    extent.height = std::clamp<uint32_t>(static_cast<uint32_t>(h), caps.minImageExtent.height, caps.maxImageExtent.height);
  }
  uint32_t imageCount = caps.minImageCount + 1;
  if(caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

  VkSwapchainCreateInfoKHR ci{}; ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = m_surface;
  ci.minImageCount = imageCount;
  ci.imageFormat = chosenFormat.format;
  ci.imageColorSpace = chosenFormat.colorSpace;
  ci.imageExtent = extent;
  ci.imageArrayLayers = 1;
  ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  uint32_t indices[2] = {m_graphicsQueueFamily, m_presentQueueFamily};
  if(m_graphicsQueueFamily != m_presentQueueFamily){
    ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    ci.queueFamilyIndexCount = 2;
    ci.pQueueFamilyIndices = indices;
  } else {
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  ci.preTransform = caps.currentTransform;
  ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  ci.presentMode = chosenPresent;
  ci.clipped = VK_TRUE;
  ci.oldSwapchain = VK_NULL_HANDLE;
  if(vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain) != VK_SUCCESS){
    throw std::runtime_error("Failed to create swapchain");
  }
  uint32_t actualCount=0; vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualCount, nullptr);
  m_swapchainImages.resize(actualCount);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualCount, m_swapchainImages.data());
  m_swapchainFormat = chosenFormat.format;
  m_swapchainExtent = extent;
}

void Renderer::createImageViews(){
  if(m_headless) return;
  m_swapchainImageViews.resize(m_swapchainImages.size());
  for(size_t i=0;i<m_swapchainImages.size();++i){
    VkImageViewCreateInfo ci{}; ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.image = m_swapchainImages[i];
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = m_swapchainFormat;
    ci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.subresourceRange.baseMipLevel = 0;
    ci.subresourceRange.levelCount = 1;
    ci.subresourceRange.baseArrayLayer = 0;
    ci.subresourceRange.layerCount = 1;
    if(vkCreateImageView(m_device, &ci, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS){
      throw std::runtime_error("Failed to create image view");
    }
  }
}

void Renderer::cleanupSwapchain(){
  for(auto v : m_swapchainImageViews){ if(v) vkDestroyImageView(m_device, v, nullptr); }
  m_swapchainImageViews.clear();
  if(m_swapchain){ vkDestroySwapchainKHR(m_device, m_swapchain, nullptr); m_swapchain = VK_NULL_HANDLE; }
  m_swapchainImages.clear();
}
