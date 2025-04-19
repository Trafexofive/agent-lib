#pragma once
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

// --- Tool Implementation (Returns JSON String) ---
std::string webSearchTool(const Json::Value &params) {
    Json::Value response; // JSON for response

    // 1. Validate Parameters
    if (!params.isObject()) {
        response["status"] = "error";
        response["result"] = "Error: webSearchTool requires a JSON object as input.";
        return response.toStyledString();
    }
    if (!params.isMember("query") || !params["query"].isString() || params["query"].asString().empty()) {
        response["status"] = "error";
        response["result"] = "Error: Missing or invalid required non-empty string parameter 'query'.";
        return response.toStyledString();
    }

    std::string query = params["query"].asString();
    // Note: num_results is not used by this basic implementation, but we keep the param validation
    int num_results = 3; // Default
    if (params.isMember("num_results") && params["num_results"].isInt()) {
        num_results = params["num_results"].asInt();
        if (num_results <= 0 || num_results > 20) { // Limit results requested
             std::cerr << "[WARN] webSearchTool: num_results out of range (1-20), using default.";
             num_results = 3;
        }
    }

    std::string search_engine = "google"; // Default
    if (params.isMember("search_engine") && params["search_engine"].isString()) {
        search_engine = params["search_engine"].asString();
    }

    std::string url;
    // Important: This tool currently only uses DDG, ignoring the 'search_engine' param for URL choice.
    // The duckduckgoSearchTool in ddg-search.cpp provides actual DDG scraping.
    // This function is more of a placeholder or needs a real search API integration.
    // For demonstration, let's stick to the DDG HTML URL.
    url = "https://html.duckduckgo.com/html/?q=" + urlEncode(query);
     std::cout << "[INFO] webSearchTool using URL: " << url << std::endl;
    /*
    if (search_engine == "google") {
        // Google search scraping is very likely to be blocked. Requires API.
        url = "https://www.google.com/search?q=" + urlEncode(query);
    } else if (search_engine == "duckduckgo") {
        url = "https://html.duckduckgo.com/html/?q=" + urlEncode(query);
    } else {
        response["status"] = "error";
        response["result"] = "Error: Invalid search engine: " + search_engine + ". Supported (currently DDG only): duckduckgo";
        return response.toStyledString();
    }
    */

    CURL *curl = curl_easy_init();
    if (!curl) {
        response["status"] = "error";
        response["result"] = "Error: Failed to initialize libcurl.";
        return response.toStyledString();
    }
    std::string readBuffer;
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup);

    try {
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // Increased timeout
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response["status"] = "error";
            response["result"] = "Error: Web search network request failed: " + std::string(curl_easy_strerror(res));
            return response.toStyledString();
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            response["status"] = "error";
            response["result"] = "Error: Web search returned HTTP " + std::to_string(http_code) + ". URL: " + url;
            return response.toStyledString();
        }

        // Extremely basic result "parsing" - just returns a snippet of the raw HTML.
        // A real implementation needs proper HTML parsing (like in ddg-search.cpp) or API usage.
        std::stringstream output;
        output << "Web Search Results for '" << query << "' (from " << url << "):
";
        output << "Raw HTML Snippet (Max 500 chars):
" << readBuffer.substr(0, 500) << (readBuffer.length() > 500 ? "..." : "") << "
";
        output << "
[Note: This tool currently returns raw HTML. Use 'duckduckgoSearchTool' for parsed results.]";

        response["status"] = "success";
        response["result"] = output.str();
        return response.toStyledString();

    } catch (const std::exception &e) {
        response["status"] = "error";
        response["result"] = "Error: Exception during web search: " + std::string(e.what());
        return response.toStyledString();
    } catch (...) {
         response["status"] = "error";
        response["result"] = "Error: Unknown exception during web search.";
        return response.toStyledString();
    }
}
