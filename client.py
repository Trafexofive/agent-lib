import requests
import json
import argparse
import sys

# --- Configuration ---
DEFAULT_SERVER_URL = "http://localhost:7777"
PROMPT_ENDPOINT = "/prompt"
REQUEST_TIMEOUT = 30 # Seconds

def send_prompt_to_agent(server_url: str, user_prompt: str) -> str:
    """
    Sends the user prompt to the agent API server and returns the response text.

    Args:
        server_url: The base URL of the agent API server.
        user_prompt: The text prompt from the user.

    Returns:
        The agent's response text.

    Raises:
        requests.exceptions.RequestException: If a network error occurs.
        ValueError: If the response is not valid JSON or missing the 'response' key.
        Exception: For other unexpected errors during the request.
    """
    api_endpoint = server_url.rstrip('/') + PROMPT_ENDPOINT
    payload = {"prompt": user_prompt}
    headers = {"Content-Type": "application/json"}

    try:
        response = requests.post(
            api_endpoint,
            json=payload,
            headers=headers,
            timeout=REQUEST_TIMEOUT
        )

        # Raise an exception for bad status codes (4xx or 5xx)
        response.raise_for_status()

        # Attempt to parse the JSON response
        try:
            response_json = response.json()
        except json.JSONDecodeError:
            raise ValueError(f"Server returned non-JSON response: {response.text[:100]}...") # Show snippet

        # Extract the agent's response text
        if "response" in response_json and isinstance(response_json["response"], str):
            return response_json["response"]
        elif "error" in response_json:
             error_details = response_json.get("details", "")
             return f"Agent API Error: {response_json['error']} {f'({error_details})' if error_details else ''}"
        else:
            raise ValueError(f"Unexpected JSON structure in response: {response_json}")

    except requests.exceptions.ConnectionError:
        raise requests.exceptions.ConnectionError(f"Could not connect to the agent server at {api_endpoint}. Is it running?")
    except requests.exceptions.Timeout:
        raise requests.exceptions.Timeout(f"Request timed out connecting to {api_endpoint}.")
    except requests.exceptions.RequestException as e:
        # Catch other request exceptions (like HTTPError from raise_for_status)
        status_code = e.response.status_code if e.response is not None else "N/A"
        error_body = e.response.text[:200] if e.response is not None else "No Response Body"
        raise requests.exceptions.RequestException(
            f"HTTP Error {status_code} from server: {error_body}..."
        ) from e
    except Exception as e:
        # Catch other potential errors
        raise Exception(f"An unexpected error occurred: {e}") from e


def main_loop(server_url: str):
    """Runs the main interactive command-line loop."""
    print(f"Connecting to Agent API at: {server_url}")
    print("Type your prompt and press Enter. Type 'exit' or 'quit' to quit.")

    while True:
        try:
            user_input = input("\n> ")
        except EOFError:
            print("\nExiting (EOF received).")
            break # Exit loop on Ctrl+D

        if user_input.lower() in ["exit", "quit"]:
            print("Exiting.")
            break
        if not user_input.strip():
            continue # Ignore empty input

        try:
            agent_response = send_prompt_to_agent(server_url, user_input)
            print(f"\nAgent: {agent_response}")
        except (requests.exceptions.RequestException, ValueError, Exception) as e:
            print(f"\n[Error]: {e}", file=sys.stderr)
        except KeyboardInterrupt:
             print("\nExiting (Interrupted by user).")
             break


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="CLI Client for the C++ Agent API.")
    parser.add_argument(
        "--url",
        type=str,
        default=DEFAULT_SERVER_URL,
        help=f"Base URL of the agent API server (default: {DEFAULT_SERVER_URL})"
    )
    args = parser.parse_args()

    main_loop(args.url)
