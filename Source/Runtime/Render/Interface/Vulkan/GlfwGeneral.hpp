#pragma once
#include "VulkanRHI.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

inline GLFWwindow* kWindow;

inline GLFWmonitor* kMonitor;

inline auto kWindowTitle = "LearnVulkan";

inline bool InitializeWindow(
    const VkExtent2D size,
    const bool fullScreen = false,
    const bool isResizable = true,
    bool enableFpsLimit = true)
{
    auto& rhi = Nova::VulkanRHI::GetInstance();
    
    if (glfwInit() == 0)
    {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }

    // 设置窗口属性
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(isResizable));

    uint32_t extensionCount = 0;
    const char** extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    if(extensionNames == nullptr)
    {
        std::cout<<std::format("[ InitializeWindow ] ERROR\nFailed to get GLFW required extensions!\n");
        glfwTerminate();
        return false;
    }

    for(size_t i=0; i < extensionCount; i++)
    {
        rhi.AddInstanceExtensionName(extensionNames[i]);
    }

    //window surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if(VkResult result = glfwCreateWindowSurface(rhi.GetVKInstance(), kWindow, nullptr, &surface))
    {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create GLFW surface!\n");
        glfwTerminate();
        return false;
    }

    rhi.SetSurface(surface);

    
    kMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* pMode = glfwGetVideoMode(kMonitor);

    // 创建窗口
    kWindow = fullScreen
        ? glfwCreateWindow(pMode->width, pMode->height, kWindowTitle, kMonitor, nullptr)
        : glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height), kWindowTitle, nullptr, nullptr);

    if (kWindow == nullptr)
    {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create GLFW window!\n");
        glfwTerminate();
        return false;
    }
    return true;
}

inline void TerminateWindow()
{
    glfwTerminate();
}

inline void UpdateWindowTitleWithFps()
{
    static double currentTime = 0.0; // 当前时间
    static double lastTime = glfwGetTime();
    static double timeDiff;
    static int frameCount = -1;
    static std::stringstream ss;

    currentTime = glfwGetTime(); // 获取当前时间
    frameCount++;

    if ((timeDiff = currentTime - lastTime) >= 0.1)
    {
        ss.precision(1);
        ss << kWindowTitle << "     " << std::fixed << frameCount / timeDiff << " FPS";
        glfwSetWindowTitle(kWindow, ss.str().c_str()); // 更新窗口标题
        ss.str("");
        lastTime = currentTime; // 更新上一帧时间
        frameCount = 0; // 重置帧计数器
    }
}

inline void MakeWindowFullScreen()
{
    const GLFWvidmode* mode = glfwGetVideoMode(kMonitor);
    glfwSetWindowMonitor(kWindow, kMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

inline void RestoreWindow(const VkOffset2D position, const VkExtent2D size)
{
    const GLFWvidmode* mode = glfwGetVideoMode(kMonitor);
    glfwSetWindowMonitor(kWindow, kMonitor,
                         position.x, position.y,
                         static_cast<int>(size.width), static_cast<int>(size.height),
                         mode->refreshRate);
}