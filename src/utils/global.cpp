
#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cctype>    // For std::toupper
#include <chrono>    // For timestamp
#include <cstdio>    // For FILE, fgets
#include <cstdlib>   // For popen, pclose, system
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

// Collection of utility functions and global helpers

// Helper to execute external commands
int executeCommand(const std::string &command, std::string &output) {
  output.clear();
  std::string cmd_with_stderr = command + " 2>&1";
  FILE *pipe = popen(cmd_with_stderr.c_str(), "r");
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

