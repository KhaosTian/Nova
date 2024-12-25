#pragma once

#include "VulkanHelper.hpp"
#include "VulkanRHI.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

inline GLFWwindow* kWindow;

inline GLFWmonitor* kMonitor;

inline bool InitializeWindow(const VkExtent2D size, const bool fullScreen = false, const bool isResizable = true, bool limitFrameRate = true) {
    auto& rhi = Nova::VulkanRHI::Singleton();

    if (glfwInit() == 0) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }

    // 设置窗口属性
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(isResizable));

    // 获取显示器
    kMonitor = glfwGetPrimaryMonitor();

    // 获取视频模式
    const GLFWvidmode* pMode = glfwGetVideoMode(kMonitor);

    // 根据模式创建窗口
    kWindow = fullScreen
                  ? glfwCreateWindow(pMode->width, pMode->height, Nova::DEFAULT_WINDOW_TITLE, kMonitor, nullptr)
                  : glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height), Nova::DEFAULT_WINDOW_TITLE, nullptr, nullptr);
    if (kWindow == nullptr) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create GLFW window!\n");
        glfwTerminate();
        return false;
    }

    // 添加实例扩展
    rhi.AddInstanceExtensionName(VK_KHR_SURFACE_EXTENSION_NAME);
    rhi.AddInstanceExtensionName(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    // 添加交换链扩展
    rhi.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    //
    // 使用最新的Vulkan API版本
    rhi.UseLatestApiVersion();

    // 创建Vulkan实例
    if (rhi.CreateInstance() != VK_SUCCESS) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create Vulkan instance!\n");
        return false;
    }

    // 获取GLFW所需的实例扩展
    uint32_t     extensionCount = 0;
    const char** extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (extensionNames == nullptr) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to get GLFW required extensions!\n");
        glfwTerminate();
        return false;
    }
    // 添加GLFW所需的实例扩展
    for (size_t i = 0; i < extensionCount; i++) {
        rhi.AddInstanceExtensionName(extensionNames[i]);
    }

    //window surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult result = glfwCreateWindowSurface(rhi.GetInstance(), kWindow, nullptr, &surface);
    if (result != VK_SUCCESS) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create GLFW surface!\n");
        glfwTerminate();
        return false;
    }

    // 设置窗口表面
    rhi.SetSurface(surface);

    if (rhi.GetPhysicalDevice() != VK_SUCCESS) {
        return false;
    }

    if (rhi.DeterminePhysicalDevice(0, true, false) != VK_SUCCESS) {
        return false;
    }

    if (rhi.CreateDevice() != VK_SUCCESS) {
        return false;
    }

    // 创建交换链
    result = rhi.TryCreateSwapchain(limitFrameRate);
    if (result != VK_SUCCESS) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create swapchain: ") << result << '\n';
        return false;
    }

    return true;
}

inline void TerminateWindow() {
    // 终止窗口前应该确保vulkan device已经空闲, 没有和窗口系统的呈现引擎进行交互
    Nova::VulkanRHI::Singleton().WaitIdleDevice();
    glfwTerminate();
}

inline void UpdateWindowTitleWithFps() {
    static double            currentTime = 0.0; // 当前时间
    static double            lastTime    = glfwGetTime();
    static double            timeDiff;
    static int               frameCount = -1;
    static std::stringstream ss;

    currentTime = glfwGetTime(); // 获取当前时间
    frameCount++;

    if ((timeDiff = currentTime - lastTime) >= 0.1) {
        ss.precision(1);
        ss << Nova::DEFAULT_WINDOW_TITLE << "     " << std::fixed << frameCount / timeDiff << " FPS";
        glfwSetWindowTitle(kWindow, ss.str().c_str()); // 更新窗口标题
        ss.str("");
        lastTime   = currentTime; // 更新上一帧时间
        frameCount = 0; // 重置帧计数器
    }
}

inline void MakeWindowFullScreen() {
    const GLFWvidmode* mode = glfwGetVideoMode(kMonitor);
    glfwSetWindowMonitor(kWindow, kMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

inline void RestoreWindow(const VkOffset2D position, const VkExtent2D size) {
    const GLFWvidmode* mode = glfwGetVideoMode(kMonitor);
    glfwSetWindowMonitor(kWindow, kMonitor, position.x, position.y, static_cast<int>(size.width), static_cast<int>(size.height), mode->refreshRate);
}