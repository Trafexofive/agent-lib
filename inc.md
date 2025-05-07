# AI Project Analysis - inc
- Generated on: Wed May  7 07:26:05 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1692 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib/inc
├── Agent.hpp
├── File.hpp
├── Groq.hpp
├── Import.hpp
├── Json.hpp
├── MiniGemini.hpp
├── modelApi.hpp
├── notes.hpp
├── Tool.hpp
├── Utils.hpp
└── variables.hpp
```

## Project Statistics
- Total Files: 113
- Total Lines of Code: 24625
- Languages: .hpp(11)

## Project Files

### File: inc/Agent.hpp
```cpp
#pragma once

#include "File.hpp" // Forward declare if only pointers/refs used, include if values/size needed
#include "MiniGemini.hpp" // Forward declare if only pointers/refs used
#include "Tool.hpp"       // Forward declare if only pointers/refs used

#include <cerrno>
#include <chrono>  // For timestamp generation
#include <iomanip> // For timestamp formatting
#include <map>
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Needed for ActionInfo struct and some method parameters
#include <json/json.h> // Includes json/value.h
                       //

// Forward declarations (if full definitions aren't needed in this header)
namespace Json {
class Value;
}
class MiniGemini;
class Tool;
class File;

// Typedefs
typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;

// Logging Enum
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

// Logging Function Prototype (Implementation likely in agent.cpp or logging
// util)
void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "");

// --- Struct Definitions for LLM Interaction (NEW/UPDATED) ---

struct StructuredThought {
  std::string type;    // e.g., PLAN, OBSERVATION, QUESTION, NORM, etc.
  std::string content; // The text of the thought
};

struct ActionInfo {
  std::string action; // Identifier (tool name, function name, script name)
  std::string type;   // Category (tool, script, internal_function, output,
                      // workflow_control)
  Json::Value
      params; // Parameters for the action (ensure json/json.h is included)
  double confidence = 1.0;           // Optional confidence score
  std::vector<std::string> warnings; // Optional warnings

  // Default constructor might be needed depending on usage
  ActionInfo() = default;
};

// --- Struct Definition for Directive (PUBLIC) ---
struct AgentDirective {
  enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
  Type type = NORMAL;
  std::string description;
  std::string format;
};

namespace types {

// dealing with runtime data context
typedef enum {
  FILE,
  FOLDER,
  EXECUTABLE, // could be  script
  STRING,
} VarType;

} // namespace types

#define FILE_EXT_ALLOWED                                                       \
  {".txt", ".csv", ".json", ".xml", ".yaml", ".yml", ".md", ".log"}

class CONTEXT_ITEM {
private:
  unsigned long id; // will just index them for now, imma look into how to
                    // porperly implement uuid  later.

  std::string name; //  in this case it is refering to the varname inside the
                    //  yaml file
  types::VarType type;
  std::string path;
  std::string content;

  std::string runtime; // for executable type only.

  bool loaded;
};

// --- Agent Class Definition ---
class Agent {
public:
  // Public alias for the directive struct
  using DIRECTIVE = AgentDirective;

  // Struct for OLD tool call format (potentially deprecated)
  struct ToolCallInfo {
    std::string toolName;
    Json::Value params;
  };

  // --- Constructor ---
  Agent(MiniGemini &api);

  // --- Static Factory (Optional) ---
  // static std::unique_ptr<Agent> loadFromYaml(const std::string& yamlPath,
  // MiniGemini& api);

  // --- Configuration (Public Setters - Now including Directive/Task/Command)
  // ---
  void setSystemPrompt(const std::string &prompt);
  void setName(const std::string &name);
  void setDescription(const std::string &description);
  void setIterationCap(int cap);
  void setDirective(const DIRECTIVE &dir);            // MADE PUBLIC
  void addTask(const std::string &task);              // MADE PUBLIC
  void addInitialCommand(const std::string &command); // MADE PUBLIC
  void setSchema(const std::string &schema);
  void setExample(const std::string &example) { this->example = example; }

