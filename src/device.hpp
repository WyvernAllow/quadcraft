#pragma once
#include <vulkan/vulkan.h>

#include <vector>

struct device {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memory_properties;

    VkDevice logical_device;

    uint32_t graphics_family_index;
    VkQueue graphics_queue;

    uint32_t present_family_index;
    VkQueue present_queue;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> present_modes;

    device(VkInstance instance, VkSurfaceKHR surface);
};