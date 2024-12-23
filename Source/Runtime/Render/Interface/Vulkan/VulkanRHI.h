#pragma once
#include "VkStart.h"
#include <cstdint>
#include <stdlib.h>

namespace Nova {

constexpr VkExtent2D kDefaultWindowSize = { 1280, 720 };

class VulkanRHI {
private:
    // 单例模式私有构造函数和析构函数，确保只能通过GetSingleton()获取实例
    VulkanRHI() = default;

public:
    // 禁用移动构造，保证单例唯一性
    VulkanRHI(VulkanRHI&&) = delete;

    // 获取单例实例的静态方法，使用Meyer'ss单例模式
    static VulkanRHI& GetInstance() {
        static VulkanRHI rhi;
        return rhi;
    }

    //===========================================================================
    // vulkan instance, layer, extension
    //================== =========================================================
private:
    VkInstance mInstance;

    // 实例层和扩展名称容器
    // 使用vector存储，支持动态添加和查询
    std::vector<const char*> mInstanceLayers;
    std::vector<const char*> mInstanceExtensions;

    // 安全地将名称添加到容器的辅助方法
    // 防止重复添加相同的层或扩展名称
    static void AddNameToContainer(const char* name, std::vector<const char*>& container) {
        // 检查是否已存在同名项
        for (const auto& item: container) {
            if (strcmp(name, item) == 0) {
                return; // 如果已存在，直接返回
            }
        }
        // 不存在则添加
        container.push_back(name);
    }

public:
    // 获取Vulkan实例句柄
    VkInstance GetVKInstance() const {
        return mInstance;
    }

    // 获取实例层名称列表
    const std::vector<const char*>& GetInstanceLayerNames() const {
        return mInstanceLayers;
    }

    // 获取实例扩展名称列表
    const std::vector<const char*>& GetInstanceExtensionNames() const {
        return mInstanceExtensions;
    }

    // 添加实例层名称
    void AddInstanceLayerName(const char* layer) {
        AddNameToContainer(layer, mInstanceLayers);
    }

    // 添加实例扩展名称
    void AddInstanceExtensionName(const char* extension) {
        AddNameToContainer(extension, mInstanceExtensions);
    }

    // 创建Vulkan实例的详细方法
    VkResult CreateVkInstance(VkInstanceCreateFlags flags = 0) {
// 在调试模式下自动添加验证层和调试扩展
#ifdef NOVA_DEBUG
        AddInstanceLayerName("VK_LAYER_KHRONOS_validation");
        AddInstanceExtensionName(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        // 配置应用程序信息
        VkApplicationInfo applicationInfo = {
            .sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = mApiVersion, // 使用存储的API版本
        };

        // 配置实例创建信息
        VkInstanceCreateInfo instanceCreateInfo = {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags                   = flags,
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(mInstanceLayers.size()),
            .ppEnabledLayerNames     = mInstanceLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(mInstanceExtensions.size()),
            .ppEnabledExtensionNames = mInstanceExtensions.data(),
        };

        // 尝试创建Vulkan实例
        if (const VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance)) {
            std::cout << std::format("[ Vulkan RHI ] 创建实例失败: ") << result << '\n';
            return result;
        }

        // 打印Vulkan API版本信息
        std::cout << std::format(
            "[ Vulkan RHI ] Vulkan API Version: {}.{}.{}\n",
            VK_VERSION_MAJOR(mApiVersion),
            VK_VERSION_MINOR(mApiVersion),
            VK_VERSION_PATCH(mApiVersion)
        );

// 在调试模式下创建调试信使
#ifdef NOVA_DEBUG
        CreateDebugMessenger();
#endif
        return VK_SUCCESS;
    }

    // 检查实例层是否可用
    VkResult CheckInstanceLayers(std::span<const char*> layersToCheck) {
        uint32_t layerCount;
        // 获取可用层数量
        if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
            std::cout << std::format("[ Vulkan RHI ] 枚举实例层属性失败: ") << result << '\n';
            return result;
        }

        // 如果没有可用层，清空输入的层列表
        if (layerCount == 0) {
            for (auto& layerName: layersToCheck) {
                layerName = nullptr;
            }
            return VK_SUCCESS;
        }

