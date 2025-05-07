#pragma once

#include <json/json.h> // For Json::Value used in callback signature
#include <map>         // For m_use_cases, m_memory_stack
#include <stdexcept>   // For std::runtime_error
#include <string>

// Forward declaration (optional, good practice)
class Agent;
namespace Json {
class Value;
}

// Callback function signatures: Takes JSON parameters, returns string result
typedef std::string (*ToolCallback)(const Json::Value &params);
typedef std::string (*ToolCallbackWithAgent)(const Json::Value &params,
                                             Agent &agent);

class Tool {
public:
  // --- Constructors ---
  // Constructor for tools needing agent context (built-in style)
  Tool(const std::string &name, const std::string &description,
       ToolCallbackWithAgent callback, Agent &agent)
      : m_name(name), m_description(description), m_callback(nullptr),
        m_builtin_callback(callback), m_agent(&agent) {}

  // Constructor for standard external tools
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback)
      : m_name(name), m_description(description), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Default constructor
  Tool()
      : m_name(""), m_description(""), m_callback(nullptr),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Constructor with just a callback (name/desc can be set later)
  Tool(ToolCallback callback)
      : m_name(""), m_description(""), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // --- Getters / Setters ---
  std::string getName() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  std::string getDescription() const { return m_description; }
  void setDescription(const std::string &description) {
    m_description = description;
  }

  // --- Execution ---
  // Execute the tool's function with given JSON parameters
  std::string execute(const Json::Value &params) {
    if (m_builtin_callback &&
        m_agent) { // Prioritize built-in if agent context is available
      return m_builtin_callback(params, *m_agent);
    } else if (m_callback) { // Fallback to standard callback
      return m_callback(params);
    } else {
      throw std::runtime_error("No valid callback function set for tool '" +
                               m_name + "'");
    }
  }
  // execute(const std::string& params) overload removed

  // --- Callback Configuration ---
  void setCallback(ToolCallback callback) {
    if (!callback) {
      throw std::invalid_argument("Tool callback cannot be NULL");
    }
    m_callback = callback;
    m_builtin_callback = nullptr; // Ensure only one callback type is active
  }

  void setBuiltinCallback(ToolCallbackWithAgent callback, Agent &agent) {
    if (!callback) {
      throw std::invalid_argument("Builtin tool callback cannot be NULL");
    }
    m_builtin_callback = callback;
    m_agent = &agent;
    m_callback = nullptr; // Ensure only one callback type is active
  }

  // --- Use Cases ---
  void addUseCase(const std::string &use_case, const std::string &example) {
    m_use_cases[use_case] = example;
  }

  std::string getUseCase(const std::string &use_case) const {
    auto it = m_use_cases.find(use_case);
    if (it != m_use_cases.end()) {
      return it->second;
    } else {
      // Return empty string or throw? Returning empty might be safer.
      return "";
      // throw std::runtime_error("Use case not found: " + use_case);
    }
  }
  std::string getAllUseCases() const {
    std::string all_use_cases;
    if (m_use_cases.empty())
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "':\n";
    for (const auto &pair : m_use_cases) {
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
    }
    return all_use_cases;
  }
  std::string getAllUseCaseCap(size_t cap) const {
    std::string all_use_cases;
    if (m_use_cases.empty() || cap == 0)
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "' (limit " +
                     std::to_string(cap) + "):\n";
    size_t count = 0;
    for (const auto &pair : m_use_cases) {
      if (count >= cap)
        break;
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
      ++count;
    }
    return all_use_cases;
  }

private:
  std::string m_name;
  std::string m_description;

  ToolCallback m_callback = nullptr; // Standard callback (expects JSON)
  ToolCallbackWithAgent m_builtin_callback =
      nullptr; // Callback needing agent context (expects JSON)
  enum {
    // Tool types can be defined here if needed
    // e.g., EXTERNAL, BUILTIN, etc.
    TOOL_TYPE_EXTERNAL,
    TOOL_TYPE_BUILTIN
  } type = TOOL_TYPE_EXTERNAL;

  std::map<std::string, std::string>
      m_use_cases; // Map of purpose -> example JSON params string

  Agent *m_agent = nullptr; // Pointer to the agent using this tool (for builtin
                            // callbacks) PureTextToolCallback removed
};
