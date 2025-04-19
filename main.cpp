#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "json/json.h"
#include <curl/curl.h>

#include "./externals/bash.cpp"
#include "./externals/cal-events.cpp"
#include "./externals/ddg-search.cpp"
#include "./externals/file.cpp"
#include "./externals/general.cpp"
#include "./externals/search.cpp"

extern std::string fileTool(const Json::Value &params);
extern std::string localSearchTool(const Json::Value &params);
extern std::string webSearchTool(const Json::Value &params);
extern std::string duckduckgoSearchTool(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);

void logMessageMain(LogLevel level, const std::string &message, const std::string &details = "") {
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char time_buffer[20];
    std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm);

    std::string prefix;
    std::string color_start = "";
    std::string color_end = "\033[0m";

    switch (level) {
        case LogLevel::DEBUG: prefix = "[DEBUG] "; color_start = "\033[36m"; break;
        case LogLevel::INFO:  prefix = "[INFO]  "; color_start = "\033[32m"; break;
        case LogLevel::WARN:  prefix = "[WARN]  "; color_start = "\033[33m"; break;
        case LogLevel::ERROR: prefix = "[ERROR] "; color_start = "\033[1;31m"; break;
        default:              prefix = "[LOG]   "; break;
    }

    std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN) ? std::cerr : std::cout;
    out << color_start << std::string(time_buffer) << " " << prefix << message << color_end << std::endl;

    if (!details.empty())
        out << "  " << details << std::endl;
}

static int orchestrator(const std::string &apiKey, Agent *note) {
  try {
    // Create API client (uses default config, checks key/env var)
    MiniGemini api(apiKey);

    Agent agent(api);

    agent.setName("Orchestrator Of Order & Light -Demurge");
    agent.addAgent(note); // Add the NoteMaster agent to the orchestrator
    // Set system prompt (mention tool format)
    agent.setSystemPrompt(
        "You are a powerful intuitive orchestrator assistant. Capable of "
        "interacting with tools and agents.\n"
        "Communicate with other agents and use tools to achieve the masters "
        "will.\n"
        "Do not ask for clarification infer from convo and fill the gaps (you "
        "are meant to be assistive to the max, giving a report/intuitive "
        "analysis based on the things done.).\n"
        "You have memory of past interaction/actions as well as the ability "
        "to skip (asking for one more compute iteration). You can only call "
        "tools/agents "
        "(strictly in the provided format below) you can even call multiple "
        "at "
        "the same time. tool_name is essential, if a tools are used and "
        "there "
        "is no output the tool_name is incorrect [INFO] all tools give some "
        "sort of feedback:\n"
        "==============================================\n"
        "```tool:{sample_tool_name (must be correct)}\n"
        "{\"param_name\": \"value\"}\n" // Use valid JSON for parameters [TIP]
                                        // tool calls can be stacked
        "```\n"
        "==============================================\n"
        "Dilimited in code by the following (tool_name is parsed after :):\n"
        "const std::string startMarker = \"```tool:\n\""
        "const std::string endMarker = \"```\"\n"
        "Only use the available tools. any other form of output that doesnt "
        "follow that format will be treated as a user reply.\n"
        "[IMPORTANT] Do not be shy about using the skip functionality, if it "
        "seems like there was an issue or mistake, feel free to "
        "rectify/debug. if the desired task/goal/purpose is not yet reached, "
        "keep trying, keep skipping. (do not stop and query the user.)\n"
        "[IMPORTANT] Again. You have a fully fledge bash tool, use that to "
        "get context (just be mindful of the history, cating files will leave "
        "them in history and by consequence they would flood the context), "
        "plan ahead and test ...\n");

    // Create Tools (simpler constructor - no schema string)
    Tool timeTool("time", "Get the current system date and time.", getTime);
    Tool calcTool("calc",
                  "Calculate a simple expression. single param expression : {1 "
                  "+ 2 / 2 * 2.5}",
                  calculate);

    Tool realBashTool(
        "bash",
        "Executes a real Bash command on the host system using \n popen and "
        "returns its combined stdout and stderr. Accepts a JSON \n object "
        "with "
        "a required name [bash] and 'command' parameter (string). Usage is "
        "fine since its in a controlled env\n."
        "[TIP] As a general rule of thumb. Anything that can be achieved using "
        "another tool should and always should be done using that tool. Of "
        "course, if a tool were to fail or malfuction or appear to be "
        "unreliable then using the bash tool is encouraged. \n",
        executeBashCommandReal);

    Tool file(
        "file",
        "Performs file system operations on relative paths. Actions:\n"
        "- 'read': Get content of a file.\n"
        "- 'write': Write/overwrite content to a file. Fails if path is a "
        "directory.\n" // Clarified write
        "- 'list': List contents of a directory.\n"
        "- 'info': Get metadata (type, size, modified time) for a path.\n"
        "- 'delete': Delete a file or an *empty* directory.\n" // Emphasized
                                                               // empty
        "- 'mkdir': Create a new directory. Fails if path exists or parent "
        "doesn't exist.\n" // Added mkdir
        "Input: JSON object {'action': string, 'path': string (relative), "
        "'content': string (required ONLY for 'write')}.\n"
        "Output: Varies by action (content, listing, info, success/error "
        "message).\n"
        "Constraints: Paths MUST be relative, safe (no '..', no unsafe chars).",
        fileTool // Function pointer to the implementation above
    );
    agent.addTool(&file);
    Tool web(
        "web",
        "Performs web search using DuckDuckGo. \nParams: JSON object "
        "{'query': 'search term', 'num_results': int (default 3), "
        "'search_engine': 'google'|'duckduckgo'}",
        webSearchTool// Function pointer from externals/ddg-search.cpp
    );
    agent.addTool(&web);

    // *** Add the tool ONLY if you fully accept the risks ***
    agent.addTool(&realBashTool);
    // Add Tools to Agent
    agent.addTool(&timeTool);
    agent.addTool(&calcTool);
    // agent.addTool(&memoryTool);
    // agent.addTool(&toolfile);

    // Run interactive loop
    agent.run();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    curl_global_cleanup();
    return 1;
  }
  return 0;
}