  // --- Tool Management ---
  void addTool(Tool *tool);
  void removeTool(const std::string &toolName);
  Tool *getTool(const std::string &toolName) const;
  Tool *getRegisteredTool(const std::string &toolName) const;
  std::string getInternalToolDescription(const std::string &toolName) const;

  // finds a tool by name then add it as a built-in
  void addBuiltin(const std::string &toolName);

  // --- Core Agent Loop ---
  void reset();
  std::string prompt(const std::string &userInput);
  void run();

  // --- Agent Interaction (Orchestration) ---
  void addAgent(Agent *agent);
  std::string
  agentInstance(const std::string &name); // Gets sub-agent name (simple check)

  // --- Manual Operations ---
  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params);

  // --- Public Getters (Ensure const correctness and existence) ---
  const std::string getName() const;
  const std::string getDescription() const;
  MiniGemini &getApi(); // Cannot be const if MiniGemini methods aren't const
  fileList getFiles() const;
  const std::vector<std::pair<std::string, std::string>> &getHistory() const;

  // --- Getters needed for saveAgentProfile (Added/Made Public/Const) ---
  const std::string &getSystemPrompt() const;
  int getIterationCap() const;
  const std::vector<std::pair<std::string, std::string>> &getEnv() const;
  const std::vector<std::string> &getExtraPrompts() const;
  const std::vector<std::string> &getTasks() const;
  const DIRECTIVE &getDirective() const; // MADE PUBLIC and CONST

  // --- Memory & State (Public Modifiers) ---
  void addMemory(const std::string &role, const std::string &content);
  void removeMemory(const std::string &role, const std::string &content);
  void addEnvVar(const std::string &key, const std::string &value);
  void importEnvFile(const std::string &filePath);
  void addPrompt(const std::string &prompt); // Adds to extraPrompts

  // --- Utilities ---
  std::string generateStamp(void);
  void addToHistory(const std::string &role, const std::string &content);
  std::string wrapText(const std::string &text);
  std::string wrapXmlLine(const std::string &content, const std::string &tag);
  std::string wrapXmlBlock(const std::string &content, const std::string &tag);
  void addBlockToSystemPrompt(const std::string &content,
                              const std::string &tag);

  void listAllAgentInfo() {
    std::cout << "Agent Name: " << name << "\n"
              << "Description: " << description << "\n"
              << "System Prompt: " << systemPrompt << "\n"
              << "Iteration Cap: " << iterationCap << "\n"
              << "Current Iteration: " << iteration << "\n"
              << "Schema: " << schema << "\n"
              << "Tasks: ";
    for (const auto &task : tasks) {
      std::cout << task << ", ";
    }
    std::cout << "\n";
  }

