#include <spdlog/spdlog.h>

#include <array>
#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

static void glfw_error_callback(int error_code, const char* description) {
    spdlog::error("GLFW: {}", description);
}

static VKAPI_ATTR VkBool32
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* pUserData) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::debug("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error("{}", pCallbackData->pMessage);
        break;
    default:
        break;
    }

    return VK_FALSE;
}

static std::vector<const char*> get_instance_layers() {
    return std::vector<const char*>{
        "VK_LAYER_KHRONOS_validation",
    };
}

static std::vector<const char*> get_instance_extensions() {
    uint32_t glfw_extension_count;
    const char** glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions,
                                        glfw_extensions + glfw_extension_count);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

static VkInstance create_instance(
    const VkDebugUtilsMessengerCreateInfoEXT& debug_info) {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_0;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    app_info.pApplicationName = "Quadcraft";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    app_info.pEngineName = nullptr;

    std::vector<const char*> layers = get_instance_layers();
    std::vector<const char*> extensions = get_instance_extensions();

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = &debug_info;
    instance_info.pApplicationInfo = &app_info;
    instance_info.ppEnabledLayerNames = layers.data();
    instance_info.enabledLayerCount = layers.size();
    instance_info.ppEnabledExtensionNames = extensions.data();
    instance_info.enabledExtensionCount = extensions.size();

    VkInstance instance;
    VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        spdlog::critical("vkCreateInstance failed: {}",
                         string_VkResult(result));
        return nullptr;
    }

    return instance;
}

static VkDebugUtilsMessengerEXT create_messenger(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& debug_info) {
    VkDebugUtilsMessengerEXT messenger;
    VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debug_info,
                                                     nullptr, &messenger);
    if (result != VK_SUCCESS) {
        spdlog::critical("vkCreateDebugUtilsMessengerEXT failed: {}",
                         string_VkResult(result));
        return nullptr;
    }

    return messenger;
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        spdlog::critical("glfwInit failed");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window =
        glfwCreateWindow(800, 450, "Quadcraft", nullptr, nullptr);
    if (!window) {
        spdlog::critical("glfwCreateWindow failed");
        return EXIT_FAILURE;
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.pfnUserCallback = vulkan_debug_callback;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance instance = create_instance(debug_info);
    if (!instance) {
        return EXIT_FAILURE;
    }

    VkDebugUtilsMessengerEXT messenger = create_messenger(instance, debug_info);
    if (!messenger) {
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}