/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI Assistant <ai@assistant.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 22:39:30 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/04/23 01:30:00 by AI Assistant      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp"
#include "../inc/modelApi.hpp" // For ApiError
#include "httplib.h"           // HTTP server library
#include <cstdlib>             // For std::getenv
#include <ctime>               // For time functions
#include <iostream>
#include <json/json.h> // JSON library
#include <memory>      // For std::unique_ptr
#include <stdexcept>
#include <vector>      // For storing tool unique_ptrs
#include <curl/curl.h> // For curl_global_init/cleanup

// --- Forward Declarations for External Tool Functions ---
// (Ensure these match functions DEFINED in the corresponding .cpp files)
extern std::string executeBashCommandReal(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);
extern std::string duckduckgoSearchTool(const Json::Value &params);
extern std::string fileTool(const Json::Value &params); // Assumes the robust version
extern std::string getTime(const Json::Value &params);
extern std::string calculate(const Json::Value &params);
extern std::string webSearchTool(const Json::Value &params);
extern std::string localSearchTool(const Json::Value&params); // Define if using
extern std::string writeFileTool(const Json::Value &params); // Often redundant if fileTool is used
extern std::string swayControlTool(const Json::Value& params); // If using sway

// --- Simple Logger ---
void serverLog(const std::string &level, const std::string &message, const std::string& details = "") {
    std::time_t now = std::time(nullptr);
    std::tm *tm_local = std::localtime(&now);
    char time_buffer[20]; // HH:MM:SS
    std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm_local);

    std::string color_start = "";
    std::string color_end = "\033[0m"; // ANSI Reset color
    std::string prefix_str;

    if (level == "FATAL" || level == "ERROR") { color_start = "\033[1;31m"; prefix_str = "[" + level + "] "; } // Bold Red
    else if (level == "WARN") { color_start = "\033[33m"; prefix_str = "[WARN]  "; } // Yellow
    else if (level == "INFO") { color_start = "\033[32m"; prefix_str = "[INFO]  "; } // Green
    else if (level == "DEBUG") { color_start = "\033[36m"; prefix_str = "[DEBUG] "; } // Cyan
    else if (level == "REQUEST") { color_start = "\033[34m"; prefix_str = "[REQ]   "; } // Blue
    else if (level == "RESPONSE") { color_start = "\033[35m"; prefix_str = "[RESP]  "; } // Magenta
    else { prefix_str = "[" + level + "] "; }

    std::ostream &out = (level == "ERROR" || level == "FATAL" || level == "WARN") ? std::cerr : std::cout;
    out << color_start << std::string(time_buffer) << " " << prefix_str << message << color_end << std::endl;
    if (!details.empty()) {
        const size_t max_detail_len = 500;
        std::string truncated_details = details.substr(0, max_detail_len) + (details.length() > max_detail_len ? "... (truncated)" : "");
        out << color_start << "  | " << truncated_details << color_end << std::endl;
    }
}

// --- RAII Helper for curl_global_cleanup ---
struct CurlGlobalCleanupGuard {
    ~CurlGlobalCleanupGuard() {
        serverLog("INFO", "Calling curl_global_cleanup().");
        curl_global_cleanup();
    }
    CurlGlobalCleanupGuard(const CurlGlobalCleanupGuard&) = delete;
    CurlGlobalCleanupGuard& operator=(const CurlGlobalCleanupGuard&) = delete;
    CurlGlobalCleanupGuard() = default;
};

