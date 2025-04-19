#pragma once

#include <json/json.h> // For Json::Value used in callback signature
#include <stdexcept>   // For std::runtime_error
#include <string>

#include "Agent.hpp"
// Forward declaration (optional, good practice)
class Agent;
namespace Json {
class Value;
}

// Callback function signature: Takes JSON parameters, returns string result
// This is: a typedef for a function pointer that takes a Json::Value and
// returns a string
typedef std::string (*ToolCallback)(const Json::Value &params);
typedef std::string (*ToolCallbackWithAgent)(const Json::Value &params,
                                             Agent &agent);
typedef std::string (*PureTextToolCallback)(const std::string &params);

class Tool {
public:
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback, Agent &agent)
      : m_name(name), m_description(description), m_callback(callback),
        m_agent(&agent) {}
  Tool(const std::string &name, const std::string &description,
       ToolCallbackWithAgent callback, Agent &agent)
      : m_name(name), m_description(description), m_builtin_callback(callback),
        m_agent(&agent) {}
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback)
      : m_name(name), m_description(description), m_callback(callback),
        m_agent(nullptr) {}
  Tool(const std::string &name, const std::string &description,
       PureTextToolCallback callback)
      : m_name(name), m_description(description), m_agent(nullptr),
        m_text_callback(callback) {}
  Tool()
      : m_name(""), m_description(""), m_callback(nullptr), m_agent(nullptr) {}
  Tool(ToolCallback callback)
      : m_name(""), m_description(""), m_callback(callback), m_agent(nullptr) {}
  // Constructor: Takes name, description, and the callback function

  // Get the tool's name
  std::string getName() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  // Get the tool's description (used in prompts)
  std::string getDescription() const { return m_description; }
  void setDescription(const std::string &description) {
    m_description = description;
  }

  // Execute the tool's function with given parameters
  std::string execute(const Json::Value &params) {
    if (m_agent)
      return m_builtin_callback(params, *m_agent);
    else if (m_callback) {
      return m_callback(params);
    } else {
      throw std::runtime_error("No callback function set for this tool");
    }
  }
  std::string execute(const std::string &params) {
    if (m_text_callback) {
      return m_text_callback(params);
    } else {
      throw std::runtime_error("No callback function set for this tool");
    }
  }

  void setCallback(ToolCallback callback) {
    if (!callback) {
      throw std::invalid_argument("Tool callback cannot be NULL");
    }
    m_callback = callback;
  }

  void setBuiltin(ToolCallbackWithAgent callback) {
    m_builtin_callback = callback;
  }

  void addUseCase(const std::string &use_case, const std::string &description) {
    m_use_cases[use_case] = description;
  }
  std::string getUseCase(const std::string &use_case) const {
    auto it = m_use_cases.find(use_case);
    if (it != m_use_cases.end()) {
      return it->second;
    } else {
      throw std::runtime_error("Use case not found");
    }
  }
  std::string getAllUseCases() const {
    std::string all_use_cases;

    all_use_cases += "All Use Cases for tool: " + m_name + "\n";
    for (const auto &pair : m_use_cases) {
      all_use_cases +=
          "Purpose: " + pair.first + "| example case: " + pair.second + "\n";
    }
    return all_use_cases;
  }
  std::string getAllUseCaseCap(size_t cap) const {
    std::string all_use_cases;
    all_use_cases += "All Use Cases for tool: " + m_name + "\n";
    size_t count = 0;
    for (const auto &pair : m_use_cases) {
      if (count >= cap) {
        break;
      }
      all_use_cases +=
          "Purpose: " + pair.first + "| example case: " + pair.second + "\n";
      ++count;
    }
    return all_use_cases;
  }
  // Memory and Storage Management
  void addMemory(const std::string &key, const std::string &value) {
    m_memory_stack[key] = value;
  }
  std::string getMemory(const std::string &key) const {
    auto it = m_memory_stack.find(key);
    if (it != m_memory_stack.end()) {
      return it->second;
    } else {
      throw std::runtime_error("Memory key not found");
    }
  }

private:
  std::string m_name;
  std::string m_description;

  ToolCallback m_callback;
  ToolCallbackWithAgent m_builtin_callback;

  std::map<std::string, std::string> m_use_cases; // for use cases
  // Memory and Storage Management:
  std::map<std::string, std::string> m_memory_stack; // for general use
  // Memory for storing tool state
  Agent *m_agent; // Pointer to the agent using this tool
  PureTextToolCallback m_text_callback;
};
