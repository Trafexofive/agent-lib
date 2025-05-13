#include "../inc/ToolRegistry.hpp"
#include "../inc/Agent.hpp" // For logMessage, if used by registry for warnings

bool ToolRegistry::registerFunction(const std::string& identifier, FunctionalToolCallback callback) {
    if (registry_.count(identifier)) {
        logMessage(LogLevel::WARN, "ToolRegistry: Duplicate function identifier registration attempted.", "Identifier: " + identifier + ". Keeping existing.");
        return false;
    }
    registry_[identifier] = callback;
    logMessage(LogLevel::DEBUG, "ToolRegistry: Registered internal function.", "Identifier: " + identifier);
    return true;
}

FunctionalToolCallback ToolRegistry::getFunction(const std::string& identifier) const {
    auto it = registry_.find(identifier);
    if (it != registry_.end()) {
        return it->second;
    }
    logMessage(LogLevel::WARN, "ToolRegistry: Function identifier not found.", "Identifier: " + identifier);
    return nullptr;
}