        // 获取可用层属性
        std::vector<VkLayerProperties> availableLayers;
        availableLayers.resize(layerCount);
        if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
            std::cout << std::format("[ Vulkan RHI ] 枚举实例层属性失败: ") << result << '\n';
            return result;
        }

        // 检查每个输入的层是否可用
        for (auto& layerName: layersToCheck) {
            bool found = false;
            for (auto& availableLayer: availableLayers) {
                if (std::strcmp(layerName, availableLayer.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            // 如果层不可用，设置为nullptr
            if (!found) {
                layerName = nullptr;
            }
        }

        return VK_SUCCESS;
    }

    // 设置实例层名称
    void SetInstanceLayerNames(const std::vector<const char*>& layerNames) {
        mInstanceLayers = layerNames;
    }

    // 检查实例扩展名称是否可用
    VkResult CheckInstanceExtensionNames(std::span<const char*> extensionNames) {
        uint32_t extensionCount;
        // 获取可用扩展数量
        if (VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr)) {
            std::cout << std::format("[ Vulkan RHI ] 枚举实例扩展属性失败: ") << result << '\n';
            return result;
        }

        // 如果没有可用扩展，清空输入的扩展列表
        if (extensionCount == 0) {
            for (auto& extensionName: extensionNames) {
                extensionName = nullptr;
            }
            return VK_SUCCESS;
        }

        // 获取可用扩展属性
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        if (VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data())) {
            std::cout << std::format("[ Vulkan RHI ] 枚举实例扩展属性失败: ") << result << '\n';
            return result;
        }

        // 检查每个输入的扩展是否可用
        for (auto& extensionName: extensionNames) {
            bool found = false;
            for (auto& availableExtension: availableExtensions) {
                if (std::strcmp(extensionName, availableExtension.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            // 如果扩展不可用，设置为nullptr
            if (!found) {
                extensionName = nullptr;
            }
        }
        return VK_SUCCESS;
    }

    // 设置实例扩展名称
    void SetInstanceExtensionNames(const std::vector<const char*>& extensionNames) {
        mInstanceExtensions = extensionNames;
    }

    //debug messenger
    //===========================================================================
private:
    VkDebugUtilsMessengerEXT mDebugMessenger;

    VkResult CreateDebugMessenger() {
        static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                                                     VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
                                                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                                     void* pUserData) -> VkBool32 {
            std::cout << std::format("[ Vulkan RHI ] Debug: ") << pCallbackData->pMessage << '\n';
            return VK_FALSE;
        };

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugUtilsMessengerCallback,
        };

        const auto vkCreateDebugUtilsMessengerExt =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessengerExt != nullptr) {
            const VkResult result = vkCreateDebugUtilsMessengerExt(mInstance, &debugUtilsMessengerCreateInfo, nullptr, &mDebugMessenger);
            if (result != VK_SUCCESS) {
                std::cout << std::format("[ Vulkan RHI ] Failed to create debug messenger: ") << result << '\n';
            }
            return result;
        }

        std::cout << std::format("[ Vulkan RHI ] Failed to get vkCreateDebugUtilsMessengerEXT\n");

        //没有合适的错误代码时就返回 VK_RESULT_MAX_ENUM
        return VK_RESULT_MAX_ENUM;
    }

    //window surface
    //===========================================================================
private:
    VkSurfaceKHR mSurface;

public:
    VkSurfaceKHR GetSurface() const {
        return mSurface;
    }

    void SetSurface(VkSurfaceKHR surface) {
        if (mSurface == nullptr) {
            mSurface = surface;
        }
    }

    //devices
    //===========================================================================
private:
    VkPhysicalDevice                 mPhysicalDevice;
    VkPhysicalDeviceProperties       mPhysicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties mPhysicalDeviceMemoryProperties;
    std::vector<VkPhysicalDevice>    mAvailablePhysicalDevices;

    VkDevice mDevice;
    uint32_t mQueueFamilyIndexGraphics     = VK_QUEUE_FAMILY_IGNORED;
    uint32_t mQueueFamilyIndexPresentation = VK_QUEUE_FAMILY_IGNORED;
    uint32_t mQueueFamilyIndexCompute      = VK_QUEUE_FAMILY_IGNORED;

