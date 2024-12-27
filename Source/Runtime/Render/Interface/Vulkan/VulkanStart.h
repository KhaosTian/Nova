#pragma once

#include <concepts>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numbers>
#include <numeric>
#include <optional>
#include <span>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>


#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

#ifdef _WIN32 // 考虑平台是Windows的情况（请自行解决其他平台上的差异）
    #define VK_USE_PLATFORM_WIN32_KHR // 在包含vulkan.h前定义该宏，会一并包含vulkan_win32.h和windows.h
    #define NOMINMAX // 定义该宏可避免windows.h中的min和max两个宏与标准库中的函数名冲突
#endif

#include <vulkan/vulkan.h>