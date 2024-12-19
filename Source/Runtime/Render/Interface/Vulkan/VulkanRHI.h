#pragma once
#include "VkStart.h"

namespace Nova {
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
    VkResult CreateInstance(VkInstanceCreateFlags flags = 0) {
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
            std::cout << std::format("[ VulkanManager ] 创建实例失败: ") << result << '\n';
            return result;
        }

        // 打印Vulkan API版本信息
        std::cout << std::format(
            "Vulkan API 版本: {}.{}.{}\n",
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
            std::cout << std::format("[ VulkanManager ] 枚举实例层属性失败: ") << result << '\n';
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
            std::cout << std::format("[ VulkanManager ] 枚举实例层属性失败: ") << result << '\n';
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
            std::cout << std::format("[ VulkanManager ] 枚举实例扩展属性失败: ") << result << '\n';
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
            std::cout << std::format("[ VulkanManager ] 枚举实例扩展属性失败: ") << result << '\n';
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
            std::cout << std::format("[ VulkanManager ] Debug: ") << pCallbackData->pMessage << '\n';
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
                std::cout << std::format("[ VulkanManager ] Failed to create debug messenger: ") << result << '\n';
            }
            return result;
        }

        std::cout << std::format("[ VulkanManager ] Failed to get vkCreateDebugUtilsMessengerEXT\n");

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
                    std::cout << std::format("[ VulkanManager ] Failed to get vkGetPhysicalDeviceSurfaceSupportKHR: ") << result << '\n';
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
            std::cout << std::format("[ VulkanManager ] Failed to enumerate physical devices: ") << result << '\n';
            return result;
        }
        if (deviceCount == 0) {
            std::cout << std::format("[ VulkanManager ] Failed to find any physical devices\n");
            abort();
        }

        mAvailablePhysicalDevices.resize(deviceCount);
        result = vkEnumeratePhysicalDevices(mInstance, &deviceCount, mAvailablePhysicalDevices.data());
        if (result != VK_SUCCESS) {
            std::cout << std::format("[ VulkanManager ] Failed to enumerate physical devices: ") << result << '\n';
        }
        return result;
    }

    // 为每个物理设备保存一份所需队列族索引的组合
    VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true) {
        static constexpr uint32_t notFound = INT32_MAX;

        struct QueueFamilyIndices {
            uint32_t graphics     = VK_QUEUE_FAMILY_IGNORED;
            uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t compute      = VK_QUEUE_FAMILY_IGNORED;
        };

        static std::vector<QueueFamilyIndices> queueFamilyIndices(mAvailablePhysicalDevices.size());
        auto& [ig, ip, ic] = queueFamilyIndices[deviceIndex];

        // 如果任意队列族索引应该被获取，但是没有得到，则失败
        if (ig == notFound && enableGraphicsQueue || ip == notFound && (mSurface != nullptr) || ic == notFound && enableComputeQueue) {
            return VK_RESULT_MAX_ENUM;
        }

        // 如果任意队列族应该被获取，但是没有找过，则尝试获取
        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue || ip == VK_QUEUE_FAMILY_IGNORED && (mSurface != nullptr) ||
            ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue) {
            uint32_t indices[3];
            VkResult result = GetQueueFamilyIndices(mAvailablePhysicalDevices[deviceIndex], enableGraphicsQueue, enableComputeQueue, indices);
            // 如果获取的结果是成功或者VK_RESULT_MAX_ENUM，则有了结果
            if (result == VK_SUCCESS || result == VK_RESULT_MAX_ENUM) {
                // 对返回的索引与INT32_MAX做按位与操作
                // 如果返回的索引为IGNORE，则按位与结果为
                if (enableGraphicsQueue) {
                    ig = indices[0] & INT32_MAX;
                }

                if (mSurface != nullptr) {
                    ip = indices[1] & INT32_MAX;
                }

                if (enableComputeQueue) {
                    ic = indices[2] & INT32_MAX;
                }
            }

            if (result != VK_SUCCESS) {
                return result;
            }
        } else {
            mQueueFamilyIndexGraphics     = enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
            mQueueFamilyIndexPresentation = (mSurface != nullptr) ? ip : VK_QUEUE_FAMILY_IGNORED;
            mQueueFamilyIndexCompute      = enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
        }

        mPhysicalDevice = mAvailablePhysicalDevices[deviceIndex];
        return VK_SUCCESS;
    }

    VkResult CreateDevice() {
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
