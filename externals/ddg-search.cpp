#pragma once // Prevent multiple inclusions

#include <iostream>
#include <memory> // For unique_ptr
#include <regex>  // For basic HTML entity decoding
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For parsing the JSON input parameters
#include <curl/curl.h> // For making HTTP requests

// --- Helper: libcurl Write Callback (Can likely be shared if put in a common
// header) ---
static size_t ddgWriteCallback(void *contents, size_t size, size_t nmemb,
                               void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// --- Helper: URL Encode (Can likely be shared) ---
std::string ddgUrlEncode(const std::string &value) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return "";
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
// Very basic, only handles a few common entities. A proper library is better.
std::string basicHtmlDecode(std::string text) {
  // Order matters for things like &lt;
  text = std::regex_replace(text, std::regex("&"), "&");
  text = std::regex_replace(text, std::regex("<"), "<");
  text = std::regex_replace(text, std::regex(">"), ">");
    text = std::regex_replace(text, std::regex("\""), "\"");
    text = std::regex_replace(text, std::regex("'"), "'");
    // Add more entities if needed
    return text;
}

// --- Helper: Extract text between tags (VERY basic) ---
std::string extractText(const std::string &html, size_t startPos) {
  size_t tagEnd = html.find('>', startPos);
  if (tagEnd == std::string::npos)
    return "";
  size_t textStart = tagEnd + 1;
  size_t textEnd = html.find('<', textStart);
  if (textEnd == std::string::npos)
    return ""; // Malformed?
  return basicHtmlDecode(html.substr(textStart, textEnd - textStart));
}

// --- Tool Implementation ---

// Tool Function: Performs a web search using DuckDuckGo HTML endpoint scraping.
// Input: JSON object like: {"query": "search terms" [, "num_results": N
// (optional, best effort)]} Output: Formatted string with search results
// (title, url, snippet) or an error message. WARNING: Relies on HTML scraping,
// which is fragile and may break without notice.
std::string duckduckgoSearchTool(const Json::Value &params) {

  // 1. Validate Parameters
  if (!params.isObject()) {
    return "Error: duckduckgoSearchTool requires a JSON object as input.";
  }
  if (!params.isMember("query") || !params["query"].isString() ||
      params["query"].asString().empty()) {
    return "Error: Missing or invalid required non-empty string parameter "
           "'query'.";
  }

  std::string query = params["query"].asString();
  int max_results = 5; // Default max results to aim for
  if (params.isMember("num_results") && params["num_results"].isInt()) {
    max_results = params["num_results"].asInt();
    if (max_results <= 0 ||
        max_results > 10) { // Keep requested count reasonable
      max_results = 5;
    }
  }
  // NOTE: We may get more/less results from the HTML page regardless of this
  // param.

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Query='" << query
            << "', Aiming for max " << max_results << " results." << std::endl;

  // 2. Prepare Request
  std::string encodedQuery = ddgUrlEncode(query);
  if (encodedQuery.empty() && !query.empty()) {
    return "Error: Failed to URL-encode the search query.";
  }

  // Use the non-JavaScript HTML endpoint
  std::string url = "https://html.duckduckgo.com/html/?q=" + encodedQuery;

  CURL *curl = curl_easy_init();
  if (!curl) {
    return "Error: Failed to initialize libcurl for DuckDuckGo search.";
  }
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(
      curl, curl_easy_cleanup);

  std::string readBuffer;
  long http_code = 0;
  // Use a common browser User-Agent
  const char *userAgent =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/91.0.4472.124 Safari/537.36";

  try {
    // Set CURL Options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ddgWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);       // 15 second timeout
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L); // Disable progress meter
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING,
                     ""); // Allow curl to handle encoding (like gzip)

    // 3. Perform API Call (Fetch HTML)
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      return "Error: DuckDuckGo search network request failed: " +
             std::string(curl_easy_strerror(res));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200) {
      std::stringstream errMsg;
      errMsg << "Error: DuckDuckGo search returned HTTP status " << http_code
             << ".";
      if (!readBuffer.empty()) {
        errMsg << " Response snippet: " << readBuffer.substr(0, 200) << "...";
      }
      return errMsg.str();
    }

  } catch (const std::exception &e) {
    return "Error: Exception during DuckDuckGo network operation: " +
           std::string(e.what());
  }
  // curl handle cleaned up automatically by unique_ptr

  // 4. Parse HTML (Basic and Fragile String Searching)
  std::stringstream output;
  output << "DuckDuckGo Search Results for '" << query << "':\n";
  int results_found = 0;
  size_t current_pos = 0;

  // Markers often used by DDG HTML results (these WILL change over time)
  const std::string result_block_start =
      "result--web"; // Often a class on a containing div
  const std::string link_marker = "result__a"; // Class for the main result link
  const std::string snippet_marker =
      "result__snippet"; // Class for the description snippet
  const std::string href_marker = "href=\"";

  while (results_found < max_results) {
    // Find the start of the next result block
    size_t block_start = readBuffer.find(result_block_start, current_pos);
    if (block_start == std::string::npos) {
      break; // No more result blocks found
    }

    // Find the link within this block
    size_t link_start = readBuffer.find(link_marker, block_start);
    if (link_start == std::string::npos) {
      current_pos =
          block_start + result_block_start.length(); // Move past this block
      continue;
    }

    // Extract URL
    size_t href_start = readBuffer.find(href_marker, link_start);
    std::string url = "N/A";
    if (href_start != std::string::npos) {
      href_start += href_marker.length();
      size_t href_end = readBuffer.find('"', href_start);
      if (href_end != std::string::npos) {
        // DDG URLs might be relative redirects, need cleaning
        url = readBuffer.substr(href_start, href_end - href_start);
        size_t uddg_param = url.find("uddg=");
        if (uddg_param != std::string::npos) {
          std::string encoded_target =
              url.substr(uddg_param + 5); // Length of "uddg="
          // Basic URL decode might be needed here, but often works without it
          url = ddgUrlEncode(
              encoded_target); // Re-use encode fn name, but it does decode %XX
          // Basic decode of %XX hex codes
          std::string decoded_url;
          CURL *temp_curl = curl_easy_init();
          int outlen;
          char *decoded = curl_easy_unescape(temp_curl, encoded_target.c_str(),
                                             encoded_target.length(), &outlen);
          if (decoded) {
            decoded_url.assign(decoded, outlen);
            curl_free(decoded);
            url = decoded_url;
          }
          curl_easy_cleanup(temp_curl);
        }
        url = basicHtmlDecode(url); // Decode entities like &
      }
    }

    // Extract Title (usually the link text)
    std::string title = extractText(readBuffer, link_start);
    if (title.empty())
      title = "N/A";

    // Find and Extract Snippet
    size_t snippet_start =
        readBuffer.find(snippet_marker, link_start); // Search after link
    std::string snippet = "N/A";
    if (snippet_start != std::string::npos) {
      // Find the actual text content after the snippet marker tag closes
      size_t tag_end = readBuffer.find('>', snippet_start);
      if (tag_end != std::string::npos) {
        size_t text_start = tag_end + 1;
        size_t text_end =
            readBuffer.find('<', text_start); // Find start of next tag
        if (text_end != std::string::npos) {
          snippet = readBuffer.substr(text_start, text_end - text_start);
          // Trim leading/trailing whitespace often present
          snippet.erase(0, snippet.find_first_not_of(" \t\n\r"));
          snippet.erase(snippet.find_last_not_of(" \t\n\r") + 1);
          snippet = basicHtmlDecode(snippet);
        }
      }
    }

    // Append formatted result
    output << "\n---\n";
    output << "Title: " << title << "\n";
    output << "URL: " << url << "\n";
    output << "Snippet: " << snippet << "\n";
    results_found++;

    // Move position past the current snippet to find the next block
    current_pos = (snippet_start != std::string::npos)
                      ? (snippet_start + snippet_marker.length())
                      : (link_start + link_marker.length());

  } // End while loop

  if (results_found == 0) {
    output << "\n(No results found or failed to parse HTML)\n";
  }

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Found and formatted "
            << results_found << " results." << std::endl;
  return output.str();
}
