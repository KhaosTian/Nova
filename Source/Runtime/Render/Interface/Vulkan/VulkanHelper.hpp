#pragma once
#include "VulkanStart.h"

namespace Nova {
static inline VkExtent2D DEFAULT_WINDOW_SIZE = { 1280, 720 };

static inline const char* DEFAULT_WINDOW_TITLE = "LearnVulkan";

static inline void AddNameToContainer(const char* name, std::vector<const char*>& container) {
    // 检查是否已存在同名项
    for (const auto& item: container) {
        if (strcmp(name, item) == 0) {
            return; // 如果已存在，直接返回
        }
    }
    // 不存在则添加
    container.push_back(name);
}

static inline VkBool32 ConvertToVkBool32(bool value) {
    return value ? VK_TRUE : VK_FALSE;
}

static inline bool ConvertToBool(VkBool32 value) {
    return (value == VK_TRUE);
}
} // namespace Nova