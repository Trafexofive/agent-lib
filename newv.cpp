
// main.cpp

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

// --- Logging Function ---
void logMessageMain(LogLevel level, const std::string &message,
                    const std::string &details = "") {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  char time_buffer[20];
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &now_tm);

  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m";

  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    color_start = "\033[36m";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    color_start = "\033[32m";
    break;
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    color_start = "\033[33m";
    break;
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    color_start = "\033[1;31m";
    break;
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    color_start = "\033[1;35m";
    break;
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    color_start = "\033[35m";
    break;
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    color_start = "\033[34m";
    break;
  }

  std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN)
                          ? std::cerr
                          : std::cout; out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    out << color_start << "  " << details.substr(0, 500)
        << (details.length() > 500 ? "..." : "") << color_end << std::endl;
  }
}
// --- Main Application Entry Point ---
int main() {
  CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (curl_global_res != CURLE_OK) {
    std::cerr << "Fatal: Failed to initialize libcurl globally: "
              << curl_easy_strerror(curl_global_res) << std::endl;
    return 1;
  }
  // logMessageMain(LogLevel::INFO, "libcurl initialized.");

  std::string apiKey;
  Agent *noteAgentPtr = nullptr;
  std::vector<Tool *> noteMasterTools;

  try {
    // logMessageMain(LogLevel::INFO, "--- Setting up NoteMaster Agent ---");

    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
      // logMessageMain(LogLevel::ERROR,
      //                "GEMINI_API_KEY environment variable not set or empty.");
      curl_global_cleanup();
      return 1;
    }
    apiKey = apiKeyEnv;
    // logMessageMain(LogLevel::INFO, "API Key loaded.");

    MiniGemini noteApiClient(apiKey);
    logMessageMain(LogLevel::INFO, "NoteMaster API client initialized.");
    // noteAgentPtr = new Agent(noteApiClient, "./agent_config.yaml");


  } catch (const ApiError &e) {
    // logMessageMain(LogLevel::ERROR, "NoteMaster Setup API Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::invalid_argument &e) {
    // logMessageMain(LogLevel::ERROR,
    //                "NoteMaster Setup Initialization Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::exception &e) {
    // logMessageMain(LogLevel::ERROR,
    //                "NoteMaster Setup Unexpected Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (...) {
    // logMessageMain(LogLevel::ERROR, "NoteMaster Setup Unknown Fatal Error.");
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  }

  // Start Orchestrator
  // int orchestratorStatus = startOrchestrator(apiKey, noteAgentPtr);

  // Cleanup
  // logMessageMain(LogLevel::INFO, "Cleaning up resources...");
  for (Tool *tool : noteMasterTools)
    delete tool;
  if (noteAgentPtr)
    delete noteAgentPtr;
  curl_global_cleanup();
  // logMessageMain(LogLevel::INFO, "Application Shutting Down.");
}
