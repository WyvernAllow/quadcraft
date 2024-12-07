#include "device.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <optional>
#include <set>
#include <vector>

struct queue_family_indices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const {
        return graphics_family.has_value() && present_family.has_value();
    }
};

static queue_family_indices get_queue_family_indices(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    queue_family_indices indices{};

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            !indices.graphics_family.has_value()) {
            indices.graphics_family = i;
        }

        VkBool32 has_present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &has_present_support);
        if (has_present_support && !indices.present_family.has_value()) {
            indices.present_family = i;
        }
    }

    return indices;
}

struct swapchain_support {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> present_modes;

    bool is_complete() const {
        return present_modes.size() != 0 && surface_formats.size() != 0;
    }
};

static swapchain_support get_swapchain_support(VkPhysicalDevice physical_device,
                                               VkSurfaceKHR surface) {
    swapchain_support support{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                              &support.capabilities);

    // Surface formats
    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &surface_format_count, nullptr);

    support.surface_formats.resize(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
                                         &surface_format_count,
                                         support.surface_formats.data());

    // Present modes
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count, nullptr);

    support.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &present_mode_count,
                                              support.present_modes.data());

    return support;
}

static int rate_physical_device(VkPhysicalDevice physical_device,
                                VkSurfaceKHR surface) {
    queue_family_indices queue_family_indices =
        get_queue_family_indices(physical_device, surface);
    if (!queue_family_indices.is_complete()) {
        return 0;
    }

    // TODO: Is this necessary? Having a queue with present support should imply
    // the existence of swapchain support.
    swapchain_support swapchain_support =
        get_swapchain_support(physical_device, surface);
    if (!swapchain_support.is_complete()) {
        return 0;
    }

    int score = 0;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    // Discrete GPUs often have better performance characteristics.
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }

    return score;
}

static VkPhysicalDevice find_best_device(VkInstance instance,
                                         VkSurfaceKHR surface) {
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count,
                               physical_devices.data());

    VkPhysicalDevice best_device = nullptr;
    int highest_score = std::numeric_limits<int>::min();
    for (const auto& physical_device : physical_devices) {
        int score = rate_physical_device(physical_device, surface);
        if (score > highest_score) {
            highest_score = score;
            best_device = physical_device;
        }
    }

    return best_device;
}

device::device(VkInstance instance, VkSurfaceKHR surface) {
    physical_device = find_best_device(instance, surface);
    if (!physical_device) {
        spdlog::critical("Failed to find a suitable physical device");
    }

    queue_family_indices queue_family_indices =
        get_queue_family_indices(physical_device, surface);

    assert(queue_family_indices.is_complete());

    swapchain_support swapchain_support =
        get_swapchain_support(physical_device, surface);

    assert(swapchain_support.is_complete());

    surface_capabilities = swapchain_support.capabilities;
    surface_formats = swapchain_support.surface_formats;
    present_modes = swapchain_support.present_modes;

    vkGetPhysicalDeviceProperties(physical_device, &properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    graphics_family_index = queue_family_indices.graphics_family.value();
    present_family_index = queue_family_indices.present_family.value();

    std::set<uint32_t> unique_queue_indices = {
        graphics_family_index,
        present_family_index,
    };

    float queue_priorities = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    for (const auto& index : unique_queue_indices) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.pQueuePriorities = &queue_priorities;
        queue_create_info.queueCount = 1;
        queue_create_info.queueFamilyIndex = index;
        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();

    VkResult result = vkCreateDevice(physical_device, &device_create_info,
                                     nullptr, &logical_device);
    if (result != VK_SUCCESS) {
        spdlog::critical("vkCreateDevice failed: {}", string_VkResult(result));
    }

    vkGetDeviceQueue(logical_device, graphics_family_index, 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, present_family_index, 0, &present_queue);
}
