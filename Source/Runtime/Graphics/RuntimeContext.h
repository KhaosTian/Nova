#pragma once

#include <memory>
#include <string>

namespace Nova {
class LogSystem;

class RuntimeContext {
public:
    static RuntimeContext& GetInstance() {
        static RuntimeContext instance;
        return instance;
    }

    RuntimeContext(RuntimeContext&&) = delete;

private:
    RuntimeContext() = default;

public:
    std::shared_ptr<LogSystem> m_logSystem;

    void Startup();
    void Shutdown();
};
} // namespace Nova