private:
  // --- Private Members (Clean Naming Scheme) ---
  MiniGemini &api;
  std::map<std::string, Tool *> tools;
  std::map<std::string, std::string> internalToolDescriptions;

  std::string systemPrompt;
  std::vector<std::pair<std::string, std::string>> history;

  int iteration = 0;
  int iterationCap = 10;
  bool skipFlowIteration = false;

  std::vector<std::pair<std::string, std::string>> env;

  fileList files; // Assuming File class definition exists
  std::string name;
  std::string description;

  std::vector<std::string> extraPrompts;

  std::vector<std::pair<std::string, Agent *>> subAgents;

  StrkeyValuePair scratchpad;
  StrkeyValuePair shortTermMemory;
  StrkeyValuePair longTermMemory;

  std::vector<std::string> tasks;
  std::vector<std::string> initialCommands;

  DIRECTIVE directive; // Private member instance192.168.1.101

  std::string schema;
  std::string example;

  // std::vector<std::string, CONTEXT_ITEM> context;
  std::map<std::string, CONTEXT_ITEM> contextMap;
  // --- Private Helper Methods ---
  std::string buildFullPrompt() const;
  // --- UPDATED parseStructuredLLMResponse declaration ---
  bool parseStructuredLLMResponse(
      const std::string &jsonString,
      std::string &status,                      // Changed
      std::vector<StructuredThought> &thoughts, // Changed
      std::vector<ActionInfo> &actions,         // Changed
      std::string &finalResponse);

  // --- ADDED declarations for new private helpers ---
  std::string processActions(const std::vector<ActionInfo> &actions);
  std::string processAction(const ActionInfo &actionInfo);
  std::string directiveTypeToString(
      Agent::DIRECTIVE::Type type) const; // Helper for buildFullPrompt

  // --- Potentially Deprecated Fallback/Old Methods ---
  std::vector<ToolCallInfo>
  extractToolCalls(const std::string &response); // Deprecated?
  std::string
  processToolCalls(const std::vector<ToolCallInfo> &toolCalls); // Deprecated?
  std::string handleToolExecution(const ToolCallInfo &call);    // Deprecated?

  std::string executeApiCall(const std::string &fullPrompt);
  void setSkipFlowIteration(bool skip);

  // --- Internal Tool Implementations (Remain Private) ---
  std::string getHelp(const Json::Value &params);
  std::string skip(const Json::Value &params);
  std::string promptAgentTool(const Json::Value &params);
  std::string summerizeTool(const Json::Value &params); // Typo preserved
  std::string summarizeHistory(const Json::Value &params);
  std::string getWeather(const Json::Value &params);
  std::string reportTool(const Json::Value &params);
  std::string generateCode(const Json::Value &params);

  // --- Task Execution Helpers (Remain Private) ---
  std::string
  executeTask(const std::string &task,
              const Json::Value &format); // Needs Json::Value variant if used
  std::string executeTask(const std::string &task);
  std::string executeTask(const std::string &task, const std::string &format);
  std::string auditResponse(const std::string &response);
};
```

### File: inc/File.hpp
```cpp
#ifndef FILE_HPP // Include guard start
#define FILE_HPP

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Forward declaration for the friend function
class File;
std::ostream &operator<<(std::ostream &os, const File &f);

class File {
public:
  // Default constructor (C++98 style)
  // Initializes members to reasonable defaults.
  File() : path_(""), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector
  }

  // Constructor to load from a file path (C++98 style)
  // Use explicit to prevent unintentional conversions
  explicit File(const std::string &filePath)
      : path_(filePath), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector

    // C++98 std::ifstream constructor often preferred const char*
    std::ifstream fileStream(path_.c_str());
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file for reading: " + path_);
    }

    // Read the entire file content efficiently using stream buffer
    std::ostringstream ss;
    ss << fileStream.rdbuf();
    content_ = ss.str();

    // fileStream is closed automatically when it goes out of scope (RAII)
  }

  // Destructor (C++98 style)
  // Default behavior is sufficient as members manage their own resources.
  ~File() {
    // No explicit cleanup needed for std::string or std::vector
  }

  // --- Getters (const methods) ---
  const std::string &getPath() const { return path_; }
  const std::string &getContent() const { return content_; }
  const std::string &getDescription() const { return description_; }
  const std::vector<std::string> &getTags() const { return tags_; }

  // --- Setters / Modifiers ---
  void setContent(const std::string &content) { content_ = content; }
  void setDescription(const std::string &desc) { description_ = desc; }
  void setTags(const std::vector<std::string> &tags) { tags_ = tags; }

  void addTag(const std::string &tag) {
    // Optional: Avoid adding duplicate tags using std::find (C++98 compatible)
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
      tags_.push_back(tag);
    }
  }

  void removeTag(const std::string &tag) {
    // Erase-remove idiom (C++98 compatible)
    // Need to explicitly state the iterator type (no 'auto')
    std::vector<std::string>::iterator new_end =
        std::remove(tags_.begin(), tags_.end(), tag);
    tags_.erase(new_end, tags_.end());
  }

  // --- File Operations ---

  // Save content back to the original path
  // Throws if path_ is empty.
  void save() {
    if (path_.empty()) {
      throw std::logic_error("Cannot save file: Path is not set. Use saveAs() "
                             "or load from a file first.");
    }
    saveAs(path_); // Delegate to saveAs
  }

  // Save content to a *new* path (and update the internal path_)
  // Note: Marked non-const here because although it doesn't change *members*
  // other than path_,
  //       it has a significant side effect (filesystem modification) and
  //       updates the path. If save() were const, calling saveAs from it would
  //       be problematic. If strict const-correctness regarding members is
  //       needed, saveAs could return void and a separate setPath method could
  //       be used, or save could be non-const. Making saveAs non-const as it
  //       modifies path_ is a reasonable C++98 approach.
  void saveAs(const std::string &newPath) {
    // C++98 std::ofstream constructor often preferred const char*
    std::ofstream outFile(newPath.c_str());
    if (!outFile.is_open()) {
      throw std::runtime_error("Could not open file for writing: " + newPath);
    }
    outFile << content_; // Write content

    // outFile is closed automatically when it goes out of scope (RAII)

    // Update the internal path only after successful write attempt
    path_ = newPath;
  }

  // --- Operator Overloads ---

  // Friend declaration needed within the class
  friend std::ostream &operator<<(std::ostream &os, const File &f);

