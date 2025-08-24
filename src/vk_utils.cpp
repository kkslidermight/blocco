#include "vk_utils.hpp"
#include <stdexcept>
#include <cstring>
#include <iostream>

namespace vkutils {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData) {
  (void)severity; (void)types; (void)userData; // silence unused warnings
  std::cerr << "[vk] " << callbackData->pMessage << "\n";
  return VK_FALSE;
}

bool checkValidationLayerSupport() {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> props(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, props.data());
  for (const char* desired : validationLayers()) {
    bool found = false;
    for (const auto& p : props) {
      if (std::strcmp(desired, p.layerName) == 0) { found = true; break; }
    }
    if (!found) return false;
  }
  return true;
}

VkInstance createInstance(const InstanceConfig& cfg) {
  if (cfg.enableValidation && !checkValidationLayerSupport()) {
    std::cerr << "Validation layers requested but not available; continuing without." << std::endl;
  }
  VkApplicationInfo app{};
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.pApplicationName = "blocco";
  app.applicationVersion = VK_MAKE_API_VERSION(0,0,1,0);
  app.pEngineName = "blocco";
  app.engineVersion = VK_MAKE_API_VERSION(0,0,1,0);
  app.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  ci.pApplicationInfo = &app;
  ci.enabledExtensionCount = static_cast<uint32_t>(cfg.extensions.size());
  ci.ppEnabledExtensionNames = cfg.extensions.empty() ? nullptr : cfg.extensions.data();

  std::vector<const char*> layers;
  if (cfg.enableValidation && checkValidationLayerSupport()) {
    layers = validationLayers();
    ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames = layers.data();
  }

  VkInstance instance{};
  VkResult res = vkCreateInstance(&ci, nullptr, &instance);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan instance");
  }
  return instance;
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
  VkDebugUtilsMessengerEXT messenger{};
  auto createFn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  auto destroyFn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (!createFn || !destroyFn) return VK_NULL_HANDLE; // Extension absent
  VkDebugUtilsMessengerCreateInfoEXT ci{};
  ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  ci.pfnUserCallback = debugCallback;
  if (createFn(instance, &ci, nullptr, &messenger) != VK_SUCCESS) {
    std::cerr << "Failed to create debug messenger" << std::endl;
    return VK_NULL_HANDLE;
  }
  return messenger;
}

void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
  if (!messenger) return;
  auto destroyFn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (destroyFn) destroyFn(instance, messenger, nullptr);
}

} // namespace vkutils
