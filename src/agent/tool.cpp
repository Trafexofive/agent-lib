#include "../../inc/Tool.hpp" // Adjust path as necessary

Tool::Tool(const std::string& toolName, const std::string& toolDescription)
    : name(toolName), description(toolDescription), functionalCallback(nullptr) {}

Tool::Tool() : functionalCallback(nullptr) {}

std::string Tool::getName() const {
    return name;
}

std::string Tool::getDescription() const {
    return description;
}

void Tool::setName(const std::string& toolName) {
    name = toolName;
}

void Tool::setDescription(const std::string& toolDescription) {
    description = toolDescription;
}

void Tool::setCallback(FunctionalToolCallback callback) {
    if (!callback) {
        // Optional: Log a warning or throw if setting a null callback is invalid
        // For this B-line, we allow it, execute will check.
    }
    functionalCallback = callback;
}

std::string Tool::execute(const Json::Value& params) {
    if (functionalCallback) {
        return functionalCallback(params);
    }
    // Throw an error or return an error message if no callback is set
    // logMessage(LogLevel::ERROR, "Tool execute called on '" + name + "' but no callback is set.");
    throw std::runtime_error("No valid callback function set for tool '" + name + "'");
}