private:
  // --- Member Variables ---
  std::string path_;              // Path of the file
  std::string content_;           // Content loaded from the file
  std::string description_;       // User-defined description
  std::vector<std::string> tags_; // User-defined tags

  // --- Private Copy Control (Optional, C++98 style) ---
  // Uncomment these lines to prevent copying/assignment if shallow copies
  // are undesirable or if managing resources requires deeper copies.
  // The default compiler-generated ones perform member-wise copy/assignment.
  // File(const File&);            // Disallow copy constructor
  // File& operator=(const File&); // Disallow assignment operator
};

// --- Operator Overloads Implementation (outside class) ---

// Overload << to print metadata summary (C++98 style loop)
inline std::ostream &operator<<(std::ostream &os, const File &f) {
  os << "File(path: \"" << f.getPath() << "\""; // Use getter for consistency
  if (!f.getDescription().empty()) {
    os << ", description: \"" << f.getDescription() << "\"";
  }
  const std::vector<std::string> &tags = f.getTags(); // Use getter
  if (!tags.empty()) {
    os << ", tags: [";
    // C++98 compatible loop (using index)
    for (std::size_t i = 0; i < tags.size(); ++i) {
      os << "\"" << tags[i] << "\"";
      if (i < tags.size() - 1) { // Check if not the last element
        os << ", ";
      }
    }
    os << "]";
  }
  os << ", content_size: " << f.getContent().length()
     << " bytes)"; // Use getter
  return os;
}

#endif // FILE_HPP // Include guard end
```

### File: inc/Groq.hpp
```cpp
#ifndef GROQ_CLIENT_HPP
#define GROQ_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // Or your preferred JSON library

class GroqClient : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    GroqClient(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Groq-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Default: https://api.groq.com/openai/v1

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Groq/OpenAI structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Groq/OpenAI JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GROQ_CLIENT_HPP
```

### File: inc/Import.hpp
```cpp

#pragma once
#include "Agent.hpp"

class Agent;

bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) ;
```

### File: inc/Json.hpp
```cpp
#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

namespace json {

// Forward declarations
class Value;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

// JSON value types
enum ValueType {
    NULL_VALUE,
    BOOL_VALUE,
    NUMBER_VALUE,
    STRING_VALUE,
    ARRAY_VALUE,
    OBJECT_VALUE
};

class Value {
private:
    ValueType type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    Array array_value_;
    Object object_value_;

public:
    // Constructors
    Value() : type_(NULL_VALUE), bool_value_(false), number_value_(0) {}
    
