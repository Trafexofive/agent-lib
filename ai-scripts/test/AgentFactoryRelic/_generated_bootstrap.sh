#!/bin/bash
# Bootstrap script for AgentFactoryRelic (v0.1.2)

# Create directories for agent configurations, logs, and data persistence.
echo "Creating initial directory structure..."
mkdir -p agent_configs
mkdir -p logs
mkdir -p data

# Populate with example agent configurations from our brainstorming session
echo "Creating example 'code-generator.yaml'..."
cat <<'EOF' > agent_configs/code-generator.yaml
agent_name: "code-generator"
version: "0.1.0"
description: "Generates Python code based on a user's request and saves it to a file."

model:
  source: "mistral:7b-instruct-q4_K_M"
  parameters:
    temperature: 0.0
    stop_sequences: ["\nObservation:"]

persona:
  prompt: |
    You are an expert Python programmer. Your only goal is to write clean, correct, and well-commented Python code that fulfills the user's request.
    You must save the generated code to a file using the 'write_file' tool.
    Do not add any conversational text.

    USER REQUEST:
    "{{.user_request}}"

  inputs:
    - name: "user_request"
      description: "A description of the Python script to be created."
      required: true

tools:
  - tool_name: "write_file"
    description: "Writes or overwrites a file with new content. Use this to save results or code."
    parameters:
      - name: "filepath"
        type: "string"
        description: "The destination file path for the code, e.g., 'scripts/my_script.py'."
        required: true
      - name: "content"
        type: "string"
        description: "The complete and valid Python code to be saved."
        required: true

output_schema:
  format: "json"
  description: "A JSON object containing your thought process and the call to the 'write_file' tool."
  example: >
    {
      "thought": "The user wants a script to parse a CSV file. I will write the Python code using the 'csv' module and then call the tool to save it as 'parsers/csv_parser.py'.",
      "tool_call": {
        "name": "write_file",
        "arguments": {
          "filepath": "data/csv_parser.py",
          "content": "import csv\n\ndef parse_csv(file_path):\n    # ... function content ...\n\nif __name__ == '__main__':\n    parse_csv('data.csv')"
        }
      }
    }
EOF

echo -e "\nBootstrap complete. You can now review '.env' and run 'make up'."
echo "IMPORTANT: After starting the stack, you must pull a model into Ollama. Example: make exec service=ollama args=\"ollama pull mistral:7b-instruct-q4_K_M\""
