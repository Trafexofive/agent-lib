#!/bin/bash
PARAMS_JSON="$1"
QUERY_TEXT=$(echo "$PARAMS_JSON" | jq -r .query_text)
TOP_K=$(echo "$PARAMS_JSON" | jq -r .top_k // 3)
echo "{\"status\":\"conceptual_success\", \"message\":\"KB retrieve similar called (stub)\", \"query_length\":${#QUERY_TEXT}, \"top_k\":$TOP_K, \"results\":[ {\"doc_id\":\"kb-doc-fake1\", \"text\":\"Placeholder similar text 1 for '$QUERY_TEXT'\", \"score\":0.9}, {\"doc_id\":\"kb-doc-fake2\", \"text\":\"Placeholder similar text 2 for '$QUERY_TEXT'\", \"score\":0.8} ]}"
