/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI Assistant <ai@assistant.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 22:39:30 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/04/24 14:30:00 by AI Assistant      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp"
#include "../inc/modelApi.hpp" // For ApiError
#include "httplib.h"           // HTTP server library
#include <cstdlib>             // For std::getenv
#include <ctime>               // For time functions, std::time, std::localtime, std::strftime
#include <iostream>
#include <json/json.h> // JSON library
#include <memory>      // For std::unique_ptr, std::make_unique
#include <stdexcept>   // For std::runtime_error, std::exception
#include <string>
#include <vector>      // For storing tool unique_ptrs
#include <curl/curl.h> // For curl_global_init/cleanup

// --- Forward Declarations for External Tool Functions ---
extern std::string executeBashCommandReal(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);
extern std::string fileTool(const Json::Value &params);
extern std::string getTime(const Json::Value &params);
extern std::string calculate(const Json::Value &params);
// extern std::string localSearchTool(const Json::Value¶ms); // Define if using
// extern std::string writeFileTool(const Json::Value ¶ms); // Often redundant
extern std::string swayControlTool(const Json::Value& params); // If using sway

// --- Simple Logger ---
void serverLog(const std::string &level, const std::string &message, const std::string& details = ""){
    std::time_t now = std::time(nullptr);
    // Use thread-safe localtime_r or localtime_s
    std::tm tm_local_buf;
    #ifdef _WIN32
        localtime_s(&tm_local_buf, &now);
        std::tm* tm_local = &tm_local_buf;
    #else
        std::tm* tm_local = localtime_r(&now, &tm_local_buf);
    #endif

    if (!tm_local) { std::cerr << "Error getting time!" << std::endl; return; }

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

// ============================================================================
// == Agent & Tool Setup Functions                                         ==
// ============================================================================

// Creates and configures the NoteMaster Agent
std::unique_ptr<Agent> setupNoteMasterAgent(
    const std::string& apiKey,
    std::vector<std::unique_ptr<Tool>>& managedTools)
{
    serverLog("INFO", "--- Setting up NoteMaster Agent ---");
    // Create API client first because Agent takes it by reference
    auto noteApiClient = std::make_unique<MiniGemini>(apiKey);
    serverLog("INFO", "NoteMaster API client initialized.");

    auto noteAgent = std::make_unique<Agent>(*noteApiClient); // Pass API client reference
    noteAgent->setAgentName("NoteMaster");
    noteAgent->setAgentDescription("Specialized agent for managing notes (in workspace) and calendar events.");
    noteAgent->setIterationCap(5); // Example: Lower iteration cap

    // Updated NoteMaster System Prompt (using new schema)
    noteAgent->setSystemPrompt(R"(SYSTEM PROMPT: NoteMaster Agent

Role: Manage notes within the workspace and calendar events. Execute specific tasks given by the Orchestrator.

Interaction Model:
Respond ONLY with a single JSON object containing:
- thoughts (array of strings): Reasoning about the file/calendar action.
- DIRECTIVE (string): e.g., "FILE_OPERATION", "CALENDAR_UPDATE", "TASK_COMPLETE", "TASK_FAILED".
- state_metadata (object | null): Optional confidence, urgency, completion %. (Usually confidence=1.0 for direct tasks).
- context_assessment (object | null): Optional. Note if required path/details are missing.
- knowledge_query (object | null): Set to null. Do not query memory unless explicitly asked.
- execution (array of objects | null): REQUIRED if executing. Contains ONLY 'file' or 'calendar' tool calls. Format: {step_id, action_type: "TOOL_CALL", tool_name, params}. NULL if task complete/failed definitively.
- execution_mode (string | null): Usually "SEQUENTIAL_STOP_ON_FAIL".
- expected_outcome (string | null): Goal of the file/calendar action.
- is_done (boolean): True ONLY when the specific file/calendar task is fully completed or has definitively failed.
- response (string | null): Confirmation/result message for the Orchestrator (e.g., "Success: File content retrieved.", "Success: Event added.", "Error: File '...' not found."). ONLY populate if is_done is true.

Guidelines:
- Focus: Use 'file' and 'calendar' tools based on the Orchestrator's request. Assume relative paths.
- Execution: Follow instructions precisely. Do NOT ask for clarification; report failure if instructions are unclear/impossible.
- Error Handling: If a tool call fails, set DIRECTIVE to TASK_FAILED, is_done to true, and provide an error message in 'response'.
)");
    serverLog("INFO", "System prompt set for NoteMaster.");

    // Register Tools for NoteMaster (using existing external functions)
    auto fileToolInstance = std::make_unique<Tool>(
        "file",
        "File system operations (read, write, list, info, delete, mkdir). Params: {'action': string, 'path': string (RELATIVE ONLY), 'content': string? for write}",
        fileTool);
    noteAgent->addTool(fileToolInstance.get()); // Add raw pointer to agent
    managedTools.push_back(std::move(fileToolInstance)); // Store owner pointer

    auto calendarToolInstance = std::make_unique<Tool>(
        "calendar",
        "Manages calendar events (add, list). Params: {'action': 'add'|'list', 'date': 'YYYY-MM-DD', 'time': 'HH:MM'?, 'description': string?}",
        calendarTool);
    noteAgent->addTool(calendarToolInstance.get());
    managedTools.push_back(std::move(calendarToolInstance));

    serverLog("INFO", "Tools registered with NoteMaster.");
    serverLog("INFO", "--- NoteMaster Agent Setup Complete ---");
    // Transfer ownership of the API client to the agent if Agent class manages it,
    // otherwise, the API client needs to be managed separately (e.g., in main).
    // Assuming Agent DOES NOT take ownership here. The caller (main) needs to keep noteApiClient alive.
    // If Agent DOES take ownership: noteAgent->setApiClient(std::move(noteApiClient));
    return noteAgent; // Return the unique_ptr
}

// Creates and configures the Orchestrator Agent
std::unique_ptr<Agent> setupOrchestratorAgent(
    const std::string& apiKey,
    Agent* noteMasterAgentPtr, // Pass raw pointer to NoteMaster
    std::vector<std::unique_ptr<Tool>>& managedTools)
{
    serverLog("INFO", "--- Setting up Orchestrator Agent ---");
    // Create API client first
    auto orchestratorApiClient = std::make_unique<MiniGemini>(apiKey);
    orchestratorApiClient->setModel("gemini-1.5-flash-latest"); // Or another capable model
    serverLog("INFO", "Orchestrator API client initialized.");

    auto orchestratorAgent = std::make_unique<Agent>(*orchestratorApiClient); // Pass API client reference
    orchestratorAgent->setAgentName("Orchestrator");
    orchestratorAgent->setAgentDescription("Top-level agent coordinating tasks and delegating to sub-agents like NoteMaster.");
    orchestratorAgent->setIterationCap(15); // Higher cap for coordinator

    // Updated Orchestrator System Prompt (using new schema)
    orchestratorAgent->setSystemPrompt(R"(SYSTEM PROMPT: Orchestrator Agent (Named: Demurge)

Role: Central coordinator. Decompose user goals into plans, manage context, query knowledge, execute local tools, or delegate tasks to specialized agents (e.g., 'NoteMaster').

Interaction Model:
Respond ONLY with a single JSON object containing:
- thoughts (array of strings): Decompose the goal, analyze results, plan next step(s).
- DIRECTIVE (string): Current phase (e.g., "PLANNING", "CONTEXT_ASSESSMENT", "KNOWLEDGE_RETRIEVAL", "DELEGATING_TASK", "TOOL_EXECUTION", "PROCESSING_RESULTS", "RESPONSE_GENERATION", "TASK_COMPLETE", "TASK_FAILED").
- state_metadata (object | null): Confidence, urgency, completion %.
- context_assessment (object | null): Assess required context (keys/info) vs available context. Set 'missing_critical_info' to true if needed before proceeding.
- knowledge_query (object | null): If internal information (from memory/history) is needed FIRST, specify {query_string, target_memory, max_results}. Execution MUST be null if this is present.
- execution (array of objects | null): Actions for THIS agent (bash, web search, etc.) OR delegation calls using 'promptAgent'. Format: {step_id, action_type: "TOOL_CALL", tool_name, params}. NULL if querying knowledge or finishing. Use 'promptAgent' tool here to delegate to sub-agents like NoteMaster.
- execution_mode (string | null): How to run 'execution' array (e.g., "SEQUENTIAL_STOP_ON_FAIL", "SEQUENTIAL_BEST_EFFORT").
- expected_outcome (string | null): Goal of the knowledge query or execution block.
- is_done (boolean): True ONLY when the overall user goal is fully achieved.
- response (string | null): Final response TO THE USER (only populate if is_done is true).

Guidelines:
- Planning: Use 'thoughts' to break down the user's request into logical steps.
- Context: Use 'context_assessment' to check if you have needed info. Use 'knowledge_query' to retrieve info from memory BEFORE deciding on tools/delegation.
- Tools vs Delegation: Use local tools ('time', 'calc', 'web', 'ddg_search', 'file' [if essential & not notes], 'bash' [rarely]) for simple tasks. For note/calendar tasks, DELEGATE using the 'promptAgent' tool within 'execution', targeting 'NoteMaster'. Provide a clear task description in 'promptAgent' params.
- Synthesis: After knowledge query or execution results appear in history, analyze them in your next 'thoughts' to decide the subsequent step.
- Completion: Only set 'is_done' to true and populate 'response' when the *entire* user request is satisfied. Intermediate confirmations/results should be processed internally via 'thoughts'.
- Error Handling: If a local tool, knowledge query, or delegation fails (check history), note it in 'thoughts', set appropriate DIRECTIVE (e.g., ERROR_RECOVERY), and decide whether to retry, use an alternative, or report failure to the user (setting is_done:true, response:error message).
)");
    serverLog("INFO", "System prompt set for Orchestrator.");

    // Register Tools for Orchestrator
    auto timeToolInstance = std::make_unique<Tool>("time", "Get current system date/time. Params: {}", getTime);
    orchestratorAgent->addTool(timeToolInstance.get());
    managedTools.push_back(std::move(timeToolInstance));

    auto calcToolInstance = std::make_unique<Tool>("calc", "Calculate simple expression. Params: {\"expression\": \"string\"}", calculate);
    orchestratorAgent->addTool(calcToolInstance.get());
    managedTools.push_back(std::move(calcToolInstance));



    // Find the file tool added by NoteMaster setup
    Tool* fileToolPtr = nullptr;
    for(const auto& tool : managedTools) {
        if(tool && tool->getName() == "file") {
            fileToolPtr = tool.get();
            break;
        }
    }
    if (fileToolPtr) {
        // Decide if Orchestrator needs direct file access. Usually better to delegate.
        // orchestratorAgent->addTool(fileToolPtr);
        // serverLog("INFO", "Shared 'file' tool potentially available to Orchestrator (use delegation preferably).");
    } else {
        serverLog("WARN", "Could not find 'file' tool instance to potentially share with Orchestrator.");
    }

    auto bashToolInstance = std::make_unique<Tool>("bash", "Execute Bash command. CAUTION ADVISED. Params: {\"command\": \"string\"}", executeBashCommandReal);
    orchestratorAgent->addTool(bashToolInstance.get());
    managedTools.push_back(std::move(bashToolInstance));

    // Register NoteMaster as a sub-agent if provided
    if (noteMasterAgentPtr) {
        orchestratorAgent->registerSubAgent(noteMasterAgentPtr); // Use correct method name
        serverLog("INFO", "Registered 'NoteMaster' with Orchestrator.");
    } else {
        serverLog("WARN", "NoteMaster agent pointer was null, could not register as sub-agent.");
    }

    serverLog("INFO", "--- Orchestrator Agent Setup Complete ---");
    // Again, assuming Agent doesn't take ownership of the API client ptr.
    return orchestratorAgent;
}

// ============================================================================
// == HTTP Server Setup & Run Functions                                      ==
// ============================================================================

// Configures the httplib::Server instance passed by reference
void setupHttpServer(httplib::Server& svr, Agent& agent) { // Takes svr by reference

    // --- Define Endpoints ---

    // CORS Preflight Handler
    svr.Options("/prompt", [](const httplib::Request &, httplib::Response &res) {
        // TODO: Make origin configurable
        // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
         res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com"); // Example domain
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept"); // Added Accept
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.status = 204;
        serverLog("REQUEST", "OPTIONS /prompt (CORS preflight)");
    });

    // Main Prompt Endpoint
    // Capture agent by reference
    svr.Post("/prompt", [&](const httplib::Request &req, httplib::Response &res) {
        // TODO: Make origin configurable
        // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
         res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com"); // Example domain

        serverLog("REQUEST", "POST /prompt from " + req.remote_addr);

        // 1. Parse Request JSON
        Json::Value requestJson;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
        std::string errors;
        if (!reader->parse(req.body.c_str(), req.body.c_str() + req.body.length(), &requestJson, &errors)) {
            serverLog("ERROR", "Failed to parse request JSON", errors);
            res.status = 400; // Bad Request
            // Use .c_str() for valueToQuotedString
            res.set_content("{\"error\": \"Invalid JSON format\", \"details\": " + Json::valueToQuotedString(errors.c_str()) + "}", "application/json");
            return;
        }

        // 2. Extract User Prompt
        if (!requestJson.isMember("prompt") || !requestJson["prompt"].isString()) {
            serverLog("ERROR", "Missing or invalid 'prompt' field");
            res.status = 400; // Bad Request
            res.set_content("{\"error\": \"'prompt' field (string) is required\"}", "application/json");
            return;
        }
        std::string userPrompt = requestJson["prompt"].asString();
        serverLog("INFO", "User prompt received", userPrompt.substr(0, 100) + (userPrompt.length() > 100 ? "..." : ""));

        // 3. Interact with the Agent (using the captured reference)
        std::string agentFinalResponse;
        try {
            agentFinalResponse = agent.processPrompt(userPrompt); // Use correct method name
            serverLog("DEBUG", "Agent final response generated", agentFinalResponse.substr(0, 200) + (agentFinalResponse.length() > 200 ? "..." : ""));
        } catch (const ApiError& e) {
            serverLog("ERROR", "Agent API error processing prompt", e.what());
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Agent API interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
            return;
        } catch (const std::exception &e) {
            serverLog("ERROR", "Agent processing error", e.what());
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Agent interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
            return;
        } catch (...) {
            serverLog("ERROR", "Unknown agent processing error");
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Unknown agent interaction failed\"}", "application/json");
            return;
        }

        // 4. Format and Send Response
        Json::Value responseJson;
        responseJson["response"] = agentFinalResponse; // Key expected by frontend
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // Compact JSON for API
        std::string responseBody = Json::writeString(writerBuilder, responseJson);

        res.status = 200; // OK
        res.set_content(responseBody, "application/json");
        serverLog("RESPONSE", "Sent 200 OK for /prompt");
    });

    // Health Check Endpoint
    svr.Get("/health", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*"); // Allow from anywhere
        res.status = 200;
        res.set_content("{\"status\": \"OK\"}", "application/json");
        serverLog("REQUEST", "GET /health");
    });

    // No return needed as svr is modified by reference
}