    // Type-specific constructors
    Value(bool value) : type_(BOOL_VALUE), bool_value_(value), number_value_(0) {}
    Value(int value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(double value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(const char* value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const std::string& value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const Array& value) : type_(ARRAY_VALUE), bool_value_(false), number_value_(0), array_value_(value) {}
    Value(const Object& value) : type_(OBJECT_VALUE), bool_value_(false), number_value_(0), object_value_(value) {}

    // Type checking
    bool isNull() const { return type_ == NULL_VALUE; }
    bool isBool() const { return type_ == BOOL_VALUE; }
    bool isNumber() const { return type_ == NUMBER_VALUE; }
    bool isString() const { return type_ == STRING_VALUE; }
    bool isArray() const { return type_ == ARRAY_VALUE; }
    bool isObject() const { return type_ == OBJECT_VALUE; }
    ValueType type() const { return type_; }

    // Value retrieval (with type checking)
    bool asBool() const {
        if (!isBool()) throw std::runtime_error("Value is not a boolean");
        return bool_value_;
    }
    
    double asNumber() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return number_value_;
    }
    
    int asInt() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return static_cast<int>(number_value_);
    }
    
    const std::string& asString() const {
        if (!isString()) throw std::runtime_error("Value is not a string");
        return string_value_;
    }
    
    const Array& asArray() const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    const Object& asObject() const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }
    
    // Mutable accessors
    Array& asArray() {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    Object& asObject() {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }

    // Array/Object element access
    const Value& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        if (index >= array_value_.size()) throw std::out_of_range("Array index out of range");
        return array_value_[index];
    }
    
    Value& operator[](size_t index) {
        if (type_ != ARRAY_VALUE) {
            type_ = ARRAY_VALUE;
            array_value_.clear();
        }
        if (index >= array_value_.size()) 
            array_value_.resize(index + 1);
        return array_value_[index];
    }
    
