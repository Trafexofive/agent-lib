#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json/json.h"

// --- Helper: libcurl Write Callback ---
static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// --- Helper: URL Encode ---
std::string urlEncode(const std::string &value) {
    CURL *curl = curl_easy_init();
    if (!curl) return "";
    char *output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    if (!output) {
        curl_easy_cleanup(curl);
        return "";
    }
    std::string result(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    return result;
}

// --- Tool Implementation ---
std::string webSearchTool(const Json::Value &params) {
    // 1. Validate Parameters
    if (!params.isObject()) {
        return "Error: webSearchTool requires a JSON object as input.";
    }
    if (!params.isMember("query") || !params["query"].isString() || params["query"].asString().empty()) {
        return "Error: Missing or invalid required non-empty string parameter 'query'.";
    }

    std::string query = params["query"].asString();
    int num_results = 3; // Default
    if (params.isMember("num_results") && params["num_results"].isInt()) {
        num_results = params["num_results"].asInt();
        if (num_results <= 0) {
            num_results = 3;
        }
    }
    std::string search_engine = "google";
    if (params.isMember("search_engine") && params["search_engine"].isString()) {
        search_engine = params["search_engine"].asString();
    }
    std::string url;
    if (search_engine == "google") {
        url = "https://www.google.com/search?q=" + urlEncode(query);
    } else if (search_engine == "duckduckgo") {
        url = "https://html.duckduckgo.com/html/?q=" + urlEncode(query);
    } else {
        return "Error: Invalid search engine: " + search_engine + ". Supported engines: google, duckduckgo";
    }
    CURL *curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize libcurl.";
    }
    std::string readBuffer;

    try {
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            return "Error: Web search failed: " + std::string(curl_easy_strerror(res));
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            curl_easy_cleanup(curl);
            return "Error: Web search returned HTTP " + std::to_string(http_code);
        }

        curl_easy_cleanup(curl);

        // Basic result parsing (This will need improvement!)
        std::stringstream output;
        output << "Web Search Results for '" << query << "':\n";
        output << "Retrieved from: " << url << "\n";
        output << "Raw response (first 200 chars):\n" << readBuffer.substr(0, 200) << "...\n"; // For debugging
        return output.str();

    } catch (const std::exception &e) {
        curl_easy_cleanup(curl);
        return "Error: Exception during web search: " + std::string(e.what());
    }
}
