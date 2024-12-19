#include "RuntimeContext.h"

#include "Core/LogSystem/LogSystem.h"

namespace Nova {
void RuntimeContext::Startup() {
    m_logSystem = std::make_shared<LogSystem>();
}

void RuntimeContext::Shutdown() {
    m_logSystem.reset();
}

} // namespace Nova