int main() {
  // 1. Initialize libcurl globally
  CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (curl_global_res != CURLE_OK) {
    std::cerr << "Fatal: Failed to initialize libcurl globally: "
              << curl_easy_strerror(curl_global_res) << std::endl;
    return 1;
  }

  logMessageMain(LogLevel::INFO, "Note Manager Agent Starting...");

  try {
    // 2. Get API Key for MiniGemini
    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
      logMessageMain(LogLevel::ERROR,
                     "GEMINI_API_KEY environment variable not set or empty.");
      curl_global_cleanup();
      return 1;
    }
    std::string apiKey(apiKeyEnv);

    // 3. Create MiniGemini API Client
    MiniGemini apiClient(apiKey);
    apiClient.setModel("gemini-2.0-flash"); // Set the model to use
    logMessageMain(LogLevel::INFO, "MiniGemini API client initialized.");

    // 4. Create the Agent
    Agent noteAgent(apiClient);
    noteAgent.setName("NoteMaster"); // Give the agent a name
    logMessageMain(LogLevel::INFO, "Agent 'NoteMaster' created.");

    // 5. Set System Prompt
    noteAgent.setSystemPrompt(
        "You are NoteMaster, a fully automated assistant specialized in "
        "managing notes, files, and information within the user's local "
        "environment. You manage only $NOTES"
        "You can a call on behalf of the user from another agent.\n"
        "Your primary goal is to help the user create, find, organize, "
        "summarize, and retrieve information from their notes and files, as "
        "well as create reports. "
        "Use the available tools precisely as documented to fulfill user "
        "requests. Do not ask any questions or seek clarifications. Infer any "
        "missing pieces and act to accomplish the task. You can keep a "
        "reference of any habits/preferences/ways-to-do-better in $NOTES/.data"
        "You can also manage calendar events which might be linked to dated "
        "notes or reminders."
        "\nAvailable Tool Formats:"
        "\n- JSON-based tools: Use ```tool:tool_name\n{\"param\": "
        "\"value\"}\n```"
        "\n- Text-based tools: Use ```tool:tool_name\ncontent in pure text "
        "format (depending on the tool description)\n```");
    logMessageMain(LogLevel::INFO, "System prompt set for NoteMaster.");

    noteAgent.addEnvVar("NOTES",
                        "/home/mlamkadm/notes"); // Set default notes directory
    noteAgent.setDescription(
        "NoteMaster - A fully automated assistant specialized in managing "
        "notes, files, and information within the user's local environment. "
        "Responsible for managing notes, files, and information within the assigned folder."
        "Resort to using this agent when needing to I/O note, calendars, events, to-do, new-tasks ...");

    Tool bashTool(
        "bash",
        "Executes a bash command. \nParams: Json object {'command': 'bash "
        "command to execute'}",
        executeBashCommandReal // Function pointer from externals/bash.cpp
    );
    noteAgent.addTool(&bashTool);
    Tool calendar("calendar",
                  "Manages calendar events. \nParams: Json object {'action': "
                  "'add'|'remove'|'list', 'event': 'event details'}",
                  calendarTool // Function pointer from externals/cal-events.cpp
    );
    noteAgent.addTool(&calendar);

    Tool file(
        "file",
        "Performs file system operations on relative paths. Actions:\n"
        "- 'read': Get content of a file.\n"
        "- 'write': Write/overwrite content to a file. Fails if path is a "
        "directory.\n" // Clarified write
        "- 'list': List contents of a directory.\n"
        "- 'info': Get metadata (type, size, modified time) for a path.\n"
        "- 'delete': Delete a file or an *empty* directory.\n" // Emphasized
                                                               // empty
        "- 'mkdir': Create a new directory. Fails if path exists or parent "
        "doesn't exist.\n" // Added mkdir
        "Input: JSON object {'action': string, 'path': string (relative), "
        "'content': string (required ONLY for 'write')}.\n"
        "Output: Varies by action (content, listing, info, success/error "
        "message).\n"
        "Constraints: Paths MUST be relative, safe (no '..', no unsafe chars).",
        fileTool // Function pointer to the implementation above
    );
    noteAgent.addTool(&file); // Add to NoteMaster
    // orchestratorAgent.addTool(&file); // Optionally add to Orchestrator too
    // Tool writeTool(
    //     "write",
    //     "text tool: Writes text content to a file. Tool Function: Writes "
    //     "content to a "
    //     "file specified by the path on the first line of input. Input : A "
    //     "string where the first line is the file path, and subsequent lines "
    //     "are the content.Output : A success message or an error message.",
    //     writeFileTool // Function pointer from externals/write.cpp
    // );
    // noteAgent.addTextTool(&writeTool);

    logMessageMain(LogLevel::INFO, "Tools registered with NoteMaster.");

    Json::Value params;

    // Example of using the bash tool directly
    // params["command"] = "cd ~/notes";
    //
    // noteAgent.manualToolCall("bash", params);
    // params.clear();
    // params["command"] = "tree --noreport -C -I .git";
    // noteAgent.manualToolCall("bash", params);
    // 7. Start the Agent's Interactive Loop
    // noteAgent.run();
    // call run from orchestrator scope
    orchestrator(apiKey, &noteAgent);

  } catch (const ApiError &e) {
    logMessageMain(LogLevel::ERROR, "API Error occurred:", e.what());
    curl_global_cleanup();
    return 1;
  } catch (const std::invalid_argument &e) {
    logMessageMain(LogLevel::ERROR, "Initialization Error:", e.what());
    curl_global_cleanup();
    return 1;
  } catch (const std::exception &e) {
    logMessageMain(LogLevel::ERROR, "An unexpected error occurred:", e.what());
    curl_global_cleanup();
    return 1;
  } catch (...) {
    logMessageMain(LogLevel::ERROR, "An unknown fatal error occurred.");
    curl_global_cleanup();
    return 1;
  }

  // 8. Cleanup libcurl globally
  curl_global_cleanup();
  logMessageMain(LogLevel::INFO, "Note Manager Agent Shutting Down.");

  return 0;
}
