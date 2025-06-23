#!/usr/bin/env python3
# config/tools/web_search/scripts/web_search.py
import sys
import json
import os
import logging
import re
import urllib.parse
from bs4 import BeautifulSoup
import httpx
from typing import List, Dict, Any

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Environment variables
FALLBACK_SEARCH_BASE = os.getenv("FALLBACK_SEARCH_BASE", "https://en.wikipedia.org/w/index.php")
DEFAULT_MAX_RESULTS = int(os.getenv("MAX_WEB_RESULTS_DEFAULT", 3))
DEFAULT_TIMEOUT = int(os.getenv("DEFAULT_PROVIDER_TIMEOUT", 30))

def validate_url(url: str) -> bool:
    """Validate that the URL starts with http:// or https://."""
    return url.startswith(("http://", "https://"))

def extract_text(html_content: str) -> str:
    """Extract plain text from HTML content using BeautifulSoup."""
    try:
        soup = BeautifulSoup(html_content, 'html.parser')
        for element in soup(["script", "style", "nav", "header", "footer"]):
            element.decompose()
        text = soup.get_text(separator=" ", strip=True)
        text = re.sub(r'\s+', ' ', text).strip()
        return text
    except Exception as e:
        logger.error(f"Error parsing HTML: {e}")
        return ""

def rank_content(text: str, query: str) -> float:
    """Score content based on query relevance."""
    query_words = set(query.lower().split())
    text_words = text.lower().split()
    matches = sum(1 for word in query_words if word in text_words)
    return matches / max(len(query_words), 1)

def parse_search_results(html_content: str, query: str, max_results: int, base_url: str) -> List[Dict[str, Any]]:
    """Parse search results from a public domain (e.g., Wikipedia)."""
    try:
        soup = BeautifulSoup(html_content, 'html.parser')
        results = []
        # Wikipedia search results are in <li> tags within <div class='mw-search-results'>
        for result in soup.select("div.mw-search-results li")[:max_results]:
            title_elem = result.find("a", title=True)
            url = urllib.parse.urljoin(base_url, title_elem["href"]) if title_elem else ""
            if url and validate_url(url):
                snippet_elem = result.find("div", class_="searchresult")
                snippet = snippet_elem.get_text(strip=True) if snippet_elem else ""
                score = rank_content(snippet, query)
                results.append({
                    "url": url,
                    "title": title_elem["title"] if title_elem else "No title",
                    "content_snippet": snippet[:500],
                    "relevance_score": score
                })
        return results
    except Exception as e:
        logger.error(f"Error parsing search results: {e}")
        return []

def main():
    if len(sys.argv) < 2:
        print("CRITICAL_ERROR: web_search.py requires a JSON parameters string.", file=sys.stderr)
        sys.exit(1)

    params_json_str = sys.argv[1]

    try:
        params = json.loads(params_json_str)
    except json.JSONDecodeError as e:
        print(f"CRITICAL_ERROR: Invalid JSON parameters string: {e}", file=sys.stderr)
        print(f"Received: {params_json_str}", file=sys.stderr)
        sys.exit(1)

    # Required parameters
    query = params.get("query")
    if not query or not isinstance(query, str):
        print("CRITICAL_ERROR: 'query' string parameter is missing or invalid.", file=sys.stderr)
        sys.exit(1)

    # Optional parameters
    max_results = max(1, int(params.get("max_results", DEFAULT_MAX_RESULTS)))
    timeout = params.get("timeout", DEFAULT_TIMEOUT)
    search_base = params.get("search_base", FALLBACK_SEARCH_BASE)

    # Validate search base URL
    if not validate_url(search_base):
        print("SECURITY_ERROR: Invalid search_base URL; must start with http:// or https://.", file=sys.stderr)
        sys.exit(1)

    logger.info(f"Executing web search for query: {query}")
    logger.debug(f"Search base: {search_base}, Max results: {max_results}, Timeout: {timeout}s")

    results = []
    try:
        with httpx.Client(timeout=timeout) as client:
            # Construct search URL (e.g., Wikipedia search)
            search_url = f"{search_base}?search={urllib.parse.quote(query)}"
            logger.debug(f"Fetching search results from: {search_url}")
            response = client.get(search_url)
            response.raise_for_status()
            results = parse_search_results(response.text, query, max_results, search_base)

        # Sort results by relevance
        results = sorted(results, key=lambda x: x["relevance_score"], reverse=True)[:max_results]

        # Format output
        output = {
            "query": query,
            "results": results,
            "total_results": len(results),
            "status": "success",
            "source": "vendored"
        }
        print(json.dumps(output))

    except httpx.HTTPStatusError as e:
        error_output = {
            "query": query,
            "results": [],
            "total_results": 0,
            "status": "error",
            "error": f"HTTP error: {e.response.status_code} - {e.response.text}",
            "source": "vendored"
        }
        print(json.dumps(error_output), file=sys.stderr)
        sys.exit(1)
    except httpx.RequestError as e:
        error_output = {
            "query": query,
            "results": [],
            "total_results": 0,
            "status": "error",
            "error": f"Request failed: {e}",
            "source": "vendored"
        }
        print(json.dumps(error_output), file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        error_output = {
            "query": query,
            "results": [],
            "total_results": 0,
            "status": "error",
            "error": f"Search failed: {str(e)}",
            "source": "vendored"
        }
        print(json.dumps(error_output), file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
