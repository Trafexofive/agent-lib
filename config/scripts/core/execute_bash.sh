#!/bin/bash
# execute_bash.sh
# Executes a shell command provided as the first argument (JSON string containing the command).

PARAMS_JSON="$1"

if [ -z "$PARAMS_JSON" ]; then
  echo "Error: No parameters JSON string provided to execute_bash.sh." >&2
  exit 1
fi

# Extract command using jq. Ensure jq is installed.
COMMAND_TO_EXECUTE=$(echo "$PARAMS_JSON" | jq -r .command)

if [ "$COMMAND_TO_EXECUTE" == "null" ] || [ -z "$COMMAND_TO_EXECUTE" ]; then
    echo "Error: 'command' field missing or empty in JSON parameters." >&2
    exit 1
fi

# For security, it's generally safer NOT to use 'eval' if possible.
# If the command is simple and doesn't involve complex shell metacharacters
# that are part of the command itself (rather than arguments),
# you might be able to execute it more directly.
# However, for a general purpose "bash" tool, 'eval' is often used
# with the understanding that the input command is trusted or pre-validated.

# Outputting a marker for easier parsing by the agent if needed.
echo "--- BASH TOOL EXECUTION START ---" >&2
eval "$COMMAND_TO_EXECUTE"
EXIT_STATUS=$?
echo "--- BASH TOOL EXECUTION END (Exit Status: $EXIT_STATUS) ---" >&2

exit $EXIT_STATUS
