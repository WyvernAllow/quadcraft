#include "swapchain.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <algorithm>

#include "device.hpp"

static VkSurfaceFormatKHR find_surface_format(
    const device& device, VkFormat desired_format,
    VkColorSpaceKHR desired_color_space) {
    for (const auto& surface_format : device.surface_formats) {
        if (surface_format.format == desired_format &&
            surface_format.colorSpace == desired_color_space) {
            return surface_format;
        }
    }

    return device.surface_formats[0];
}

static VkPresentModeKHR find_present_mode(
    const device& device, VkPresentModeKHR desired_present_mode) {
    for (const auto& present_mode : device.present_modes) {
        if (present_mode == desired_present_mode) {
            return present_mode;
        }
    }

    return device.present_modes[0];
}

static VkExtent2D find_extent(const device& device, GLFWwindow* window) {
    if (device.surface_capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return device.surface_capabilities.currentExtent;
    }

    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent{};
    actual_extent.width = width;
    actual_extent.height = height;

    actual_extent.width = std::clamp(
        actual_extent.width, device.surface_capabilities.minImageExtent.width,
        device.surface_capabilities.maxImageExtent.width);

    actual_extent.height = std::clamp(
        actual_extent.height, device.surface_capabilities.minImageExtent.height,
        device.surface_capabilities.maxImageExtent.height);

    return actual_extent;
}

swapchain::swapchain(device& device, VkSurfaceKHR surface, GLFWwindow* window,
                     VkPresentModeKHR desired_present_mode,
                     VkFormat desired_format,
                     VkColorSpaceKHR desired_color_space) {
    surface_format =
        find_surface_format(device, desired_format, desired_color_space);

    present_mode = find_present_mode(device, desired_present_mode);
    extent = find_extent(device, window);

    image_count = device.surface_capabilities.minImageCount + 1;
    if (device.surface_capabilities.maxImageCount > 0 &&
        image_count > device.surface_capabilities.maxImageCount) {
        image_count = device.surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (device.graphics_family_index != device.present_family_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

        std::vector<uint32_t> queue_family_indices = {
            device.graphics_family_index,
            device.present_family_index,
        };

        create_info.queueFamilyIndexCount = queue_family_indices.size();
        create_info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = device.surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device.logical_device, &create_info,
                                           nullptr, &swapchain_handle);
    if (result != VK_SUCCESS) {
        spdlog::critical("vkCreateSwapchainKHR failed: {}",
                         string_VkResult(result));
    }

    vkGetSwapchainImagesKHR(device.logical_device, swapchain_handle,
                            &image_count, nullptr);

    images.resize(image_count);

    vkGetSwapchainImagesKHR(device.logical_device, swapchain_handle,
                            &image_count, images.data());

    for (uint32_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo image_view_info{};
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.image = images[i];
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = surface_format.format;
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;

        VkImageView image_view;
        VkResult result = vkCreateImageView(
            device.logical_device, &image_view_info, nullptr, &image_view);

        if (result != VK_SUCCESS) {
            spdlog::critical("vkCreateImageView failed: {}",
                             string_VkResult(result));
        }

        image_views.push_back(image_view);
    }
}

void swapchain::cleanup(device& device) {
    for (const auto& image_view : image_views) {
        vkDestroyImageView(device.logical_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(device.logical_device, swapchain_handle, nullptr);
}