    const Value& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        Object::const_iterator it = object_value_.find(key);
        if (it == object_value_.end()) throw std::out_of_range("Object key not found");
        return it->second;
    }
    
    Value& operator[](const std::string& key) {
        if (type_ != OBJECT_VALUE) {
            type_ = OBJECT_VALUE;
            object_value_.clear();
        }
        return object_value_[key];
    }

    // Helper methods for object access
    bool has(const std::string& key) const {
        if (!isObject()) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    // Size information
    size_t size() const {
        if (isArray()) return array_value_.size();
        if (isObject()) return object_value_.size();
        return 0;
    }

    // String representation
    std::string toString(bool pretty = false, int indent = 0) const {
        std::ostringstream os;
        
        switch (type_) {
            case NULL_VALUE:
                os << "null";
                break;
                
            case BOOL_VALUE:
                os << (bool_value_ ? "true" : "false");
                break;
                
            case NUMBER_VALUE:
                os << number_value_;
                break;
                
            case STRING_VALUE:
                os << '"';
                for (size_t i = 0; i < string_value_.length(); ++i) {
                    char c = string_value_[i];
                    switch (c) {
                        case '"':  os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\b': os << "\\b"; break;
                        case '\f': os << "\\f"; break;
                        case '\n': os << "\\n"; break;
                        case '\r': os << "\\r"; break;
                        case '\t': os << "\\t"; break;
                        default:
                            if (c < 32) {
                                os << "\\u" 
                                   << std::hex << std::uppercase
                                   << static_cast<int>(c)
                                   << std::dec << std::nouppercase;
                            } else {
                                os << c;
                            }
                    }
                }
                os << '"';
                break;
                
            case ARRAY_VALUE: {
                os << '[';
                
                if (pretty && !array_value_.empty()) {
                    os << '\n';
                }
                
                for (size_t i = 0; i < array_value_.size(); ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << array_value_[i].toString(pretty, indent + 2);
                    
                    if (i < array_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !array_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << ']';
                break;
            }
            
            case OBJECT_VALUE: {
                os << '{';
                
                if (pretty && !object_value_.empty()) {
                    os << '\n';
                }
                
                Object::const_iterator it = object_value_.begin();
                for (size_t i = 0; it != object_value_.end(); ++it, ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << '"' << it->first << "\":";
                    
                    if (pretty) {
                        os << ' ';
                    }
                    
                    os << it->second.toString(pretty, indent + 2);
                    
                    if (i < object_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !object_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << '}';
                break;
            }
        }
        
        return os.str();
    }
};

// Parsing functions
namespace parser {
    // Simple tokenizer for JSON parsing
    class Tokenizer {
    private:
        const std::string& input_;
        size_t pos_;

        void skipWhitespace() {
            while (pos_ < input_.length() && isspace(input_[pos_]))
                ++pos_;
        }

    public:
        Tokenizer(const std::string& input) : input_(input), pos_(0) {}

        bool hasMore() const {
            return pos_ < input_.length();
        }

        char peek() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_];
        }

        char next() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_++];
        }

        std::string readString() {
            // Assume the opening quote has been consumed
            std::string result;
            bool escape = false;

            while (hasMore()) {
                char c = input_[pos_++];
                
                if (escape) {
                    escape = false;
                    switch (c) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': {
                            // Handle \uXXXX escape sequences
                            if (pos_ + 4 > input_.length()) {
                                throw std::runtime_error("Invalid \\u escape sequence");
                            }
                            std::string hex = input_.substr(pos_, 4);
                            pos_ += 4;
                            
                            // Convert hex to int and then to char
                            int value = 0;
                            for (int i = 0; i < 4; ++i) {
                                value = value * 16;
                                char h = hex[i];
                                if (h >= '0' && h <= '9') value += (h - '0');
                                else if (h >= 'A' && h <= 'F') value += (h - 'A' + 10);
                                else if (h >= 'a' && h <= 'f') value += (h - 'a' + 10);
                                else throw std::runtime_error("Invalid hex character in \\u escape");
                            }
                            
                            // This is a simplification - proper UTF-8 handling would be more complex
                            if (value < 128) {
                                result += static_cast<char>(value);
                            } else {
                                // Just for basic handling, more comprehensive UTF-8 would be needed
                                result += '?';
                            }
                            break;
                        }
                        default:
                            result += c;
                    }
                } else if (c == '\\') {
                    escape = true;
                } else if (c == '"') {
                    // End of string
                    break;
                } else {
                    result += c;
                }
            }
            
            return result;
        }

        std::string readNumber() {
            size_t start = pos_ - 1;  // Include the first digit
            
            // Read digits, dot, exponent, etc.
            while (hasMore() && (
                   isdigit(input_[pos_]) || 
                   input_[pos_] == '.' || 
                   input_[pos_] == 'e' || 
                   input_[pos_] == 'E' || 
                   input_[pos_] == '+' || 
                   input_[pos_] == '-')) {
                ++pos_;
            }
            
            return input_.substr(start, pos_ - start);
        }

        std::string readToken() {
            size_t start = pos_ - 1;
            while (hasMore() && isalpha(input_[pos_])) {
                ++pos_;
            }
            return input_.substr(start, pos_ - start);
        }
        
        void expect(char c) {
            if (next() != c) {
                std::ostringstream msg;
                msg << "Expected '" << c << "' at position " << pos_;
                throw std::runtime_error(msg.str());
            }
        }
    };

    Value parseValue(Tokenizer& tokenizer);

    Value parseObject(Tokenizer& tokenizer) {
        Object obj;
        
        // Empty object?
        if (tokenizer.peek() == '}') {
            tokenizer.next();  // Consume '}'
            return Value(obj);
        }
        
        while (true) {
            // Parse key
            if (tokenizer.next() != '"') {
                throw std::runtime_error("Expected string as object key");
            }
            std::string key = tokenizer.readString();
            
            // Parse colon
            if (tokenizer.next() != ':') {
                throw std::runtime_error("Expected ':' after object key");
            }
            
            // Parse value
            obj[key] = parseValue(tokenizer);
            
            // More items?
            char c = tokenizer.next();
            if (c == '}') {
                break;  // End of object
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or '}' after object value");
            }
        }
        
        return Value(obj);
    }

    Value parseArray(Tokenizer& tokenizer) {
        Array arr;
        
        // Empty array?
        if (tokenizer.peek() == ']') {
            tokenizer.next();  // Consume ']'
            return Value(arr);
        }
        
        while (true) {
            // Parse value
            arr.push_back(parseValue(tokenizer));
            
            // More items?
            char c = tokenizer.next();
            if (c == ']') {
                break;  // End of array
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or ']' after array value");
            }
        }
        
        return Value(arr);
    }

    Value parseValue(Tokenizer& tokenizer) {
        char c = tokenizer.peek();
        
        switch (c) {
            case 'n':
                // null
                tokenizer.next();  // Consume 'n'
                if (tokenizer.readToken() == "ull") {
                    return Value();
                }
                throw std::runtime_error("Invalid token: expected 'null'");
                
            case 't':
                // true
                tokenizer.next();  // Consume 't'
                if (tokenizer.readToken() == "rue") {
                    return Value(true);
                }
                throw std::runtime_error("Invalid token: expected 'true'");
                
            case 'f':
                // false
                tokenizer.next();  // Consume 'f'
                if (tokenizer.readToken() == "alse") {  
                    return Value(false);
                }
                throw std::runtime_error("Invalid token: expected 'false'");
                
            case '"':
                // string
                tokenizer.next();  // Consume '"'
                return Value(tokenizer.readString());
                
            case '{':
                // object
                tokenizer.next();  // Consume '{'
                return parseObject(tokenizer);
                
            case '[':
                // array
                tokenizer.next();  // Consume '['
                return parseArray(tokenizer);
                
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                // number
                tokenizer.next();  // Consume first digit/sign
                return Value(atof(tokenizer.readNumber().c_str()));
                
            default:
                throw std::runtime_error("Unexpected character in JSON");
        }
    }
    
    Value parse(const std::string& input) {
        Tokenizer tokenizer(input);
        Value result = parseValue(tokenizer);
        
        // Check for trailing data
        if (tokenizer.hasMore()) {
            char c = tokenizer.peek();
            if (!isspace(c)) {
                throw std::runtime_error("Unexpected trailing data in JSON");
            }
        }
        
        return result;
    }
}  // namespace parser

