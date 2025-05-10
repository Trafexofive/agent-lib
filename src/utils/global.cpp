#include "../../inc/Utils.hpp"
#include "../../inc/Agent.hpp" // For logMessage and LogLevel (assuming Agent.hpp defines these)

#include <cstdio>     // For popen, pclose, fgets, remove, mkstemp, perror
#include <cstdlib>    // For system, WEXITSTATUS
#include <cstring>    // For strcpy, strerror
#include <unistd.h>   // For mkstemp, fchmod, write, close, remove
#include <sys/stat.h> // For fchmod, stat, S_IRUSR, S_IWUSR, S_IXUSR
#include <sys/wait.h> // For WEXITSTATUS
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm> // For std::find for example

// Existing executeCommand from your project structure
int executeCommand(const std::string &command, std::string &output) {
  output.clear();
  std::string cmdWithStderr = command + " 2>&1"; // Consistent naming
  FILE *pipe = popen(cmdWithStderr.c_str(), "r");
  if (!pipe) {
    logMessage(LogLevel::ERROR, "popen() failed for command:", command);
    return -1;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  int status = pclose(pipe);
  return WEXITSTATUS(status);
}

// Generic script execution function
std::string executeScriptTool(const std::string& scriptPathOrContent,
                              const std::string& runtime,
                              const Json::Value& params,
                              bool isContentInline) {
    std::string commandToExecute;
    std::string tempScriptFilePath; // Store path for cleanup if a temp file is made

    if (isContentInline) {
        char tempTemplate[] = "/tmp/agent_script_XXXXXX"; // mkstemp modifies this
        int fd = mkstemp(tempTemplate);
        if (fd == -1) {
            logMessage(LogLevel::ERROR, "mkstemp failed for script tool", strerror(errno));
            return "Error: Could not create temporary script file. Errno: " + std::string(strerror(errno));
        }
        tempScriptFilePath = tempTemplate;

        ssize_t written = write(fd, scriptPathOrContent.c_str(), scriptPathOrContent.length());
        if (written != (ssize_t)scriptPathOrContent.length()) {
            logMessage(LogLevel::ERROR, "Failed to write to temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
            close(fd);
            remove(tempScriptFilePath.c_str());
            return "Error: Failed to write to temporary script file.";
        }

        // Set execute permission primarily for bash scripts if run directly
        if (runtime == "bash") {
            if (fchmod(fd, S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
                logMessage(LogLevel::ERROR, "Failed to set permissions on temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
                close(fd);
                remove(tempScriptFilePath.c_str());
                return "Error: Failed to set permissions on temporary script file.";
            }
        }
        close(fd);

        if (runtime == "bash") {
            commandToExecute = tempScriptFilePath; // Can be run directly if executable
        } else {
            commandToExecute = runtime + " " + tempScriptFilePath;
        }
    } else { // scriptPathOrContent is an actual file path
        if (scriptPathOrContent.find("..") != std::string::npos || scriptPathOrContent[0] == '/') {
             logMessage(LogLevel::ERROR, "Potentially unsafe script path provided (must be relative, no '..')", scriptPathOrContent);
             return "Error: Script path must be relative and not contain '..'.";
        }
        std::string safeScriptPath = scriptPathOrContent;
        if (safeScriptPath.rfind("./", 0) != 0) { // Ensure it starts with ./ if relative
            safeScriptPath = "./" + safeScriptPath;
        }

        if (runtime == "bash") {
            // Check if script is already executable, otherwise prepend bash
            struct stat st;
            if (stat(safeScriptPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                commandToExecute = safeScriptPath;
            } else {
                commandToExecute = "bash " + safeScriptPath;
            }
        } else {
            commandToExecute = runtime + " " + safeScriptPath;
        }
    }

    // Parameter passing (simplified for this B-line approach)
    std::string argsString;
    if (params.isObject() && !params.empty()) {
        // For Python, pass the whole JSON object as a single string argument.
        // For Bash, could pass individual values or also JSON string. Let's stick to JSON string for consistency.
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // Compact JSON
        std::string paramsJson = Json::writeString(writerBuilder, params);
        
        // Basic shell escaping for the JSON string argument
        std::string escapedParamsJson = "'"; // Use single quotes for the outer shell argument
        for (char c : paramsJson) {
            if (c == '\'') {
                escapedParamsJson += "'\\''"; // Replaces ' with '\'', ends current ', adds \', starts new '
            } else {
                escapedParamsJson += c;
            }
        }
        escapedParamsJson += "'";
        argsString = " " + escapedParamsJson;
    }
    commandToExecute += argsString;

    std::string output;
    logMessage(LogLevel::DEBUG, "[ScriptTool] Executing final command", commandToExecute);

    int status = ::executeCommand(commandToExecute, output); // Global ::executeCommand

    if (!tempScriptFilePath.empty()) {
        if (remove(tempScriptFilePath.c_str()) != 0) {
            logMessage(LogLevel::WARN, "Failed to remove temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
        }
    }

    if (status != 0) {
        logMessage(LogLevel::ERROR, "[ScriptTool] Execution failed (status " + std::to_string(status) + ") for command", commandToExecute);
        logMessage(LogLevel::ERROR, "[ScriptTool] Script Output (stderr may be included)", output);
        return "Error executing script (status " + std::to_string(status) + "):\n" + output;
    }
    return output;
}
