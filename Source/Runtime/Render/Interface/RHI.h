#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <vector>

namespace Nova {
class WindowSystem;
struct RHIInitInfo {
    std::shared_ptr<WindowSystem> windowSystem;
};
class RHI {
public:
    virtual ~RHI() {}
    virtual void Initialize(RHIInitInfo initInfo) = 0;

    virtual void PrepareContext() = 0;

    // create
};
} // namespace Nova