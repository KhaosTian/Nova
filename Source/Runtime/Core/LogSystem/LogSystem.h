#pragma once

#include <spdlog/spdlog.h>

namespace Nova {
class LogSystem {
public:
    enum LogLevel : uint8_t {
        Debug,
        Info,
        Warning,
        Error,
    };

private:
    std::shared_ptr<spdlog::logger> m_logger;

public:
    LogSystem();
    ~LogSystem();

    template<typename... TArgs>
    void Log(LogLevel level, TArgs&&... args) {
        switch (level) {
            case Debug:
                m_logger->debug(std::forward<TArgs>(args)...);
                break;
            case Info:
                m_logger->info(std::forward<TArgs>(args)...);
                break;
            case Warning:
                m_logger->warn(std::forward<TArgs>(args)...);
                break;
            case Error:
                m_logger->error(std::forward<TArgs>(args)...);
                break;
        }
    }
};
} // namespace Nova
