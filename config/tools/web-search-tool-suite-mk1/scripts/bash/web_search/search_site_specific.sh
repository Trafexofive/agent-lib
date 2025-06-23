#!/bin/bash
PARAMS_JSON="$1"
QUERY=$(echo "$PARAMS_JSON" | jq -r .query)
SITE_DOMAIN=$(echo "$PARAMS_JSON" | jq -r .site_domain)
ENGINE=$(echo "$PARAMS_JSON" | jq -r .engine // "duckduckgo")

if [[ -z "$QUERY" || "$QUERY" == "null" ]]; then echo '{"error":"query parameter missing"}'; exit 1; fi
if [[ -z "$SITE_DOMAIN" || "$SITE_DOMAIN" == "null" ]]; then echo '{"error":"site_domain parameter missing"}'; exit 1; fi

# Construct the site-specific query
FULL_QUERY="site:${SITE_DOMAIN} ${QUERY}"
ENCODED_QUERY=$(echo "$FULL_QUERY" | jq -s -R -r @uri)

SEARCH_URL=""

case "$ENGINE" in
  "duckduckgo")
    SEARCH_URL="https://duckduckgo.com/?q=${ENCODED_QUERY}&ia=web"
    ;;
  "google")
    # WARNING: Scraping Google may violate ToS.
    SEARCH_URL="https://www.google.com/search?q=${ENCODED_QUERY}"
    ;;
  *)
    echo "{\"error\":\"Unsupported search engine for site-specific search: $ENGINE\"}"; exit 1;;
esac

HTTP_STATUS_FETCH=$(curl -s -L -A "AgentShellCurl/1.0 (WebSearchTool)" -o /dev/null -w "%{http_code}" "$SEARCH_URL")

if [ "$HTTP_STATUS_FETCH" -ge 200 ] && [ "$HTTP_STATUS_FETCH" -lt 400 ]; then
    echo "{\"status\":\"success\", \"engine\":\"$ENGINE\", \"query\":\"$QUERY\", \"site_domain\":\"$SITE_DOMAIN\", \"search_url\":\"$SEARCH_URL\", \"http_status_on_fetch_attempt\":$HTTP_STATUS_FETCH, \"message\":\"Site-specific search URL constructed.\"}"
else
    echo "{\"status\":\"error_fetching_serp\", \"engine\":\"$ENGINE\", \"query\":\"$QUERY\", \"site_domain\":\"$SITE_DOMAIN\", \"search_url\":\"$SEARCH_URL\", \"http_status_on_fetch_attempt\":$HTTP_STATUS_FETCH, \"message\":\"Could not reliably fetch the search engine results page.\"}"
fi