// Helper functions
inline Value parse(const std::string& input) {
    return parser::parse(input);
}

inline Value parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parse(buffer.str());
}

}  // namespace json

#endif  // JSON_H_
```

### File: inc/MiniGemini.hpp
```cpp
#ifndef GEMINI_CLIENT_HPP
#define GEMINI_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // For Json::Value (or your preferred JSON library)

const std::string MODELS[] = {
    // Main models
    "gemini-2.0-flash",         // Latest Flash model
    "gemini-2.0-flash-lite",    // Cost-efficient Flash variant
    "gemini-1.5-flash",         // Previous generation Flash model
    "gemini-1.5-flash-8b",      // Lightweight 8B parameter variant
    "gemini-2.5-pro-exp-03-25", // Experimental Pro model

    // Special purpose models
    "gemini-embedding-exp",      // Text embedding model
    "models/text-embedding-004", // Basic text embedding model
    "imagen-3.0-generate-002",   // Image generation model
    "models/embedding-001"       // Legacy embedding model
};

class MiniGemini : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    MiniGemini(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Gemini-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Allow changing the base URL if needed

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Gemini structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Gemini's JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback (can be shared or made global if needed)
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GEMINI_CLIENT_HPP
```

### File: inc/modelApi.hpp
```cpp
#pragma once
#include <stdexcept>
#include <string>

// Custom exception for API errors (can be shared by all clients)
class ApiError : public std::runtime_error {
public:
    ApiError(const std::string& message) : std::runtime_error(message) {}
};

// Abstract base class for LLM clients
class LLMClient {
public:
    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~LLMClient() = default;

    // Pure virtual function that all derived clients MUST implement
    // Takes a prompt and returns the generated text or throws ApiError
    virtual std::string generate(const std::string& prompt) = 0;

    // Optional: Add common configuration setters if desired,
    // but they might be better handled in derived classes if APIs differ significantly.
    virtual void setModel(const std::string& model) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setMaxTokens(int maxTokens) = 0;
};

```

### File: inc/notes.hpp
```cpp
#ifndef NOTES_HPP
#define NOTES_HPP

#include <fstream>
#include <string>
#include <vector>

class note {
public:
  note(const std::string &text) : _text(text) {}
  note(const std::string &text, const std::string &path)
      : _text(text), _path(path) {
    // if (_path.empty()) {
    //     _path = "./";
    //     }
    // open file and write text to it
    std::ofstream file(_path);

    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file << text;
    }
  }
  ~note() {
    // close file
    std::ofstream file(_path, std::ios::app);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file.close();
    }
  }
  std::string getText() const;
  void setText(const std::string &text);

  std::string getPath() const;

