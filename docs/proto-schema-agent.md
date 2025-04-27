{
  "_comment": "REVISED JSON structure expected as output FROM the LLM, TO BE PARSED by the Agent/Orchestrator. Reflects changes like thoughts array, action/type fields. Comments clarify requirements.",
  "llm_output_schema_version": "llm-json-output-draft-0.2", // Internal versioning for this JSON structure idea

  "thoughts": [ // CHANGED: Now an array of strings. Can be empty `[]`.
    "string - Step 1 of reasoning: Identified the user's core need.",
    "string - Step 2: Decided an external search is required.",
    "string - Step 3: Formulating parameters for the 'web_search' action."
    // The LLM can break down its thought process into multiple steps or aspects.
  ],

  "actions": [ // RENAMED: Was 'tool_calls'. REQUIRED array. MUST be `[]` if no action is taken.
    // --- Example of a single action object ---
    {
      "_comment": "Object representing one action requested by the LLM. Required fields: 'action', 'type', 'params'.",

      "action": "string - RENAMED: Was 'tool_name'. REQUIRED. The identifier for the action to take (e.g., 'web_search', 'bash', 'URLValidator', 'summarize_chunk'). This identifier must map to a known Tool, Script, API call, internal function, etc.",

      "type": "string - ADDED: REQUIRED. Specifies the category of the action. Helps the orchestrator select the correct handler. Examples: 'tool', 'script', 'http_request', 'internal_function', 'workflow_control'.",
      // Examples:
      // 'tool': Corresponds to a registered Tool object (like the C++ Tool class).
      // 'script': Refers to an imported/defined script (e.g., bash, python).
      // 'http_request': Specifies parameters for making a direct HTTP call.
      // 'internal_function': Calls a hardcoded function within the orchestrator/agent library (e.g., 'summarizeHistory').
      // 'workflow_control': Actions that affect the workflow itself (e.g., 'fail_step', 'skip_to').

      "params": { // REQUIRED: A JSON object containing the parameters needed by the specified 'action' and 'type'. Structure depends entirely on the action's requirements.
        "query": "AI in healthcare latest research", // Example for a 'web_search' action
        "region": "us"
        // ... structure defined by the action referenced in 'action'/'type'
      }
    }
    // --- End of example action object ---
    // , { "action": "bash", "type": "script", "params": { "command": "ls -l" } } // Multiple actions possible
  ],

  "final_response": "string - REQUIRED: The textual response intended for the end-user. Logic remains the same: \n - If 'actions' is EMPTY (`[]`), this MUST contain the final answer.\n - If 'actions' is NOT EMPTY, this SHOULD be `\"\"` or an interim message (ignored by default loop)."
}

============================================

"thoughts": [
  {
    "type": "OBSERVATION", // Types: PLAN, OBSERVATION, QUESTION, HYPOTHESIS, CRITIQUE, ASSUMPTION, GOAL, NORM (way of doing x, avoiding y because)
    "content": "User query asks for healthcare AI articles within the last month."
  },
  {
    "type": "PLAN",
    "content": "1. Need to perform a web search. 2. Will need current date for 'last month'. 3. Filter results. 4. Summarize."
  },
  {
    "type": "ASSUMPTION",
    "content": "Assuming 'last month' means the previous calendar month relative to today."
  },
  {
    "type": "QUESTION", // Internal question/uncertainty
    "content": "Is the user interested in academic papers or news articles primarily?"
  }
],

good to have the tought framework, easily extendable as well.

"thoughts": [ { "type": "OBSERVATION", "content": "User asked for weather but didn't specify location." } ],
"actions": [
  {
    "action": "web_search",
    "type": "tool",
    "params": { "query": "AI healthcare news", "timeframe": "past_30_days" },
    "confidence": 0.85, // Optional: Estimated confidence (0.0 to 1.0) that this action/params are correct.
    "warnings": [ // Optional: Specific warnings about this action/params.
      "Parameter 'timeframe=past_30_days' is an interpretation of 'last month'; could be ambiguous.",
      "Web search might return irrelevant results requiring filtering."
    ]
  }
  {
    "action": "request_user_input", // Or potentially a specific type like "clarification_request"
    "type": "workflow_control", // Indicate it affects the flow, not an external tool
    "params": {
      "query_to_user": "I can get the weather for you! Which city are you interested in?",
      "required_info": ["location (city name)"], // What information is needed
      "context": "User asked for weather forecast." // Optional context for the request
    }
  }
  {
    "action": "response", // could be the final response or not, the're just gonna stack.
    "type": "default", 
    "params": {
      "reply": "here is what you request $(context.report)",
      "file": "./path-to.." // displays the file content
    }
  }
],

