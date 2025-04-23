This project implements an agent that interacts with a language model and uses tools to accomplish tasks. The agent can execute bash commands, read and write files, and access the current time.

## Documentation

### Agent Class

The `Agent` class is the core component of this project. It is responsible for interacting with the language model, managing tools, and processing user input.

#### Core Components

- `MiniGemini &m_api`: A reference to the API client used to interact with the language model.
- `std::map<std::string, Tool *> m_tools`: A map of available external tools. The keys are the tool names, and the values are pointers to the `Tool` objects.
- `std::map<std::string, std::string> m_internalToolDescriptions`: A map of descriptions for internal tools, such as `help`.

#### State

- `std::string m_systemPrompt`: The system prompt used to initialize the language model.
- `std::vector<std::pair<std::string, std::string>> m_history`: A conversation history, stored as pairs of roles and content.
- `int iteration`: The current iteration number.
- `int iterationCap`: The maximum number of iterations.
- `bool skipFlowIteration`: A flag to skip the final LLM call after tool execution.
- `std::vector<std::pair<std::string, std::string>> _env`: Stores the results of tool calls for later use.
- `fileList _files`: Stores files for later use.
- `std::string _name`: Stores the agent's name.
- `StrkeyValuePair Scratchpad`: A scratchpad for temporary storage.
- `StrkeyValuePair ShortTermMemory`: Short-term memory for the agent.
- `StrkeyValuePair LongTermMemory`: Long-term memory for the agent.

#### Methods

- `Agent(MiniGemini &api)`: Constructor. Takes a `MiniGemini` object by reference.
- `void setSystemPrompt(const std::string &prompt)`: Sets the system prompt.
- `void addTool(Tool *tool)`: Adds a tool to the agent.
- `void addTextTool(Tool *tool)`: Adds a text-based tool to the agent.
- `void reset()`: Resets the agent's state.
- `std::string prompt(const std::string &userInput)`: Processes user input and interacts with the language model.
- `void run()`: Starts the interactive loop.
- `void addMemory(const std::string &role, const std::string &content)`: Adds a memory to the agent's long-term memory.
- `void removeMemory(const std::string &role, const std::string &content)`: Removes a memory from the agent's long-term memory.
- `std::string getMemory(const std::string &key) const`: Retrieves a memory from the agent's long-term memory.
- `void clearMemory()`: Clears the agent's long-term memory.
- `MiniGemini &getApi()`: Returns a reference to the `MiniGemini` object.
- `fileList getFiles()`: Returns the list of files.

### Tool Class

The `Tool` class represents a tool that the agent can use to accomplish tasks.

#### Members

- `std::string m_name`: The name of the tool.
- `std::string m_description`: A description of the tool.
- `ToolCallback m_callback`: A function pointer to the tool's callback function.
- `Agent *m_agent`: A pointer to the agent using this tool.
- `PureTextToolCallback m_text_callback`: A function pointer for text-based tools.
- `std::map<std::string, std::string> m_use_cases`: A map of use cases for the tool.
- `std::map<std::string, std::string> m_memory_stack`: Memory for storing tool state.

#### Methods

- `Tool(const std::string &name, const std::string &description, ToolCallback callback, Agent &agent)`: Constructor.
- `std::string getName() const`: Returns the tool's name.
- `std::string getDescription() const`: Returns the tool's description.
- `std::string execute(const Json::Value &params)`: Executes the tool's callback function.
- `std::string execute(const std::string &params)`: Executes the text-based tool's callback function.
- `void setCallback(ToolCallback callback)`: Sets the tool's callback function.
- `void setBuiltin(ToolCallbackWithAgent callback)`: Sets the tool's built-in callback function.
- `void addUseCase(const std::string &use_case, const std::string &description)`: Adds a use case for the tool.
- `std::string getUseCase(const std::string &use_case) const`: Returns a use case for the tool.
- `void addMemory(const std::string &key, const std::string &value)`: Adds a memory to the tool's memory stack.
- `std::string getMemory(const std::string &key) const`: Retrieves a memory from the tool's memory stack.

### MiniGemini Class

The `MiniGemini` class is responsible for interacting with the Gemini language model API.

#### Members

- `std::string m_apiKey`: The API key used to authenticate with the Gemini API.
- `std::string m_model`: The name of the Gemini language model to use.
- `double m_temperature`: The temperature parameter used to control the randomness of the language model's output.
- `int m_maxTokens`: The maximum number of tokens in the language model's output.
- `const std::string m_baseUrl`: The base URL for the Gemini API.

#### Methods

- `MiniGemini(const std::string &apiKey = "")`: Constructor.
- `std::string generate(const std::string &prompt)`: Generates content based on a prompt.
- `void setApiKey(const std::string &apiKey)`: Sets the API key.
- `void setModel(const std::string &model)`: Sets the model name.
- `void setTemperature(double temperature)`: Sets the temperature.
- `void setMaxTokens(int maxTokens)`: Sets the maximum number of tokens.

### Available tools:

- **help**: Provides descriptions of available tools. Takes an optional 'tool_name' parameter to get help for a specific tool.
- **bash**: Execute a bash command. Requires a JSON object with a 'command' parameter (string). Example: {"command": "ls -l"}.
- **getCurrentTime**: Returns the current date and time. params ignored.
- **write**: Tool Function: Writes content to a file specified by the path on the first line of input.
Input: A string where the first line is the file path, and subsequent lines are the content.
Output: A success message or an error message.