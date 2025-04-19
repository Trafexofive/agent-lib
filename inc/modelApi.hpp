#pragma once
#include <stdexcept>
#include <string>

// Custom exception for API errors (can be shared by all clients)
class ApiError : public std::runtime_error {
public:
    ApiError(const std::string& message) : std::runtime_error(message) {}
};

// Abstract base class for LLM clients
class LLMClient {
public:
    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~LLMClient() = default;

    // Pure virtual function that all derived clients MUST implement
    // Takes a prompt and returns the generated text or throws ApiError
    virtual std::string generate(const std::string& prompt) = 0;

    // Optional: Add common configuration setters if desired,
    // but they might be better handled in derived classes if APIs differ significantly.
    virtual void setModel(const std::string& model) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setMaxTokens(int maxTokens) = 0;
};

