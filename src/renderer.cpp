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
#include <array>

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
  createRenderPass();
  createFramebuffers();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
}

void Renderer::createSurface(){
  if(m_headless) return;
  if(!SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface)){
    throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
  }
}

void Renderer::cleanup(){
  if(!m_headless){
    vkDeviceWaitIdle(m_device);
  }
  for(auto fb: m_framebuffers){ if(fb) vkDestroyFramebuffer(m_device, fb, nullptr); }
  m_framebuffers.clear();
  if(m_renderPass){ vkDestroyRenderPass(m_device, m_renderPass, nullptr); m_renderPass = VK_NULL_HANDLE; }
  for(auto s: m_imageAvailable){ if(s) vkDestroySemaphore(m_device, s, nullptr); }
  for(auto s: m_renderFinished){ if(s) vkDestroySemaphore(m_device, s, nullptr); }
  for(auto f: m_inFlightFences){ if(f) vkDestroyFence(m_device, f, nullptr); }
  m_imageAvailable.clear(); m_renderFinished.clear(); m_inFlightFences.clear();
  if(m_commandPool){ vkDestroyCommandPool(m_device, m_commandPool, nullptr); m_commandPool = VK_NULL_HANDLE; }
  cleanupSwapchain();
  if(m_device){ vkDestroyDevice(m_device, nullptr); m_device = VK_NULL_HANDLE; }
  if(m_surface){ vkDestroySurfaceKHR(m_instance, m_surface, nullptr); m_surface = VK_NULL_HANDLE; }
  if(m_debugMessenger){ vkutils::destroyDebugMessenger(m_instance, m_debugMessenger); m_debugMessenger = VK_NULL_HANDLE; }
  if(m_instance){ vkDestroyInstance(m_instance, nullptr); m_instance = VK_NULL_HANDLE; }
  if(m_window){ SDL_DestroyWindow(m_window); m_window=nullptr; }
  if(!m_headless){ SDL_Quit(); }
}

void Renderer::drawFrame(){
  if(m_headless){ ++m_frameIndex; return; }
  vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
  uint32_t imageIndex;
  VkResult acq = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailable[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
  if(acq == VK_ERROR_OUT_OF_DATE_KHR){ /* TODO: recreate swapchain */ return; }
  if(acq != VK_SUCCESS && acq != VK_SUBOPTIMAL_KHR){ throw std::runtime_error("Failed to acquire swapchain image"); }
  if(m_imagesInFlight[imageIndex]){
    vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  }
  m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];
  // Record command buffer for this image
  vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);
  vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
  recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex);
  VkSemaphore waitSemaphores[] = { m_imageAvailable[m_currentFrame] };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore signalSemaphores[] = { m_renderFinished[m_currentFrame] };
  VkSubmitInfo submit{}; submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.waitSemaphoreCount = 1; submit.pWaitSemaphores = waitSemaphores; submit.pWaitDstStageMask = waitStages;
  submit.commandBufferCount = 1; submit.pCommandBuffers = &m_commandBuffers[imageIndex];
  submit.signalSemaphoreCount = 1; submit.pSignalSemaphores = signalSemaphores;
  if(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_inFlightFences[m_currentFrame]) != VK_SUCCESS){ throw std::runtime_error("Failed to submit draw"); }
  VkPresentInfoKHR present{}; present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.waitSemaphoreCount = 1; present.pWaitSemaphores = signalSemaphores;
  present.swapchainCount = 1; present.pSwapchains = &m_swapchain; present.pImageIndices = &imageIndex; present.pResults = nullptr;
  VkResult presRes = vkQueuePresentKHR(m_presentQueue, &present);
  if(presRes == VK_ERROR_OUT_OF_DATE_KHR || presRes == VK_SUBOPTIMAL_KHR){ /* TODO recreate swapchain */ }
  else if(presRes != VK_SUCCESS){ throw std::runtime_error("Failed to present"); }
  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

