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

// --- Forward Declarations for External Tool Functions ---
extern std::string executeBashCommandReal(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);
extern std::string duckduckgoSearchTool(const Json::Value &params);
extern std::string fileTool(const Json::Value &params);
extern std::string getTime(const Json::Value &params);
extern std::string calculate(const Json::Value &params);
extern std::string webSearchTool(const Json::Value &params);
extern std::string writeFileTool(const Json::Value &params);
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

// --- Orchestrator Setup and Run Function ---
int startOrchestrator(const std::string &apiKey, Agent *noteAgent) {
  logMessageMain(LogLevel::INFO, "--- Starting Orchestrator Agent ---");
  std::vector<Tool *> orchestratorTools;
  try {
    MiniGemini orchestratorApi(apiKey);
    Agent orchestratorAgent(orchestratorApi);
    orchestratorAgent.setName("Orchestrator");
    orchestratorAgent.setDescription("Top-level agent coordinating tasks.");

    if (noteAgent) {
      orchestratorAgent.addAgent(noteAgent);
      logMessageMain(LogLevel::INFO,
                     "Registered 'NoteMaster' with Orchestrator.");
    }

    // ** UPDATED Orchestrator System Prompt **
    orchestratorAgent.setSystemPrompt(
        R"(SYSTEM PROMPT: Orchestrator Agent (Named: Demurge)

**Role:** Central coordinator agent (basically the master agent if you will). Serve the Master (Named:CleverLord) by translating high-level goals into executable workflows, leveraging sub-agents and tools. Aim for 10x-100x amplification of the Master's capabilities (Jarvis/Alfred vision).

**Core Interaction Model:**
You MUST respond with a single JSON object containing the following fields:
1.  `thought`: (String) Your reasoning, analysis of the current situation, and plan for the next step(s). Be concise but clear.
2.  `tool_calls`: (Array of Objects | null) A list of tools to execute *now*. Each object in the array MUST be formatted EXACTLY as: `{"tool_name": "...", "params": { ... }}`. If no tools need to be called, this should be `null` or an empty array `[]`.
3.  `final_response`: (String | null) The final answer or response to the user/Master. Provide this ONLY when the entire task is complete and no further actions are needed. If more steps (tool calls, thinking) are required, this MUST be `null`.

**Operational Guidelines:**
*   **Decomposition:** Break down complex requests from the Master into logical steps.
*   **Tool Usage:** Use available tools via the `tool_calls` field. Prioritize specific tools over `bash`. Use `bash` cautiously. Use `help` if unsure about tool parameters.
*   **Delegation:** Use the `promptAgent` tool in `tool_calls` to delegate tasks to specialized agents. That said if the task at hand isnt big enough, there is no need for of loading to sub-agents (Again. use tools at hand smartly to achieve the desired result). Provide clear context in the `prompt` parameter.
*   **Agent Creation:** Use the `createAgent` tool to instantiate new agents from YAML configs when needed. (not implemented yet)
*   **Synthesis:** If previous steps involved tool calls or agent delegation, use your `thought` field to analyze the results (present in history) and plan the next step or formulate the `final_response`.
*   **Context & Inference:** Utilize conversation history and available memory to understand context, infer missing details ("the nitty gritty gritty"), and anticipate needs. Minimize clarification requests.
*   **Workflow Control:** If a task requires multiple LLM iterations (e.g., read file -> process -> write file), use `tool_calls` for the first action(s) and set `final_response` to `null`. The results will appear in the history for your next turn. Use the `skip` tool *only* if you explicitly need the system to pause *before* generating the next LLM response after tool execution (rarely needed with this structured format).
*   **Error Handling:** If a tool call fails (indicated in history), your `thought` should analyze the error, and your next action could be to retry, use a different tool, or inform the Master in `final_response` if the task cannot be completed.

**Example Response Format (Calling a tool):**
```json
{
  "thought": "The Master wants the weather in London. I need to use the getWeather tool.",
  "tool_calls": [
    {
      "tool_name": "getWeather",
      "params": { "location": "London" }
    }
  ],
  "final_response": null
}


Example Response Format (Final Answer):

{
  "thought": "I received the weather for London from the tool. The task is complete, I can now provide the final response.",
  "tool_calls": null,
  "final_response": "The current weather in London is [Weather Details from Tool Result]."
}
IGNORE_WHEN_COPYING_START
content_copy
download
Use code with caution. 
Json
IGNORE_WHEN_COPYING_END

Adhere strictly to this JSON response format. Orchestrate effectively.
)");

    // --- Register Tools for Orchestrator ---
    Tool *timeTool =
        new Tool("time", "Get the current system date and time. Parameters: {}",
                 getTime);
    orchestratorAgent.addTool(timeTool);
    orchestratorTools.push_back(timeTool);

    Tool *calcTool = new Tool("calc",
                              "Calculate a simple arithmetic expression. "
                              "Parameters: {\"expression\": \"string\"}",
                              calculate);
    orchestratorAgent.addTool(calcTool);
    orchestratorTools.push_back(calcTool);

    Tool *webSearch = new Tool(
        "web",
        "Performs web search. Parameters: {\"query\": \"string\", "
        "\"num_results\": int?, \"search_engine\": \"google\"|\"duckduckgo\"?}",
        webSearchTool);
    orchestratorAgent.addTool(webSearch);
    orchestratorTools.push_back(webSearch);

    Tool *ddgSearch =
        new Tool("ddg_search",
                 "[Alt Web Search] Uses HTML scraping. Parameters: {\"query\": "
                 "\"string\", \"num_results\": int?}",
                 duckduckgoSearchTool);
    orchestratorAgent.addTool(ddgSearch);
    orchestratorTools.push_back(ddgSearch);

    Tool *fileOpsTool = new Tool(
        "file",
        "File system operations (read, write, list, info, delete, mkdir). "
        "Params: {'action': string, 'path': string, 'content': string?}. "
        "Relative paths assumed unless $NOTES is used.",
        fileTool);
    orchestratorAgent.addTool(fileOpsTool);
    orchestratorTools.push_back(fileOpsTool);

    Tool *bashTool = new Tool("bash",
                              "Executes raw Bash command. Params: "
                              "{\"command\": \"string\"}",
                              executeBashCommandReal);
    orchestratorAgent.addTool(bashTool);
    orchestratorTools.push_back(bashTool);

    // TODO: Add Agent Factory Tool registration here once implemented
    // extern std::string createAgentFromYamlTool(const Json::Value& params,
    // Agent& callingAgent); Tool* agentFactoryTool = new Tool("createAgent",
    // "Creates agent from YAML. Params: {\"config_path\": \"string\"}",
    // createAgentFromYamlTool, orchestratorAgent);
    // orchestratorAgent.addTool(agentFactoryTool);
    // orchestratorTools.push_back(agentFactoryTool);

    logMessageMain(LogLevel::INFO, "Tools registered with Orchestrator.");
    orchestratorAgent.run();

  } catch (const ApiError &e) {
    logMessageMain(LogLevel::ERROR, "Orchestrator API Error:", e.what());
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  } catch (const std::exception &e) {
    logMessageMain(LogLevel::ERROR, "Orchestrator Error:", e.what());
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  } catch (...) {
    logMessageMain(LogLevel::ERROR, "Unknown error in Orchestrator.");
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  }

  for (Tool *tool : orchestratorTools)
    delete tool;
  logMessageMain(LogLevel::INFO, "--- Orchestrator Agent Finished ---");
  return 0;
}

// --- Main Application Entry Point ---
int main() {
  CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (curl_global_res != CURLE_OK) {
    std::cerr << "Fatal: Failed to initialize libcurl globally: "
              << curl_easy_strerror(curl_global_res) << std::endl;
    return 1;
  }
  logMessageMain(LogLevel::INFO, "libcurl initialized.");

  std::string apiKey;
  Agent *noteAgentPtr = nullptr;
  std::vector<Tool *> noteMasterTools;

  try {
    logMessageMain(LogLevel::INFO, "--- Setting up NoteMaster Agent ---");

    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
      logMessageMain(LogLevel::ERROR,
                     "GEMINI_API_KEY environment variable not set or empty.");
      curl_global_cleanup();
      return 1;
    }
    apiKey = apiKeyEnv;
    logMessageMain(LogLevel::INFO, "API Key loaded.");

    MiniGemini noteApiClient(apiKey);
    logMessageMain(LogLevel::INFO, "NoteMaster API client initialized.");

    noteAgentPtr = new Agent(noteApiClient);
    noteAgentPtr->setName("NoteMaster");
    noteAgentPtr->setDescription(
        "Specialized agent for managing notes ($NOTES), calendar events.");
    // Ensure this directory exists or the agent can create it.
    noteAgentPtr->addEnvVar("NOTES", "/home/mlamkadm/notes");
    logMessageMain(LogLevel::INFO, "Agent 'NoteMaster' created.");

    // ** UPDATED NoteMaster System Prompt **
    noteAgentPtr->setSystemPrompt(R"(SYSTEM PROMPT: NoteMaster Agent

Role: Automated agent managing notes and calendar events within the $NOTES directory.

Core Interaction Model:
You MUST respond with a single JSON object containing the following fields:

thought: (String) Your reasoning about the request and plan.

tool_calls: (Array of Objects | null) Tools to execute now, each formatted as {"tool_name": "...", "params": { ... }}. Use null or [] if no tools are needed.

final_response: (String | null) The final response to the caller (usually Orchestrator). Provide ONLY when the task is fully complete. Set to null if more steps/tools are needed.

Operational Guidelines:

Focus: Primarily use the file tool for CRUD operations on notes within the $NOTES directory and the calendar tool for events. Use full paths when possible, leveraging the $NOTES env var.

Execution: Execute requests from the Orchestrator or Master precisely. Infer intent for missing details; do not ask for clarification.

Tool Format: Adhere strictly to the {"tool_name": "...", "params": { ... }} format within the tool_calls array.

Response: Ensure final_response is non-null only upon task completion. If calling tools, final_response MUST be null.

Error Handling: If a tool fails, note it in your thought and potentially inform the caller via final_response if the task cannot proceed.

Example Response (Adding a calendar event):

{
  "thought": "Request received to add a calendar event. I will use the 'calendar' tool with the 'add' action.",
  "tool_calls": [
    {
      "tool_name": "calendar",
      "params": {
        "action": "add",
        "date": "YYYY-MM-DD", // Fill in calculated date
        "time": "14:00",
        "description": "Team Meeting"
      }
    }
  ],
  "final_response": null
}

Example Response (After successful tool call):

{
  "thought": "The calendar tool reported success in adding the event. The task is complete.",
  "tool_calls": null,
  "final_response": "Success: Calendar event 'Team Meeting' added for YYYY-MM-DD at 14:00."
}

)");
    logMessageMain(LogLevel::INFO, "System prompt set for NoteMaster.");

    // Register Tools for NoteMaster
    Tool *file = new Tool("file",
                          "File system operations (read, write, list, info, "
                          "delete, mkdir) within $NOTES env var. Params: {'action': "
                          "string, 'path': string, 'content': string?}",
                          fileTool);
    noteAgentPtr->addTool(file);
    noteMasterTools.push_back(file);

    Tool *calTool = new Tool(
        "calendar",
        "Manages calendar events (add, list). Params: {'action': 'add'|'list', "
        "'date': 'YYYY-MM-DD', 'time': 'HH:MM'?, 'description': string?}",
        calendarTool);
    noteAgentPtr->addTool(calTool);
    noteMasterTools.push_back(calTool);

    // Bash tool potentially restricted for NoteMaster
    // Tool* noteBashTool = new Tool("bash", "[DANGEROUS] Bash commands ONLY
    // within $NOTES. Params: {\"command\": \"string\"}",
    // executeBashCommandReal); noteAgentPtr->addTool(noteBashTool);
    // noteMasterTools.push_back(noteBashTool);

    logMessageMain(LogLevel::INFO, "Tools registered with NoteMaster.");

  } catch (const ApiError &e) {
    logMessageMain(LogLevel::ERROR, "NoteMaster Setup API Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::invalid_argument &e) {
    logMessageMain(LogLevel::ERROR,
                   "NoteMaster Setup Initialization Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::exception &e) {
    logMessageMain(LogLevel::ERROR,
                   "NoteMaster Setup Unexpected Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (...) {
    logMessageMain(LogLevel::ERROR, "NoteMaster Setup Unknown Fatal Error.");
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  }

  // Start Orchestrator
  int orchestratorStatus = startOrchestrator(apiKey, noteAgentPtr);

  // Cleanup
  logMessageMain(LogLevel::INFO, "Cleaning up resources...");
  for (Tool *tool : noteMasterTools)
    delete tool;
  if (noteAgentPtr)
    delete noteAgentPtr;
  curl_global_cleanup();
  logMessageMain(LogLevel::INFO, "Application Shutting Down.");
}

// **Key Prompt Changes:**
//
// 1.  **Explicit JSON Structure:** Both prompts now explicitly demand a single
// JSON object response with `thought`, `tool_calls`, and `final_response`
// fields.
// 2.  **Field Explanations:** Each field's purpose is clearly defined
// (`thought` for reasoning, `tool_calls` for actions, `final_response` only
// when done).
// 3.  **Tool Call Format within JSON:** The prompts specify that the
// `tool_calls` array must contain objects formatted as `{"tool_name": "...",
// "params": { ... }}`.
// 4.  **Conditional Logic:** Instructions clarify that `final_response` should
// be `null` if `tool_calls` are present, and vice-versa (enforcing the idea
// that the agent either thinks/acts *or* gives a final answer in one turn).
// 5.  **Workflow/Multi-Step Handling:** The prompts explain how to handle
// multi-step tasks by using `tool_calls` and setting `final_response` to
// `null`, relying on the history/context for the next iteration. The role of
// the `skip` tool is de-emphasized slightly as the structured format provides
// better control.
// 6.  **Error Handling Guidance:** Briefly mentions how to handle tool errors
// (analyze in `thought`, potentially inform in `final_response`).
// 7.  **Role-Specific Instructions:** The prompts retain their specific focus
// (Orchestrator coordinates/delegates, NoteMaster manages notes/calendar in
// $NOTES).
// 8.  **Examples:** Clear examples of the expected JSON output format are
// provided for both tool calls and final responses.
//
// These updated prompts should guide the LLM much more effectively to produce
// output compatible with the enhanced `Agent::prompt` loop logic discussed
// previously. Remember that the `Agent::prompt` function itself needs the
// corresponding parsing logic (`parseStructuredLLMResponse`) to handle this new
// format.
