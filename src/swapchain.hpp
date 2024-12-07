#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "device.hpp"

struct swapchain {
    VkSwapchainKHR swapchain_handle;

    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;

    uint32_t image_count;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;

    swapchain(
        device& device, VkSurfaceKHR surface, GLFWwindow* window,
        VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
        VkFormat desired_format = VK_FORMAT_B8G8R8A8_SRGB,
        VkColorSpaceKHR desired_color_space =
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    // TODO: Add RAII
    void cleanup(device& device);
};