void Renderer::createRenderPass(){
  if(m_headless) return;
  VkAttachmentDescription color{};
  color.format = m_swapchainFormat;
  color.samples = VK_SAMPLE_COUNT_1_BIT;
  color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference colorRef{}; colorRef.attachment = 0; colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription sub{}; sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount = 1; sub.pColorAttachments = &colorRef;
  VkSubpassDependency dep{}; dep.srcSubpass = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0;
  dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.srcAccessMask = 0;
  dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo ci{}; ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  ci.attachmentCount = 1; ci.pAttachments = &color;
  ci.subpassCount = 1; ci.pSubpasses = &sub;
  ci.dependencyCount = 1; ci.pDependencies = &dep;
  if(vkCreateRenderPass(m_device, &ci, nullptr, &m_renderPass) != VK_SUCCESS){ throw std::runtime_error("Failed to create render pass"); }
}

void Renderer::createFramebuffers(){
  if(m_headless) return;
  m_framebuffers.resize(m_swapchainImageViews.size());
  for(size_t i=0;i<m_swapchainImageViews.size();++i){
    VkImageView attachments[] = { m_swapchainImageViews[i] };
    VkFramebufferCreateInfo ci{}; ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    ci.renderPass = m_renderPass;
    ci.attachmentCount = 1; ci.pAttachments = attachments;
    ci.width = m_swapchainExtent.width; ci.height = m_swapchainExtent.height; ci.layers = 1;
    if(vkCreateFramebuffer(m_device, &ci, nullptr, &m_framebuffers[i]) != VK_SUCCESS){ throw std::runtime_error("Failed to create framebuffer"); }
  }
}

void Renderer::createCommandPool(){
  if(m_headless) return;
  VkCommandPoolCreateInfo ci{}; ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; ci.queueFamilyIndex = m_graphicsQueueFamily; ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if(vkCreateCommandPool(m_device, &ci, nullptr, &m_commandPool) != VK_SUCCESS){ throw std::runtime_error("Failed to create command pool"); }
}

void Renderer::createCommandBuffers(){
  if(m_headless) return;
  m_commandBuffers.resize(m_swapchainImages.size());
  VkCommandBufferAllocateInfo ai{}; ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.commandPool = m_commandPool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
  if(vkAllocateCommandBuffers(m_device, &ai, m_commandBuffers.data()) != VK_SUCCESS){ throw std::runtime_error("Failed to allocate command buffers"); }
}

void Renderer::createSyncObjects(){
  if(m_headless) return;
  m_imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
  m_renderFinished.resize(MAX_FRAMES_IN_FLIGHT);
  m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);
  VkSemaphoreCreateInfo si{}; si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fi{}; fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for(int i=0;i<MAX_FRAMES_IN_FLIGHT;++i){
    if(vkCreateSemaphore(m_device, &si, nullptr, &m_imageAvailable[i]) != VK_SUCCESS ||
       vkCreateSemaphore(m_device, &si, nullptr, &m_renderFinished[i]) != VK_SUCCESS ||
       vkCreateFence(m_device, &fi, nullptr, &m_inFlightFences[i]) != VK_SUCCESS){
      throw std::runtime_error("Failed to create sync objects"); }
  }
}

void Renderer::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex){
  VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if(vkBeginCommandBuffer(cmd, &bi) != VK_SUCCESS) throw std::runtime_error("Begin cmd buffer failed");
  VkClearValue clear{}; clear.color = {{0.02f,0.02f,0.05f,1.0f}};
  VkRenderPassBeginInfo rp{}; rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp.renderPass = m_renderPass; rp.framebuffer = m_framebuffers[imageIndex];
  rp.renderArea.offset = {0,0}; rp.renderArea.extent = m_swapchainExtent;
  rp.clearValueCount = 1; rp.pClearValues = &clear;
  vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
  // (No pipeline yet) just clear
  vkCmdEndRenderPass(cmd);
  if(vkEndCommandBuffer(cmd) != VK_SUCCESS) throw std::runtime_error("End cmd buffer failed");
}