private:
  std::string _text;
  std::string _path;
};

typedef std::vector<note> notes;

#endif
```

### File: inc/Tool.hpp
```cpp
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
```

### File: inc/Utils.hpp
```cpp

#pragma once

#include <string>

int executeCommand(const std::string &command, std::string &output) ;
```

### File: inc/variables.hpp
```cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <sstream>
// std::any is C++17 and later
#include <any>

template <typename T>
class Variable {
private:
    std::string key;
    T value;

public:
    Variable(const std::string& k, const T& val) : key(k), value(val) {}

    std::string getKey() const { return key; }
    T getValue() const { return value; }
};


class NamespaceVariables {
private:
    std::map<std::string, NamespaceVariables*> namespaces;
    std::map<std::string, Variable<std::any>> variables;  // Use std::any to store any type

public:
    // Access a namespace (creates it if it doesn't exist)
    NamespaceVariables& operator[](const std::string& ns) {
        if (namespaces.find(ns) == namespaces.end()) {
            namespaces[ns] = new NamespaceVariables();
        }
        return *namespaces[ns];
    }

    // Set a variable within the current namespace
    template <typename T>
    void set(const std::string& key, const T& value) {
        variables[key] = Variable<std::any>(key, value);
    }

    // Get a variable within the current namespace
    template <typename T>
    T get(const std::string& key) const {
        try {
            return std::any_cast<T>(variables.at(key).getValue());
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Error: Invalid type cast for key: " << key << std::endl;
            throw; // Re-throw the exception or handle it differently
        }
    }


    // Helper function to print all variables in a namespace (and sub-namespaces)
    void print(const std::string& prefix = "") const {
        for (const auto& var : variables) {
            std::cout << prefix << var.first << ": ";
            try { // Attempt to print common types
                std::cout << std::any_cast<std::string>(var.second.getValue()) << std::endl;
            } catch(...) {
                try { std::cout << std::any_cast<int>(var.second.getValue()) << std::endl; }
                catch(...) {
                    try { std::cout << std::any_cast<bool>(var.second.getValue()) << std::endl; }
                    catch(...) {
                        try { std::cout << std::any_cast<std::filesystem::path>(var.second.getValue()) << std::endl; }
                        catch(...) { std::cout << "[Unprintable Type]" << std::endl; }
                    }
                }
            }
        }
        for (const auto& ns : namespaces) {
            std::cout << prefix << ns.first << ":" << std::endl;
            ns.second->print(prefix + "  ");
        }
    }


};




int main() {
    NamespaceVariables vars;

    vars["global"]["configs"]["agents"]["standard"].set("name", "Standard Agent");
    vars["agent-1"]["state"]["input"].set("value", 123);
    vars["global"]["configs"]["port"].set("number", 8080);
    vars["global"]["configs"]["path"].set<std::filesystem::path>("value", "data/config.json");


    vars.print();


    std::cout << "Agent name: " << vars["global"]["configs"]["agents"]["standard"].get<std::string>("name") << std::endl;
    std::cout << "Port number: " << vars["global"]["configs"]["port"].get<int>("number") << std::endl;

    // Example demonstrating error handling:
    try {
        std::string portNumber = vars["global"]["configs"]["port"].get<std::string>("number"); // Incorrect type
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }


    return 0;
}
```
