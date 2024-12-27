#pragma once

#include "VulkanStart.h"
#include <Render/Interface/RHI.h>
#include <vector>

namespace Nova {
class VkRHI final: RHI {
private:
    GLFWwindow*      mWindow { nullptr };
    VkInstance       mInstance { nullptr };
    VkPhysicalDevice mPhysicalDevice { nullptr };
    VkDevice         mDevice { nullptr };

    VkSwapchainKHR       mSwapChain { nullptr };
    std::vector<VkImage> mSwapChainImages;
};
} // namespace Nova