#pragma once // Prevent multiple inclusions

#include <iostream>
#include <memory> // For unique_ptr
#include <regex>  // For basic HTML entity decoding
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For parsing the JSON input parameters and response
#include <curl/curl.h> // For making HTTP requests

// --- Helper: libcurl Write Callback ---
static size_t ddgWriteCallback(void *contents, size_t size, size_t nmemb,
                               void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// --- Helper: URL Encode ---
std::string ddgUrlEncode(const std::string &value) {
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

// --- Helper: Basic HTML Entity Decode ---
std::string basicHtmlDecode(std::string text) {
  text = std::regex_replace(text, std::regex("&"), "&");
  text = std::regex_replace(text, std::regex("<"), "<");
  text = std::regex_replace(text, std::regex(">"), ">");
  text = std::regex_replace(text, std::regex("""), """);
  text = std::regex_replace(text, std::regex("'"), "'");
  return text;
}

// --- Helper: Extract text between tags ---
std::string extractText(const std::string &html, size_t startPos) {
  size_t tagEnd = html.find('>', startPos);
  if (tagEnd == std::string::npos) return "";
  size_t textStart = tagEnd + 1;
  size_t textEnd = html.find('<', textStart);
  if (textEnd == std::string::npos) return "";
  return basicHtmlDecode(html.substr(textStart, textEnd - textStart));
}

// --- Tool Implementation (Returns JSON String) ---
std::string duckduckgoSearchTool(const Json::Value &params) {
  Json::Value response; // JSON object for the response

  // 1. Validate Parameters
  if (!params.isObject()) {
    response["status"] = "error";
    response["result"] = "Error: duckduckgoSearchTool requires a JSON object.";
    return response.toStyledString();
  }
  if (!params.isMember("query") || !params["query"].isString() ||
      params["query"].asString().empty()) {
    response["status"] = "error";
    response["result"] = "Error: Missing or invalid required non-empty string parameter 'query'.";
    return response.toStyledString();
  }

  std::string query = params["query"].asString();
  int max_results = 5;
  if (params.isMember("num_results") && params["num_results"].isInt()) {
    max_results = params["num_results"].asInt();
    if (max_results <= 0 || max_results > 10) {
      max_results = 5;
    }
  }

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Query='" << query
            << "', Aiming for max " << max_results << " results." << std::endl;

  // 2. Prepare Request
  std::string encodedQuery = ddgUrlEncode(query);
  if (encodedQuery.empty() && !query.empty()) {
    response["status"] = "error";
    response["result"] = "Error: Failed to URL-encode the search query.";
    return response.toStyledString();
  }

  std::string url = "https://html.duckduckgo.com/html/?q=" + encodedQuery;
  CURL *curl = curl_easy_init();
  if (!curl) {
    response["status"] = "error";
    response["result"] = "Error: Failed to initialize libcurl for DuckDuckGo search.";
    return response.toStyledString();
  }
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup);

  std::string readBuffer;
  long http_code = 0;
  const char *userAgent =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36";

  try {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ddgWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    // 3. Perform HTTP Request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      response["status"] = "error";
      response["result"] = "Error: DuckDuckGo search network request failed: " + std::string(curl_easy_strerror(res));
      return response.toStyledString();
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
      std::stringstream errMsg;
      errMsg << "Error: DuckDuckGo search returned HTTP status " << http_code << ".";
      if (!readBuffer.empty()) {
        errMsg << " Response snippet: " << readBuffer.substr(0, 200) << "...";
      }
      response["status"] = "error";
      response["result"] = errMsg.str();
      return response.toStyledString();
    }

  } catch (const std::exception &e) {
    response["status"] = "error";
    response["result"] = "Error: Exception during DuckDuckGo network operation: " + std::string(e.what());
    return response.toStyledString();
  }

  // 4. Parse HTML
  std::stringstream output;
  output << "DuckDuckGo Search Results for '" << query << "':
";
  int results_found = 0;
  size_t current_pos = 0;
  const std::string result_block_start = "result--web";
  const std::string link_marker = "result__a";
  const std::string snippet_marker = "result__snippet";
  const std::string href_marker = "href="";

  while (results_found < max_results) {
    size_t block_start = readBuffer.find(result_block_start, current_pos);
    if (block_start == std::string::npos) break;
    size_t link_start = readBuffer.find(link_marker, block_start);
    if (link_start == std::string::npos) {
      current_pos = block_start + result_block_start.length();
      continue;
    }

    // Extract URL
    std::string result_url = "N/A";
    size_t href_start = readBuffer.find(href_marker, link_start);
    if (href_start != std::string::npos) {
      href_start += href_marker.length();
      size_t href_end = readBuffer.find('"', href_start);
      if (href_end != std::string::npos) {
        result_url = readBuffer.substr(href_start, href_end - href_start);
        size_t uddg_param = result_url.find("uddg=");
        if (uddg_param != std::string::npos) {
          std::string encoded_target = result_url.substr(uddg_param + 5);
          std::string decoded_url;
          CURL *temp_curl = curl_easy_init();
          int outlen;
          char *decoded = curl_easy_unescape(temp_curl, encoded_target.c_str(), encoded_target.length(), &outlen);
          if (decoded) {
            decoded_url.assign(decoded, outlen);
            curl_free(decoded);
            result_url = decoded_url;
          }
          curl_easy_cleanup(temp_curl);
        }
        result_url = basicHtmlDecode(result_url);
      }
    }

    // Extract Title
    std::string title = extractText(readBuffer, link_start);
    if (title.empty()) title = "N/A";

    // Extract Snippet
    std::string snippet = "N/A";
    size_t snippet_start = readBuffer.find(snippet_marker, link_start);
    if (snippet_start != std::string::npos) {
      size_t tag_end = readBuffer.find('>', snippet_start);
      if (tag_end != std::string::npos) {
        size_t text_start = tag_end + 1;
        size_t text_end = readBuffer.find('<', text_start);
        if (text_end != std::string::npos) {
          snippet = readBuffer.substr(text_start, text_end - text_start);
          snippet.erase(0, snippet.find_first_not_of(" 	
"));
          snippet.erase(snippet.find_last_not_of(" 	
") + 1);
          snippet = basicHtmlDecode(snippet);
        }
      }
    }

    output << "
---
";
    output << "Title: " << title << "
";
    output << "URL: " << result_url << "
";
    output << "Snippet: " << snippet << "
";
    results_found++;
    current_pos = (snippet_start != std::string::npos)
                      ? (snippet_start + snippet_marker.length())
                      : (link_start + link_marker.length());
  }

  if (results_found == 0) {
    output << "
(No results found or failed to parse HTML)
";
  }

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Found and formatted "
            << results_found << " results." << std::endl;

  // 5. Prepare JSON success response
  response["status"] = "success";
  response["result"] = output.str();
  return response.toStyledString();
}
