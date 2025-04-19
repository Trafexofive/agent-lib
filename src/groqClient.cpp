#include "../inc/Groq.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <cstdlib> // For getenv
#include <stdexcept>
#include <iostream> // For potential debug logging
#include <memory>   // For unique_ptr

// Libcurl write callback
size_t GroqClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor implementation
GroqClient::GroqClient(const std::string& apiKey) :
    // Sensible defaults for Groq
    m_model("llama3-8b-8192"), // Example Groq model
    m_temperature(0.5),
    m_maxTokens(4096),
    m_baseUrl("https://api.groq.com/openai/v1") // OpenAI-compatible endpoint base
{
    if (!apiKey.empty()) {
        m_apiKey = apiKey;
    } else {
        const char* envKey = std::getenv("GROQ_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0') {
            m_apiKey = envKey;
             std::cout << "[INFO] GroqClient: Using API key from GROQ_API_KEY environment variable." << std::endl;
        } else {
            std::cerr << "[WARN] GroqClient: API key not provided via constructor or GROQ_API_KEY env var. API calls will likely fail." << std::endl;
            // throw std::invalid_argument("Groq API key required: Provide via constructor or GROQ_API_KEY env var");
        }
    }
    // curl_global_init in main()
}

// Configuration Setters
void GroqClient::setApiKey(const std::string& apiKey) { m_apiKey = apiKey; }
void GroqClient::setModel(const std::string& model) { m_model = model; }
void GroqClient::setTemperature(double temperature) { m_temperature = temperature; }
void GroqClient::setMaxTokens(int maxTokens) { m_maxTokens = maxTokens; }
void GroqClient::setBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }

// Generate implementation (Overrides base class)
std::string GroqClient::generate(const std::string& prompt) {
     if (m_apiKey.empty()) {
        throw ApiError("Groq API key is not set.");
    }
    // Groq uses the chat completions endpoint
    std::string url = m_baseUrl + "/chat/completions";

    // Build JSON payload specific to Groq/OpenAI Chat Completions API
    Json::Value root;
    Json::Value message;
    Json::Value messagesArray(Json::arrayValue); // Array for messages

    // Simple structure: only one user message
    message["role"] = "user";
    message["content"] = prompt;
    messagesArray.append(message);

    root["messages"] = messagesArray;
    root["model"] = m_model;
    root["temperature"] = m_temperature;
    root["max_tokens"] = m_maxTokens;
    // Add other Groq/OpenAI-specific parameters if needed (e.g., top_p, stream)

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = ""; // Compact output
    std::string payload = Json::writeString(writerBuilder, root);

    // std::cout << "[DEBUG] GroqClient Request URL: " << url << std::endl;
    // std::cout << "[DEBUG] GroqClient Request Payload: " << payload << std::endl;

    try {
        std::string responseBody = performHttpRequest(url, payload);
        // std::cout << "[DEBUG] GroqClient Response Body: " << responseBody << std::endl;
        return parseJsonResponse(responseBody);
    } catch (const ApiError& e) {
        std::cerr << "[ERROR] GroqClient API Error: " << e.what() << std::endl;
        throw; // Re-throw API specific errors
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GroqClient Request Failed: " << e.what() << std::endl;
        throw ApiError(std::string("Groq request failed: ") + e.what()); // Wrap other errors
    } catch (...) {
         std::cerr << "[ERROR] GroqClient Unknown request failure." << std::endl;
         throw ApiError("Unknown error during Groq request.");
    }
}


// HTTP request helper (Note the Authorization header)
std::string GroqClient::performHttpRequest(const std::string& url, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string readBuffer;
    long http_code = 0;
    struct curl_slist* headers = nullptr;

    // Groq/OpenAI typically uses Bearer token authorization
    std::string authHeader = "Authorization: Bearer " + m_apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header_list(headers, curl_slist_free_all);


    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    CURLcode res = curl_easy_perform(curl);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup);

    if (res != CURLE_OK) {
        throw ApiError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        std::ostringstream errMsg;
        errMsg << "HTTP Error: " << http_code;
        errMsg << " | Response: " << readBuffer.substr(0, 500);
        if (readBuffer.length() > 500) errMsg << "...";
        throw ApiError(errMsg.str());
    }

    return readBuffer;
}

// JSON parsing helper (Adapted for Groq/OpenAI Chat Completions response)
std::string GroqClient::parseJsonResponse(const std::string& jsonResponse) const {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(jsonResponse.c_str(), jsonResponse.c_str() + jsonResponse.length(), &root, &errors);

    if (!parsingSuccessful) {
        throw ApiError("Failed to parse Groq JSON response: " + errors);
    }

    // Check for OpenAI-style error structure first
    if (root.isMember("error") && root["error"].isObject()) {
         std::string errorMsg = "API Error: ";
         if (root["error"].isMember("message") && root["error"]["message"].isString()) {
            errorMsg += root["error"]["message"].asString();
         } else if (root["error"].isMember("type") && root["error"]["type"].isString()){
             errorMsg += root["error"]["type"].asString(); // Include error type if message is missing
         } else {
             errorMsg += Json::writeString(Json::StreamWriterBuilder(), root["error"]);
         }
         throw ApiError(errorMsg);
    }
     // Also handle top-level 'detail' which Groq sometimes uses for errors
     if (root.isMember("detail") && root["detail"].isString()){
        throw ApiError("API Error Detail: " + root["detail"].asString());
     }


    // Navigate the expected Groq/OpenAI success structure
    try {
        // Structure: root -> choices[0] -> message -> content
        if (root.isMember("choices") && root["choices"].isArray() && !root["choices"].empty()) {
            const Json::Value& firstChoice = root["choices"][0u];
            if (firstChoice.isMember("message") && firstChoice["message"].isObject()) {
                const Json::Value& message = firstChoice["message"];
                if (message.isMember("content") && message["content"].isString()) {
                    return message["content"].asString();
                }
            }
             // Check finish reason if content is missing/null
             if (firstChoice.isMember("finish_reason") && firstChoice["finish_reason"].asString() != "stop") {
                 throw ApiError("Content generation finished unexpectedly. Reason: " + firstChoice["finish_reason"].asString());
             }
        }
        // If structure wasn't as expected
         throw ApiError("Could not extract content from Groq API response structure. Response: " + jsonResponse.substr(0, 500) + "...");

    } catch (const Json::Exception& e) {
        throw ApiError(std::string("JSON access error while parsing Groq response: ") + e.what());
     } catch (const ApiError& e) {
        throw;
    } catch (const std::exception& e) {
         throw ApiError(std::string("Standard exception while parsing Groq response: ") + e.what());
    }
}