{
  "status": "REQUIRES_ACTION", // Possible values: SUCCESS_FINAL, REQUIRES_ACTION, REQUIRES_CLARIFICATION, ERROR_INTERNAL
  "thoughts": [ ... ],
  "actions": [ ... ], // Non-empty if status is REQUIRES_ACTION
  "final_response": "..." // Contains final answer if status is SUCCESS_FINAL
}

==================================================================

{
  "_comment": "REFINED JSON structure expected from LLM (v0.3). Includes structured thoughts, action confidence/warnings, clarification requests, inline responses, and root status.",
  "llm_output_schema_version": "llm-json-output-draft-0.3",

  "status": "string - REQUIRED: High-level outcome hint. Values: 'SUCCESS_FINAL', 'REQUIRES_ACTION', 'REQUIRES_CLARIFICATION', 'ERROR_INTERNAL'. Orchestrator can use this for quick dispatch.",

  "thoughts": [ // REQUIRED: Array of structured thought objects. Can be empty `[]`.
    // --- Example Thought Object ---
    {
      "type": "string - REQUIRED: Category of the thought. Suggested values: PLAN, OBSERVATION, QUESTION, HYPOTHESIS, CRITIQUE, ASSUMPTION, GOAL, NORM (explaining a standard procedure or reason).",
      "content": "string - REQUIRED: The textual content of the thought."
    }
    // --- Examples ---
    // { "type": "OBSERVATION", "content": "User requested a summary of $(state.document_id)." },
    // { "type": "PLAN", "content": "1. Fetch document content. 2. Call summarization tool. 3. Format as response." },
    // { "type": "NORM", "content": "Standard procedure is to summarize documents under 5000 words directly; longer ones require chunking." },
    // { "type": "QUESTION", "content": "Is a specific summary length required?" }
  ],

  "actions": [ // REQUIRED: Array of action objects. MUST be `[]` if status is 'SUCCESS_FINAL' or 'ERROR_INTERNAL' and no cleanup actions are needed.
    // --- Example Action Object (Tool/Script/API Call) ---
    {
      "_comment": "Represents executing a tool, script, API call, etc.",
      "action": "string - REQUIRED: Identifier for the action (tool name, script name, function name).",
      "type": "string - REQUIRED: Category ('tool', 'script', 'http_request', 'internal_function', etc.).",
      "params": { // REQUIRED: Parameters for the action, specific to the action/type.
        // "paramName": value, ...
      },
      "confidence": "float - OPTIONAL (0.0-1.0): LLM's estimated confidence in this action/params.",
      "warnings": [ // OPTIONAL: Array of strings warning about potential issues.
        // "string - Example warning about parameter ambiguity."
      ]
    },
    // --- Example Action Object (Clarification Request) ---
    {
      "_comment": "Represents asking the user for more information.",
      "action": "request_user_input", // Standardized action name for clarification.
      "type": "workflow_control", // Type indicates it affects the workflow/interaction loop.
      "params": {
        "query_to_user": "string - REQUIRED: The question to ask the user.",
        "required_info": ["string - OPTIONAL: Hint about what info is needed (e.g., 'city name')."],
        "context": "string - OPTIONAL: Brief context for the question."
      },
      "confidence": 1.0, // Typically high confidence if clarification is needed.
      "warnings": [] // Usually no warnings needed here.
    },
    // --- Example Action Object (Direct Response/Output) ---
    {
      "_comment": "Represents sending a direct response fragment or content to the output sink(s). Can be intermediate or part of the final output.",
      "action": "send_response", // Standardized action name for direct output.
      "type": "output", // Type indicates it's content to be delivered.
      "params": {
        // Choose ONE of the following content sources:
        "text": "string - REQUIRED (if no file/variable): Literal text content to send. Can use $(...) variables.",
        // "file_path": "string - REQUIRED (if no text/variable): Path to a local file whose content should be sent.", // Requires orchestrator file access
        // "variable": "string - REQUIRED (if no text/file): State variable whose content should be sent (e.g., '$(state.summary)')." // FUTURE: Requires careful variable resolution
      },
      "confidence": 1.0,
      "warnings": []
    }
    // ... more actions can be included in the array
  ],

  "final_response": "string - DEPRECATED / REDUNDANT? With status='SUCCESS_FINAL' and potentially `action: send_response`, this might be less necessary. TBD: Keep for simple cases or remove? For now, retain for backward compatibility or very simple, single-response outputs where status='SUCCESS_FINAL' and actions=[]."
  // If kept:
  // - If status='SUCCESS_FINAL' and actions=[], this MUST contain the final answer.
  // - If status='REQUIRES_ACTION' or 'REQUIRES_CLARIFICATION', this SHOULD be `""`.
  // - If status='ERROR_INTERNAL', this might contain an error message.
}
