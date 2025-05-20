#!/bin/bash
# config/scripts/web/basic_ddg_snippet_search.sh
# Basic DuckDuckGo search snippet retriever using curl and pup (or similar HTML parser)
#
# Parameters (passed as a single JSON string argument):
# {
#   "query": "string (REQUIRED, the search query)",
#   "num_results": "integer (OPTIONAL, default 3, max 5, number of result snippets to return)"
# }

PARAMS_JSON="$1"

if [ -z "$PARAMS_JSON" ]; then
  echo "Error: No parameters JSON string provided to basic_ddg_snippet_search.sh." >&2
  exit 1
fi

# --- Dependency Check: pup (HTML parser) ---
if ! command -v pup &> /dev/null; then
    echo "Error: 'pup' HTML parser is not installed. Please install it (e.g., 'go install github.com/ericchiang/pup@latest')." >&2
    exit 1
fi
# --- Dependency Check: curl ---
if ! command -v curl &> /dev/null; then
    echo "Error: 'curl' is not installed. Please install it." >&2
    exit 1
fi
# --- Dependency Check: jq ---
if ! command -v jq &> /dev/null; then
    echo "Error: 'jq' is not installed. Please install it." >&2
    exit 1
fi

# --- Extract Parameters ---
QUERY=$(echo "$PARAMS_JSON" | jq -r .query)
NUM_RESULTS=$(echo "$PARAMS_JSON" | jq -r .num_results // "3") # Default to 3 if null

if [ "$QUERY" == "null" ] || [ -z "$QUERY" ]; then
    echo "Error: 'query' field is missing or empty in JSON parameters." >&2
    exit 1
fi

# Validate num_results (integer between 1 and 5)
if ! [[ "$NUM_RESULTS" =~ ^[0-9]+$ ]] || [ "$NUM_RESULTS" -lt 1 ] || [ "$NUM_RESULTS" -gt 5 ]; then
    echo "Error: 'num_results' must be an integer between 1 and 5. Got: '$NUM_RESULTS'." >&2
    NUM_RESULTS=3 # Fallback to default
fi

# --- Perform Search ---
# DuckDuckGo HTML version URL (less likely to change than JS-heavy versions)
# The `&ia=web` parameter often gives simpler results.
SEARCH_URL="https://html.duckduckgo.com/html/?q=$(echo "$QUERY" | curl -s -G --data-urlencode @- "" | sed 's/%20/+/g')"

echo "--- DDG Snippet Search Tool ---" >&2
echo "Querying: $SEARCH_URL" >&2
echo "Number of results requested: $NUM_RESULTS" >&2

# Fetch HTML content and parse using pup
# The selectors for DDG HTML can be fragile and might need updates if DDG changes its layout.
# This targets common result structures.
# .result__title a.result__a (for title and link)
# .result__snippet (for snippet)

HTML_CONTENT=$(curl -s -L -A "Mozilla/5.0 (compatible; ChimeraAgent/1.0; +http://localhost)" "$SEARCH_URL")

if [ -z "$HTML_CONTENT" ]; then
    echo "Error: Failed to fetch search results from DuckDuckGo." >&2
    exit 1
fi

# Use pup to extract titles, links, and snippets
# This is a simplified example; robust parsing would require more error handling.
# Combining title, link, and snippet for each result.
# The exact pup selectors might need adjustment based on DDG's current HTML structure.
RESULTS_JSON=$(echo "$HTML_CONTENT" | pup --charset UTF-8 ".web-result" \
    json{} | jq -s \
    --argjson num_res "$NUM_RESULTS" \
    'map(select(.children[0].children[0].tag == "a")) | .[0:$num_res] | map({
        title: (.children[0].children[0].text // "N/A"),
        link: (.children[0].children[0].href // "N/A"),
        snippet: (.children[1].text // "N/A" | gsub("\\n"; " ") | gsub(" +"; " ") | strflocaltime("%Y-%m-%d %H:%M:%S") as $now | gsub("Today"; $now | split(" ")[0]))  # Basic cleaning
    })')
    # The snippet often has a "Today" string for date, which is replaced. More complex date handling might be needed.

if [ -z "$RESULTS_JSON" ] || [ "$RESULTS_JSON" == "[]" ] || [ "$RESULTS_JSON" == "null" ]; then
    echo "No results found or parsing failed for query: '$QUERY'"
    exit 0
fi

# Output the JSON array of results
echo "$RESULTS_JSON"

echo "--- DDG Snippet Search Complete ---" >&2
exit 0