    VkQueue mQueueGraphics;
    VkQueue mQueuePresentation;
    VkQueue mQueueCompute;

    std::vector<const char*> mDeviceExtensionNames;

public:
    const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const {
        return mPhysicalDeviceProperties;
    }

    const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const {
        return mPhysicalDeviceMemoryProperties;
    }

    VkDevice GetDevice() const {
        return mDevice;
    }

    VkPhysicalDevice GetAvailablePhysicalDevice(uint32_t index) const {
        return mAvailablePhysicalDevices[index];
    }

    uint32_t GetAvailablePhysicalDeviceCount() const {
        return static_cast<uint32_t>(mAvailablePhysicalDevices.size());
    }

    static VkBool32 ConvertToVkBool32(bool value) {
        return value ? VK_TRUE : VK_FALSE;
    }

    static bool ConvertToBool(VkBool32 value) {
        return (value == VK_TRUE);
    }

    VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphics, bool enableCompute, uint32_t (&queueFamilyIndices)[3]) {
        // 获取队列族数量
        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &QueueFamilyCount, nullptr);
        if (QueueFamilyCount == 0) {
            return VK_RESULT_MAX_ENUM;
        }

        // 获取队列族属性
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &QueueFamilyCount, queueFamilyProperties.data());

        // 初始化三种队列族索引
        auto& [ig, ip, ic] = queueFamilyIndices;
        ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;

        // 遍历队列族
        for (uint32_t i = 0; i < QueueFamilyCount; i++) {
            // 检查当前队列族是否支持三种队列
            bool supportGraphics     = enableGraphics && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0);
            bool supportPresentation = false;
            bool supportCompute      = enableCompute && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0);

            // 只有当surface存在时，才检查是否支持Presentation
            if (mSurface != nullptr) {
                VkBool32 support = ConvertToVkBool32(supportPresentation);
                VkResult result  = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mSurface, &support);
                if (result != VK_SUCCESS) {
                    std::cout << std::format("[ Vulkan RHI ] Failed to get vkGetPhysicalDeviceSurfaceSupportKHR: ") << result << '\n';
                    return result;
                }
                supportPresentation = ConvertToBool(support);
            }

            // 同时支持图形管线和计算管线时
            if (supportGraphics && supportCompute) {
                // 若需要呈现，则三者队列族索引应当相同
                if (supportPresentation) {
                    ig = ip = ic = i;
                    break;
                }

                // 若不需要呈现，则确保图形和计算管线的队列族相同
                if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED) {
                    ig = ic = i;
                }

                //surface不存在，直接结束
                if (mSurface == nullptr) {
                    break;
                }
            }

            // 如果任一种虽然支持但是没有获得索引，则设置为当前队列族的索引
            if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED) {
                ig = i;
            }

            if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED) {
                ic = i;
            }

            if (supportPresentation && ip == VK_QUEUE_FAMILY_IGNORED) {
                ip = i;
            }
        }

        // 如果任意需要的队列族没有得到，则失败
        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphics || ip == VK_QUEUE_FAMILY_IGNORED && (mSurface != nullptr) ||
            ic == VK_QUEUE_FAMILY_IGNORED && enableCompute) {
            return VK_RESULT_MAX_ENUM;
        }

        // 记录队列族索引
        mQueueFamilyIndexGraphics     = ig;
        mQueueFamilyIndexPresentation = ip;
        mQueueFamilyIndexCompute      = ic;
        return VK_SUCCESS;
    }

    uint32_t GetQueueFamilyIndexGraphics() const {
        return mQueueFamilyIndexGraphics;
    }

    uint32_t GetQueueFamilyIndexPresentation() const {
        return mQueueFamilyIndexPresentation;
    }

    uint32_t GetQueueFamilyIndexCompute() const {
        return mQueueFamilyIndexCompute;
    }

    const std::vector<const char*>& GetDeviceExtensions() const {
        return mDeviceExtensionNames;
    }

    void AddDeviceExtension(const char* extensionName) {
        AddNameToContainer(extensionName, mDeviceExtensionNames);
    }

    VkResult GetPhysicalDevice() {
        uint32_t deviceCount = 0;
        VkResult result      = vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ Vulkan RHI ] Failed to enumerate physical devices: ") << result << '\n';
            return result;
        }
        if (deviceCount == 0) {
            std::cout << std::format("[ Vulkan RHI ] Failed to find any physical devices\n");
            abort();
        }

        mAvailablePhysicalDevices.resize(deviceCount);
        result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, mAvailablePhysicalDevices.data());
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ Vulkan RHI ] Failed to enumerate physical devices: ") << result << '\n';
        }
        return result;
    }

    // 为每个物理设备保存一份所需队列族索引的组合
    VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphics = true, bool enableCompute = true) {
        static constexpr uint32_t NotFound = INT32_MAX;

        struct QueueIndex {
            uint32_t index  = VK_QUEUE_FAMILY_IGNORED;
            bool     enable = false;

            bool IsInvalid() const {
                return (index == NotFound) && enable;
            }

            bool ShouldGet() const {
                return (index == VK_QUEUE_FAMILY_IGNORED) && enable;
            }
        };

        struct QueueFamilyIndices {
            QueueIndex graphics;
            QueueIndex present;
            QueueIndex compute;
        };

        static std::vector<QueueFamilyIndices> queueFamilyIndices(mAvailablePhysicalDevices.size());
        auto& [ig, ip, ic] = queueFamilyIndices[deviceIndex];

        ig.enable = enableGraphics;
        ip.enable = mSurface != nullptr;
        ic.enable = enableCompute;

        // 如果任意队列族索引应该被获取，但是没有得到，则失败
        if (ig.IsInvalid() || ip.IsInvalid() || ic.IsInvalid()) {
            return VK_RESULT_MAX_ENUM;
        }

        // 如果任意队列族应该被获取，但是没有找过，则尝试获取
        if (ig.ShouldGet() || ip.ShouldGet() || ic.ShouldGet()) {
            uint32_t indices[3];
            VkResult result = GetQueueFamilyIndices(mAvailablePhysicalDevices[deviceIndex], enableGraphics, enableCompute, indices);
            // 如果获取的结果是成功或者VK_RESULT_MAX_ENUM，则有了结果
            if (result == VK_SUCCESS || result == VK_RESULT_MAX_ENUM) {
                // 对返回的索引与INT32_MAX做按位与操作
                // 如果返回的索引为IGNORE，则按位与结果为
                if (enableGraphics) {
                    ig.index = indices[0] & INT32_MAX;
                }

                if (mSurface != nullptr) {
                    ip.index = indices[1] & INT32_MAX;
                }

                if (enableCompute) {
                    ic.index = indices[2] & INT32_MAX;
                }
            }

            if (result != VK_SUCCESS) {
                return result;
            }
        }
        // 如果已经有了结果，则直接使用
        else {
            mQueueFamilyIndexGraphics     = enableGraphics ? ig.index : VK_QUEUE_FAMILY_IGNORED;
            mQueueFamilyIndexPresentation = (mSurface != nullptr) ? ip.index : VK_QUEUE_FAMILY_IGNORED;
            mQueueFamilyIndexCompute      = enableCompute ? ic.index : VK_QUEUE_FAMILY_IGNORED;
        }

        mPhysicalDevice = mAvailablePhysicalDevices[deviceIndex];
        return VK_SUCCESS;
    }

    VkResult CreateDevice() {
        // 统一使用1.0f的优先级
        float queuePriority = 1.0f;

        // 初始化队列创建信息数组
        VkDeviceQueueCreateInfo queueCreateInfos[3] = {
            {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = mQueueFamilyIndexGraphics,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            },
            {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = mQueueFamilyIndexPresentation,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            },
            {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = mQueueFamilyIndexCompute,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            },
        };

        // 创建设备队列信息的个数由队列族索引的个数决定
        uint32_t queueCreateInfoCount = 0;
        if (mQueueFamilyIndexGraphics != VK_QUEUE_FAMILY_IGNORED) {
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = mQueueFamilyIndexGraphics;
        }
        if (mQueueFamilyIndexPresentation != VK_QUEUE_FAMILY_IGNORED && mQueueFamilyIndexPresentation != mQueueFamilyIndexGraphics) {
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = mQueueFamilyIndexPresentation;
        }
        if (mQueueFamilyIndexCompute != VK_QUEUE_FAMILY_IGNORED && mQueueFamilyIndexCompute != mQueueFamilyIndexPresentation) {
            queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = mQueueFamilyIndexCompute;
        }

        // 获取物理设备特性
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(mPhysicalDevice, &physicalDeviceFeatures);

        // 构建设备创建信息
        VkDeviceCreateInfo deviceCreateInfo = { .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                                .queueCreateInfoCount    = queueCreateInfoCount,
                                                .pQueueCreateInfos       = queueCreateInfos,
                                                .enabledExtensionCount   = static_cast<uint32_t>(mDeviceExtensionNames.size()),
                                                .ppEnabledExtensionNames = mDeviceExtensionNames.data(),
                                                .pEnabledFeatures        = &physicalDeviceFeatures };

        // 创建逻辑设备
        VkResult result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ Vulkan RHI ] Failed to create vulkan logical device: ") << result << '\n';
            return result;
        }

        // 设置了每个队列族只创建一个队列，因此queueIndex为0
        if (mQueueFamilyIndexGraphics != VK_QUEUE_FAMILY_IGNORED) {
            vkGetDeviceQueue(mDevice, mQueueFamilyIndexGraphics, 0, &mQueueGraphics);
        }
        if (mQueueFamilyIndexPresentation != VK_QUEUE_FAMILY_IGNORED) {
            vkGetDeviceQueue(mDevice, mQueueFamilyIndexPresentation, 0, &mQueuePresentation);
        }
        if (mQueueFamilyIndexCompute != VK_QUEUE_FAMILY_IGNORED) {
            vkGetDeviceQueue(mDevice, mQueueFamilyIndexCompute, 0, &mQueueCompute);
        }

        // 获取物理设备属性和内存属性
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mPhysicalDeviceMemoryProperties);

        // 打印设备名称
        std::cout << std::format("[ Vulkan RHI ] Physical Device: {}\n", mPhysicalDeviceProperties.deviceName);

        return VK_SUCCESS;
    }

    VkResult CheckDeviceExtensionNames(std::span<const char*> extensionNamesToCheck, const char* layerName = nullptr) const {
        return VK_SUCCESS;
    }

    void SetDeviceExtensionNames(const std::vector<const char*>& extensionNames) {
        mDeviceExtensionNames = extensionNames;
    }

    //swap chain
    //===========================================================================
