#pragma once

#include <json/json.h>
#include <string>
#include <functional> // For std::function
#include <stdexcept>  // For std::runtime_error

// Forward declaration (if Tool methods might take Agent context, not used in this simple version)
// class Agent;

// Type alias for the functional callback Tool will use
using FunctionalToolCallback = std::function<std::string(const Json::Value&)>;

class Tool {
private:
    std::string name;
    std::string description;
    FunctionalToolCallback functionalCallback;

public:
    // Constructors
    Tool(const std::string& toolName, const std::string& toolDescription);
    Tool(); // Default constructor

    // Getters
    std::string getName() const;
    std::string getDescription() const;

    // Setters
    void setName(const std::string& toolName);
    void setDescription(const std::string& toolDescription);
    void setCallback(FunctionalToolCallback callback);

    // Execution
    std::string execute(const Json::Value& params);
};
