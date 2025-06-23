#!/bin/bash
PARAMS_JSON="$1"
FILE_PATH=$(echo "$PARAMS_JSON" | jq -r .file_path)
if [ -z "$FILE_PATH" ] || [ "$FILE_PATH" == "null" ]; then echo '{"error":"file_path parameter missing"}'; exit 1; fi
if [ ! -f "$FILE_PATH" ]; then echo "{\"error\":\"Python file not found: $FILE_PATH\"}"; exit 1; fi

LINT_MESSAGE=$(python3 -m py_compile "$FILE_PATH" 2>&1)
EXIT_STATUS=$?

if [ $EXIT_STATUS -eq 0 ]; then
  echo "{\"status\":\"ok\", \"file\":\"$FILE_PATH\", \"message\":\"Successfully compiled (basic syntax check ok). It does not mean the code is bug-free or stylistically perfect.\"}"
elif [ -n "$LINT_MESSAGE" ]; then
  ESCAPED_MESSAGE=$(echo "$LINT_MESSAGE" | tr -d '\n' | sed 's/"/\\"/g')
  echo "{\"status\":\"error\", \"file\":\"$FILE_PATH\", \"message\":\"$ESCAPED_MESSAGE\"}"
else
  echo "{\"status\":\"error\", \"file\":\"$FILE_PATH\", \"message\":\"Compilation check failed with exit code $EXIT_STATUS, but no specific message captured. Check file manually.\"}"
fi
