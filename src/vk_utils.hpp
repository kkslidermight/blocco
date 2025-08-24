// Minimal Vulkan helper utilities for instance creation & validation checks.
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace vkutils {
struct InstanceConfig {
  bool enableValidation{false};
  std::vector<const char*> extensions; // required instance extensions
};

// Return list of validation layers we want (currently single standard layer).
inline const std::vector<const char*>& validationLayers() {
  static const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
  return layers;
}

bool checkValidationLayerSupport();

VkInstance createInstance(const InstanceConfig& cfg);

// Debug messenger handling (only used if validation enabled and extension present)
VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance);
void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger);
} // namespace vkutils
