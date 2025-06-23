#!/bin/bash
PARAMS_JSON="$1"
QUERY=$(echo "$PARAMS_JSON" | jq -r .query)
ENGINE=$(echo "$PARAMS_JSON" | jq -r .engine // "duckduckgo")
NUM_RESULTS_HINT=$(echo "$PARAMS_JSON" | jq -r .num_results_hint // 10) # This is a hint, actual number depends on engine

if [[ -z "$QUERY" || "$QUERY" == "null" ]]; then echo '{"error":"query parameter missing"}'; exit 1; fi

# URL Encode the query
ENCODED_QUERY=$(echo "$QUERY" | jq -s -R -r @uri)

SEARCH_URL=""
HTML_OUTPUT=""

case "$ENGINE" in
  "duckduckgo")
    # DuckDuckGo HTML version is often less complex: &kp=-1 (no JS redirect), &kad=HTML
    # Using the standard search URL first. For HTML-only, often 'html.duckduckgo.com/html/'
    SEARCH_URL="https://duckduckgo.com/?q=${ENCODED_QUERY}&ia=web"
    # Alternative for simpler HTML: SEARCH_URL="https://html.duckduckgo.com/html/?q=${ENCODED_QUERY}"
    ;;
  "google")
    # WARNING: Scraping Google directly may violate ToS and is prone to blocks/CAPTCHAs.
    # This is for illustrative purposes. Using a proper API is recommended.
    SEARCH_URL="https://www.google.com/search?q=${ENCODED_QUERY}&num=${NUM_RESULTS_HINT}"
    ;;
  "bing_conceptual")
    # WARNING: Conceptual, scraping Bing may violate ToS.
    SEARCH_URL="https://www.bing.com/search?q=${ENCODED_QUERY}&count=${NUM_RESULTS_HINT}"
    ;;
  *)
    echo "{\"error\":\"Unsupported search engine: $ENGINE\"}"; exit 1;;
esac

# For this script, we will primarily return the search URL.
# Fetching and returning HTML can be very large and complex to parse in bash.
# The agent can use 'search_fetch_page_content' or 'net_curl_url' with this URL if needed.

HTTP_STATUS_FETCH=$(curl -s -L -A "AgentShellCurl/1.0 (WebSearchTool)" -o /dev/null -w "%{http_code}" "$SEARCH_URL")

if [ "$HTTP_STATUS_FETCH" -ge 200 ] && [ "$HTTP_STATUS_FETCH" -lt 400 ]; then
    echo "{\"status\":\"success\", \"engine\":\"$ENGINE\", \"query\":\"$QUERY\", \"search_url\":\"$SEARCH_URL\", \"http_status_on_fetch_attempt\":$HTTP_STATUS_FETCH, \"message\":\"Search URL constructed. Agent can use fetch_page_content or net_curl_url to get HTML. Direct SERP HTML parsing is complex.\"}"
else
    echo "{\"status\":\"error_fetching_serp\", \"engine\":\"$ENGINE\", \"query\":\"$QUERY\", \"search_url\":\"$SEARCH_URL\", \"http_status_on_fetch_attempt\":$HTTP_STATUS_FETCH, \"message\":\"Could not reliably fetch the search engine results page. The URL might be valid, but the server responded with an error or redirect loop during the test fetch.\"}"
fi
