#!/bin/bash
PARAMS_JSON="$1"
TEXT_CONTENT=$(echo "$PARAMS_JSON" | jq -r .text_content)
SOURCE_ID=$(echo "$PARAMS_JSON" | jq -r .source_id)
TAGS=$(echo "$PARAMS_JSON" | jq -c .tags // "[]")
echo "{\"status\":\"conceptual_success\", \"message\":\"KB store text called (stub)\", \"text_length\":${#TEXT_CONTENT}, \"source_id\":\"$SOURCE_ID\", \"tags\":$TAGS, \"doc_id\":\"kb-doc-$(date +%s%N)\"}"