int main() {
    // --- Global Initialization ---
    CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_global_res != CURLE_OK) {
        serverLog("FATAL", "Failed to initialize libcurl globally: " + std::string(curl_easy_strerror(curl_global_res)));
        return 1;
    }
    serverLog("INFO", "libcurl initialized.");
    CurlGlobalCleanupGuard curl_guard; // RAII cleanup

    // --- Configuration ---
    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
        serverLog("FATAL", "GEMINI_API_KEY environment variable not set or empty.");
        return 1;
    }
    std::string apiKey(apiKeyEnv);
    serverLog("INFO", "API Key loaded from environment variable.");
    const std::string host = "0.0.0.0";
    const int port = 7777; // Make sure this matches the port in app.js API_ENDPOINT

    // --- Agent and Tool Storage ---
    std::unique_ptr<MiniGemini> orchestratorApiClient;
    std::unique_ptr<Agent> orchestratorAgent;
    std::unique_ptr<MiniGemini> noteApiClient;
    std::unique_ptr<Agent> noteMasterAgent;
    std::vector<std::unique_ptr<Tool>> managedTools; // RAII for tools

    try {
        // --- Initialize NoteMaster Agent (Mirrors main.cpp setup) ---
        serverLog("INFO", "--- Setting up NoteMaster Agent ---");
        noteApiClient = std::make_unique<MiniGemini>(apiKey);
        serverLog("INFO", "NoteMaster API client initialized.");

        noteMasterAgent = std::make_unique<Agent>(*noteApiClient);
        noteMasterAgent->setName("NoteMaster");
        noteMasterAgent->setDescription("Specialized agent for managing notes ($NOTES) and calendar events.");
        // Use the AGENT_WORKSPACE env var used by the robust fileTool, or set it here
        // setenv("AGENT_WORKSPACE", "/home/mlamkadm/notes", 1); // Uncomment to force workspace
        // noteMasterAgent->addEnvVar("NOTES", "/home/mlamkadm/notes"); // Redundant if fileTool uses AGENT_WORKSPACE
        serverLog("INFO", "Agent 'NoteMaster' created.");

        noteMasterAgent->setSystemPrompt(R"(SYSTEM PROMPT: NoteMaster Agent
Role: Automated agent managing notes and calendar events within the designated workspace (usually AGENT_WORKSPACE).
Core Interaction Model: Respond ONLY with a single JSON object: {"thought": string, "tool_calls": array|null, "final_response": string|null}.
Focus: Primarily use the 'file' tool for notes and 'calendar' tool for events. Use relative paths.
Execution: Execute requests precisely. Infer intent. Avoid clarification.
Tool Format: Use STRICT {"tool_name": "...", "params": { ... }} format in 'tool_calls'.
Response: 'final_response' is ONLY for task completion. If calling tools, it MUST be null.
Error Handling: Note tool failures in 'thought', inform caller via 'final_response' if task cannot proceed.
)");
        serverLog("INFO", "System prompt set for NoteMaster.");

        // Register Tools for NoteMaster
        auto fileToolInstance = std::make_unique<Tool>(
            "file",
            "File system operations (read, write, list, info, delete, mkdir) within the agent's workspace. "
            "Params: {'action': string, 'path': string (RELATIVE path ONLY), 'content': string? for write}. ",
            fileTool); // Use the secure version
        noteMasterAgent->addTool(fileToolInstance.get());
        managedTools.push_back(std::move(fileToolInstance));

        auto calendarToolInstance = std::make_unique<Tool>(
            "calendar",
            "Manages calendar events (add, list). Params: {'action': 'add'|'list', 'date': 'YYYY-MM-DD', 'time': 'HH:MM'?, 'description': string?}",
            calendarTool);
        noteMasterAgent->addTool(calendarToolInstance.get());
        managedTools.push_back(std::move(calendarToolInstance));

        serverLog("INFO", "Tools registered with NoteMaster.");

        // --- Initialize Orchestrator Agent (Mirrors main.cpp setup) ---
        serverLog("INFO", "--- Setting up Orchestrator Agent ---");
        orchestratorApiClient = std::make_unique<MiniGemini>(apiKey);
        orchestratorApiClient->setModel("gemini-1.5-flash-latest");
        serverLog("INFO", "Orchestrator API client initialized.");

        orchestratorAgent = std::make_unique<Agent>(*orchestratorApiClient);
        orchestratorAgent->setName("Orchestrator");
        orchestratorAgent->setDescription("Top-level agent coordinating tasks and delegating to sub-agents like NoteMaster.");
        serverLog("INFO", "Agent 'Orchestrator' created.");

        orchestratorAgent->setSystemPrompt(R"(SYSTEM PROMPT: Orchestrator Agent (Named: Demurge)
Role: Central coordinator agent. Translate high-level goals into workflows using sub-agents and tools. Serve the Master (CleverLord).
Core Interaction Model: Respond ONLY with a single JSON object: {"thought": string, "tool_calls": array|null, "final_response": string|null}.
Guidelines:
- Decomposition: Break down complex requests.
- Tool Usage: Use available tools (`time`, `calc`, `web`, `ddg_search`, `file`, `bash`) via `tool_calls`. Use `bash` cautiously. Use `help`.
- Delegation: Use `promptAgent` tool to delegate tasks to agents (e.g., `NoteMaster`). Provide clear context. Delegate complex/specialized tasks, use direct tools for simple ones.
- Synthesis: Analyze history/tool results in `thought` to plan next step or `final_response`.
- Context & Inference: Use history/memory to infer details. Minimize clarification.
- Workflow Control: Use `tool_calls` for actions, set `final_response` to `null`. Results appear in history. `skip` is rarely needed.
- Error Handling: Analyze tool failures in `thought`, retry/alternative/inform via `final_response`.
Format: STRICT JSON response format mandatory.
)");
        serverLog("INFO", "System prompt set for Orchestrator.");

        // Register Tools for Orchestrator
        auto timeToolInstance = std::make_unique<Tool>("time", "Get current system date/time. Params: {}", getTime);
        orchestratorAgent->addTool(timeToolInstance.get());
        managedTools.push_back(std::move(timeToolInstance));

        auto calcToolInstance = std::make_unique<Tool>("calc", "Calculate simple expression. Params: {\"expression\": \"string\"}", calculate);
        orchestratorAgent->addTool(calcToolInstance.get());
        managedTools.push_back(std::move(calcToolInstance));

        auto webSearchToolInstance = std::make_unique<Tool>("web", "Web search. Params: {\"query\": string, \"num_results\": int?, \"search_engine\": \"google\"|\"duckduckgo\"?}", webSearchTool);
        orchestratorAgent->addTool(webSearchToolInstance.get());
        managedTools.push_back(std::move(webSearchToolInstance));

        auto ddgSearchToolInstance = std::make_unique<Tool>("ddg_search", "[Alt Web Search] HTML scraping. Params: {\"query\": string, \"num_results\": int?}", duckduckgoSearchTool);
        orchestratorAgent->addTool(ddgSearchToolInstance.get());
        managedTools.push_back(std::move(ddgSearchToolInstance));

        // Add the shared fileTool instance pointer to Orchestrator
        Tool* fileToolPtr = nullptr;
        for(const auto& tool : managedTools) {
            if(tool->getName() == "file") {
                fileToolPtr = tool.get();
                break;
            }
        }
        if (fileToolPtr) {
            orchestratorAgent->addTool(fileToolPtr);
            serverLog("INFO", "Shared 'file' tool registered with Orchestrator.");
        } else {
            serverLog("WARN", "Could not find 'file' tool instance to share with Orchestrator.");
        }

        auto bashToolInstance = std::make_unique<Tool>("bash", "Execute Bash command. CAUTION ADVISED. Params: {\"command\": \"string\"}", executeBashCommandReal);
        orchestratorAgent->addTool(bashToolInstance.get());
        managedTools.push_back(std::move(bashToolInstance));

        // Add localSearch if needed
        // auto localSearchToolInstance = std::make_unique<Tool>("localSearch", "Searches text in local files. Params: {'query': string, 'base_path': string}", localSearchTool);
        // orchestratorAgent->addTool(localSearchToolInstance.get());
        // managedTools.push_back(std::move(localSearchToolInstance));

        // --- Link Agents ---
        orchestratorAgent->addAgent(noteMasterAgent.get());
        serverLog("INFO", "Registered 'NoteMaster' with Orchestrator.");

        serverLog("INFO", "All agents initialized successfully.");

        // --- Initialize HTTP Server ---
        httplib::Server svr;

        // --- Define Endpoints ---

        // == CORS Preflight Handler (OPTIONS) ==
        // Handles browser checks before the actual POST request is sent from a different origin.
        svr.Options("/prompt", [](const httplib::Request &, httplib::Response &res) {
            // Allow requests specifically from the frontend server origin
            // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
            res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com");
            // Allow the Content-Type header (needed for sending JSON)
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            // Allow the POST method (and OPTIONS itself)
            res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.status = 204; // Standard success response for OPTIONS preflight
            serverLog("REQUEST", "OPTIONS /prompt (CORS preflight)");
        });

        // == Main Prompt Endpoint (POST) ==
        // Processes the actual user prompts sent from the frontend.
        svr.Post("/prompt", [&](const httplib::Request &req, httplib::Response &res) {
            // Allow requests specifically from the frontend server origin for the actual request
            // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
            res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com");
            // If you need broader access during development, uncomment the line below and comment out the line above
            // res.set_header("Access-Control-Allow-Origin", "*");

            serverLog("REQUEST", "POST /prompt from " + req.remote_addr);

            // 1. Parse Request
            Json::Value requestJson;
            Json::CharReaderBuilder readerBuilder; // Using CharReaderBuilder for modern jsoncpp
            std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
            std::string errors;

            if (!reader->parse(req.body.c_str(), req.body.c_str() + req.body.length(), &requestJson, &errors)) {
                serverLog("ERROR", "Failed to parse request JSON", errors);
                res.status = 400;
                res.set_content("{\"error\": \"Invalid JSON format\"}", "application/json");
                return;
            }

            // 2. Extract Prompt
            if (!requestJson.isMember("prompt") || !requestJson["prompt"].isString()) {
                serverLog("ERROR", "Missing or invalid 'prompt' field");
                res.status = 400;
                res.set_content("{\"error\": \"'prompt' field (string) is required\"}", "application/json");
                return;
            }
            std::string userPrompt = requestJson["prompt"].asString();
            serverLog("INFO", "User prompt received", userPrompt.substr(0, 100) + (userPrompt.length() > 100 ? "..." : ""));

            // 3. Interact with Orchestrator Agent
            std::string agentFinalResponse;
            try {
                // Ensure the agent pointer is valid before calling
                if (!orchestratorAgent) {
                    throw std::runtime_error("Orchestrator agent is not initialized.");
                }
                agentFinalResponse = orchestratorAgent->prompt(userPrompt);
                serverLog("DEBUG", "Agent final response generated", agentFinalResponse.substr(0, 200) + (agentFinalResponse.length() > 200 ? "..." : ""));
            } catch (const ApiError& e) {
                 serverLog("ERROR", "Agent API error processing prompt", e.what());
                 res.status = 500;
                 // Use Json::valueToQuotedString for safe JSON string embedding
                 res.set_content("{\"error\": \"Agent API interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
                 return;
            } catch (const std::exception &e) {
                serverLog("ERROR", "Agent processing error", e.what());
                res.status = 500;
                res.set_content("{\"error\": \"Agent interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
                return;
            } catch (...) {
                serverLog("ERROR", "Unknown agent processing error");
                res.status = 500;
                res.set_content("{\"error\": \"Unknown agent interaction failed\"}", "application/json");
                return;
            }

            // 4. Format and Send Response
            Json::Value responseJson;
            responseJson["response"] = agentFinalResponse; // Ensure 'response' key matches RESPONSE_KEY in app.js
            Json::StreamWriterBuilder writerBuilder; // Use StreamWriterBuilder for modern jsoncpp
            writerBuilder["indentation"] = ""; // Optional: Use "" for compact output, or "  " for pretty print
            std::string responseBody = Json::writeString(writerBuilder, responseJson);

            res.status = 200;
            res.set_content(responseBody, "application/json");
            serverLog("RESPONSE", "Sent 200 OK for /prompt");
        });

        // == Health Check Endpoint ==
        svr.Get("/health", [](const httplib::Request &, httplib::Response &res) {
            // Allow requests from anywhere for health check
            res.set_header("Access-Control-Allow-Origin", "*");
            res.status = 200;
            res.set_content("{\"status\": \"OK\"}", "application/json");
            serverLog("REQUEST", "GET /health");
        });

        // --- Start Server ---
        serverLog("INFO", "Starting server on " + host + ":" + std::to_string(port));
        if (!svr.listen(host.c_str(), port)) {
            serverLog("FATAL", "Failed to start server on " + host + ":" + std::to_string(port));
            return 1;
        }

    } catch (const ApiError &e) {
        serverLog("FATAL", "API Error during initialization", e.what());
        return 1;
    } catch (const std::exception &e) {
        serverLog("FATAL", "Initialization failed", e.what());
        return 1;
    } catch (...) {
        serverLog("FATAL", "Unknown initialization error.");
        return 1;
    }

    // Server runs until stopped (e.g., Ctrl+C)
    serverLog("INFO", "Server stopped.");
    // Resources managed by unique_ptr and curl_guard cleaned up automatically
    return 0;
} // <-- curl_guard destructor called here