private:
    std::vector<VkSurfaceFormatKHR> mAvailableSurfaceFormats;

    VkSwapchainKHR           mSwapChain;
    std::vector<VkImage>     mSwapChainImages;
    std::vector<VkImageView> mSwapChainImageViews;

    VkSwapchainCreateInfoKHR mSwapChainCreateInfo = {};

    VkResult CreateSwapchainInternal() {
        // 获取surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCapabilities)) {
            std::cout << std::format("[ Vulkan RHI ] Failed to get physical device surface capabilities: ") << result << '\n';
            return result;
        }
        mSwapChainCreateInfo.minImageCount =
            surfaceCapabilities.minImageCount + static_cast<uint32_t>(surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);

        // 如果当前尺寸已经存在，则使用当前尺寸，否则使用最小尺寸和最大尺寸之间的默认尺寸
        VkExtent2D imageExtent = {};
        // 如果尺寸未定，当前尺寸的值会是-1
        if (surfaceCapabilities.currentExtent.width == -1) {
            imageExtent.width =
                glm::clamp(kDefaultWindowSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            imageExtent.height =
                glm::clamp(kDefaultWindowSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        } else {
            imageExtent = surfaceCapabilities.currentExtent;
        }
        mSwapChainCreateInfo.imageExtent = imageExtent;

        // 视点数
        mSwapChainCreateInfo.imageArrayLayers = 1;
        // 使用当前变换
        mSwapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;

        // 指定处理swapchia图形的透明通道的方式
        // window可能会指定，此时应选择继承方式
        if ((surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0u) {
            mSwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        } else {
            for (size_t i = 0; i < 4; i++) {
                if ((surfaceCapabilities.supportedCompositeAlpha & 1 << i) != 0u) {
                    mSwapChainCreateInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(surfaceCapabilities.supportedCompositeAlpha & 1 << i);
                    break;
                }
            }
        }

        // 设置交换链图像格式,默认为color attachment
        mSwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // 如果支持传输目标，则添加传输目标标志,这样可以执行清屏以及blit操作
        if ((surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0u) {
            mSwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        // 如果支持传输源，则添加传输源标志,可以实现窗口截屏
        if ((surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0u) {
            mSwapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        } else {
            std::cout << std::format("[ Vulkan RHI ] VK_IMAGE_USAGE_TRANSFER_SRC_BIT is not supported\n");
        }

        return VK_SUCCESS;
    }

public:
    const VkFormat& GetAvailableSurfaceFormat(uint32_t index) const {
        return mAvailableSurfaceFormats[index].format;
    }

    const VkColorSpaceKHR& GetAvailableSurfaceColorSpace(uint32_t index) const {
        return mAvailableSurfaceFormats[index].colorSpace;
    }

    uint32_t GetAvailableSurfaceFormatCount() const {
        return static_cast<uint32_t>(mAvailableSurfaceFormats.size());
    }

    VkSwapchainKHR GetSwapChain() const {
        return mSwapChain;
    }

    VkImage GetSwapChainImage(uint32_t index) const {
        return mSwapChainImages[index];
    }

    uint32_t GetSwapChainImageCount() const {
        return static_cast<uint32_t>(mSwapChainImages.size());
    }

    VkImageView GetSwapChainImageView(uint32_t index) const {
        return mSwapChainImageViews[index];
    }

    const VkSwapchainCreateInfoKHR& GetSwapChainCreateInfo() const {
        return mSwapChainCreateInfo;
    }

    VkResult GetSurfaceFormats() {
        uint32_t surfaceFormatCount = 0;

        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &surfaceFormatCount, nullptr);
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ Vulkan RHI ] Failed to get the count of surface formats: {}\n", int32_t(result));
            return result;
        }

        if (surfaceFormatCount == 0) {
            std::cout << std::format("[ Vulkan RHI ] Failed to find any surface formats\n");
            abort();
        }

        // 获取surface formats
        mAvailableSurfaceFormats.resize(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &surfaceFormatCount, mAvailableSurfaceFormats.data());
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ Vulkan RHI ] Failed to get surface formats\nError Code: {}\n", int32_t(result));
        }

        return VK_SUCCESS;
    }

    VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
        bool formatIsAvailable = false;
        bool needCheckFormat   = (surfaceFormat.format != VK_FORMAT_UNDEFINED);
        for (auto& item: mAvailableSurfaceFormats) {
            if (item.colorSpace == surfaceFormat.colorSpace && (!needCheckFormat || item.format == surfaceFormat.format)) {
                mSwapChainCreateInfo.imageFormat     = item.format;
                mSwapChainCreateInfo.imageColorSpace = item.colorSpace;
                formatIsAvailable                    = true;
                break;
            }
        }

        // 如果没有符合的格式,返回指定错误码
        if (formatIsAvailable == false) {
            std::cout << std::format("[ Vulkan RHI ] Surface format is not available\n");
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        // 如果交换链已经存在,则重新创建交换链
        if (mSwapChain != nullptr) {
            return RecreateSwapChain();
        }

        return VK_SUCCESS;
    }

    VkResult CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0) {
        return VK_SUCCESS;
    }

    VkResult RecreateSwapChain() {
        return VK_SUCCESS;
    }

    //vulkan version
    //===========================================================================
private:
    uint32_t mApiVersion = VK_API_VERSION_1_0;

public:
    uint32_t GetApiVersion() const {
        return mApiVersion;
    }

    VkResult UseLatestApiVersion() {
        if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion") != nullptr) {
            return vkEnumerateInstanceVersion(&mApiVersion);
        }
        return VK_SUCCESS;
    }
};
}; // namespace Nova
