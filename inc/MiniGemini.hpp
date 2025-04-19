#ifndef GEMINI_CLIENT_HPP
#define GEMINI_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // For Json::Value (or your preferred JSON library)

const std::string MODELS[] = {
    // Main models
    "gemini-2.0-flash",         // Latest Flash model
    "gemini-2.0-flash-lite",    // Cost-efficient Flash variant
    "gemini-1.5-flash",         // Previous generation Flash model
    "gemini-1.5-flash-8b",      // Lightweight 8B parameter variant
    "gemini-2.5-pro-exp-03-25", // Experimental Pro model

    // Special purpose models
    "gemini-embedding-exp",      // Text embedding model
    "models/text-embedding-004", // Basic text embedding model
    "imagen-3.0-generate-002",   // Image generation model
    "models/embedding-001"       // Legacy embedding model
};

class MiniGemini : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    MiniGemini(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Gemini-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Allow changing the base URL if needed

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Gemini structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Gemini's JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback (can be shared or made global if needed)
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GEMINI_CLIENT_HPP
