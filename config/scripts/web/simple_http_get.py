#!/usr/bin/env python3
# config/scripts/web/simple_http_get.py
import sys
import json
import requests # Requires 'pip install requests' in the agent's Python environment

def main():
    if len(sys.argv) < 2:
        print("Error: No parameters JSON string provided.", file=sys.stderr)
        sys.exit(1)

    params_json_str = sys.argv[1]
    try:
        params = json.loads(params_json_str)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON parameters: {e}. Received: {params_json_str}", file=sys.stderr)
        sys.exit(1)

    url = params.get("url")
    timeout = params.get("timeout_seconds", 10) # Default to 10 seconds

    if not url or not isinstance(url, str):
        print("Error: 'url' string parameter is required.", file=sys.stderr)
        sys.exit(1)

    if not isinstance(timeout, int) or not (1 <= timeout <= 60):
        print("Error: 'timeout_seconds' must be an integer between 1 and 60.", file=sys.stderr)
        timeout = 10 # Fallback to default

    print(f"Fetching URL: {url} with timeout {timeout}s", file=sys.stderr)

    try:
        response = requests.get(url, timeout=timeout, headers={'User-Agent': 'ChimeraAgent/1.0'})
        response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)
        
        # Output the text content to stdout for the C++ tool to capture
        print(response.text)

    except requests.exceptions.Timeout:
        print(f"Error: Request to {url} timed out after {timeout} seconds.", file=sys.stderr)
        # Output an error message to stdout that the agent can parse
        print(f"HTTP_GET_ERROR: Timeout accessing URL {url}")
    except requests.exceptions.ConnectionError:
        print(f"Error: Could not connect to {url}. Check network or URL validity.", file=sys.stderr)
        print(f"HTTP_GET_ERROR: Connection error for URL {url}")
    except requests.exceptions.HTTPError as e:
        print(f"Error: HTTP error {e.response.status_code} for {url}.", file=sys.stderr)
        print(f"HTTP_GET_ERROR: HTTP {e.response.status_code} for URL {url}. Response: {e.response.text[:200]}")
    except requests.exceptions.RequestException as e:
        print(f"Error: A request exception occurred for {url}: {e}", file=sys.stderr)
        print(f"HTTP_GET_ERROR: Request exception for URL {url}: {e}")
    except Exception as e:
        print(f"An unexpected error occurred during HTTP GET for {url}: {e}", file=sys.stderr)
        print(f"HTTP_GET_ERROR: Unexpected error for URL {url}: {e}")
        
if __name__ == "__main__":
    main()
