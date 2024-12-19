#include <Runtime/Render/Interface/Vulkan/GlfwGeneral.hpp>

int main() {
    if (!InitializeWindow(VkExtent2D { 1280, 720 }))
    {
        return -1;
    }

    while (glfwWindowShouldClose(kWindow) == 0)
    {
        glfwPollEvents();
        UpdateWindowTitleWithFps();
    }

    TerminateWindow();
    return 0;
}