// Starts the HTTP server listening loop
bool startHttpServer(httplib::Server& svr, const std::string& host, int port) {
    serverLog("INFO", "Starting server on " + host + ":" + std::to_string(port));
    if (!svr.listen(host.c_str(), port)) {
        serverLog("FATAL", "Failed to start server on " + host + ":" + std::to_string(port));
        return false;
    }
    // svr.listen blocks until stopped
    serverLog("INFO", "Server has stopped listening.");
    return true;
}

// ============================================================================
// == Main Function                                                          ==
// ============================================================================

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
    const std::string host = "0.0.0.0"; // Listen on all interfaces
    const int port = 7777;

    // --- Agent and Tool Storage ---
    // unique_ptrs manage the lifetime of agents and owned tools
    std::vector<std::unique_ptr<Tool>> managedTools;
    // API Clients need to outlive the Agents that use them by reference
    std::unique_ptr<MiniGemini> noteApiClientPtr;
    std::unique_ptr<MiniGemini> orchestratorApiClientPtr;
    std::unique_ptr<Agent> noteMasterAgent;
    std::unique_ptr<Agent> orchestratorAgent;
    // Create server object here, setup happens later
    httplib::Server svr;

    try {
        // --- Setup Agents & Tools ---
        // Create API clients first (will be passed by reference to agents)
        noteApiClientPtr = std::make_unique<MiniGemini>(apiKey);
        orchestratorApiClientPtr = std::make_unique<MiniGemini>(apiKey);

        // Now setup agents, passing the API client references
        noteMasterAgent = setupNoteMasterAgent(apiKey, managedTools); // apiKey is redundant if client passed? Maybe refactor setup funcs
        orchestratorAgent = setupOrchestratorAgent(apiKey, noteMasterAgent.get(), managedTools);

        if (!orchestratorAgent) {
             throw std::runtime_error("Failed to initialize Orchestrator Agent.");
        }
        serverLog("INFO", "All agents initialized successfully.");

        // --- Setup HTTP Server ---
        // Pass orchestratorAgent by reference
        setupHttpServer(svr, *orchestratorAgent);
        serverLog("INFO", "HTTP server configured.");

        // --- Start Server ---
        // This call will block until the server is stopped (e.g., Ctrl+C)
        if (!startHttpServer(svr, host, port)) {
            return 1; // Exit if server fails to start
        }

    } catch (const ApiError &e) {
        serverLog("FATAL", "API Error during initialization or runtime", e.what());
        // unique_ptrs will handle cleanup on exit
        return 1;
    } catch (const std::exception &e) {
        serverLog("FATAL", "Standard Exception during initialization or runtime", e.what());
         // unique_ptrs will handle cleanup on exit
        return 1;
    } catch (...) {
        serverLog("FATAL", "Unknown error occurred.");
         // unique_ptrs will handle cleanup on exit
        return 1;
    }

    // Server runs until stopped (e.g., Ctrl+C in startHttpServer -> svr.listen)
    serverLog("INFO", "Server process ended.");
    // Resources (agents via unique_ptr; tools via unique_ptr in vector; curl via RAII guard) cleaned up automatically
    return 0;
}

