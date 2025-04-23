#!/usr/bin/env python3
import requests
import json
import argparse
import sys
import os # For model path check

# Attempt to import voice libraries, provide guidance if missing
try:
    import speech_recognition as sr
    vosk_available = True # Defined globally here
except ImportError:
    print("INFO: 'SpeechRecognition' library not found. Voice input disabled.", file=sys.stderr)
    print("INFO: To enable voice input, run: pip install SpeechRecognition vosk", file=sys.stderr)
    vosk_available = False # Or defined globally here
except Exception as e:
    print(f"ERROR: Unexpected error importing speech_recognition: {e}", file=sys.stderr)
    vosk_available = False # Or defined globally here

# --- Configuration ---
DEFAULT_SERVER_URL = "http://localhost:7777"
PROMPT_ENDPOINT = "/prompt"
REQUEST_TIMEOUT = 60 # Increased timeout slightly for potentially longer agent processing
DEFAULT_VOSK_MODEL_PATH = "vosk-model-small-en-us-0.15" # EXAMPLE path - USER MUST PROVIDE!

# --- Existing Network Function ---
def send_prompt_to_agent(server_url: str, user_prompt: str) -> str:
    """Sends the user prompt to the agent API server and returns the response text."""
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
        response.raise_for_status()
        try:
            response_json = response.json()
        except json.JSONDecodeError:
            raise ValueError(f"Server returned non-JSON response: {response.text[:100]}...")

        if "response" in response_json and isinstance(response_json["response"], str):
            return response_json["response"]
        elif "error" in response_json:
             error_details = response_json.get("details", "")
             return f"Agent API Error: {response_json['error']} {f'({error_details})' if error_details else ''}"
        else:
            raise ValueError(f"Unexpected JSON structure in response: {response_json}")

    except requests.exceptions.ConnectionError:
        return f"[Error]: Could not connect to the agent server at {api_endpoint}. Is it running?"
    except requests.exceptions.Timeout:
        return f"[Error]: Request timed out connecting to {api_endpoint}."
    except requests.exceptions.RequestException as e:
        status_code = e.response.status_code if e.response is not None else "N/A"
        error_body = e.response.text[:200] if e.response is not None else "No Response Body"
        return f"[Error]: HTTP Error {status_code} from server: {error_body}..."
    except ValueError as e:
        return f"[Error]: Invalid response from server: {e}"
    except Exception as e:
        return f"[Error]: An unexpected error occurred: {e}"

# --- NEW Voice Handling Function ---
def handle_voice_input(recognizer, microphone, model_path) -> str | None:
    """Captures audio, transcribes using Vosk, returns text or None on error."""
    global vosk_available # Need global here too if modifying it on error
    if not vosk_available:
        print("[Error]: Voice libraries not available.", file=sys.stderr)
        return None
    if not os.path.isdir(model_path):
         print(f"[Error]: Vosk model not found at path: {model_path}", file=sys.stderr)
         print(f"INFO: Download a model (e.g., small English) from https://alphacephei.com/vosk/models", file=sys.stderr)
         print(f"INFO: And place it at '{model_path}' or specify path with --vosk-model.", file=sys.stderr)
         return None

    try:
        with microphone as source:
            print("Adjusting for ambient noise...")
            try:
                 # Duration helps prevent cutting off noise adjustment too soon
                 recognizer.adjust_for_ambient_noise(source, duration=0.5)
            except Exception as e:
                 print(f"Warning: Failed ambient noise adjustment: {e}. Continuing...", file=sys.stderr)

            print("Listening... (Speak clearly and pause when finished)")
            try:
                # Listen until a pause is detected
                # timeout: max seconds of silence before considering phrase complete
                # phrase_time_limit: max seconds to record before stopping
                audio = recognizer.listen(source, timeout=5, phrase_time_limit=15)
            except sr.WaitTimeoutError:
                print("No speech detected within timeout.", file=sys.stderr)
                return None
            except Exception as e:
                 print(f"Error during listening: {e}", file=sys.stderr)
                 return None

        print("Transcribing...")
        try:
            # Use recognize_vosk for local transcription
            # *** IMPORTANT: Provide the model_path to recognize_vosk ***
            text_result = json.loads(recognizer.recognize_vosk(audio, model_path=model_path))
            text = text_result.get("text", "") # Extract text field from Vosk JSON result
            if not text:
                print("Vosk transcription returned empty text.", file=sys.stderr)
                return None
            print(f"Heard: '{text}'")
            return text
        except sr.UnknownValueError:
            print("Vosk could not understand audio", file=sys.stderr)
            return None
        except sr.RequestError as e:
            # This might indicate issues loading the model or with the Vosk installation
            print(f"Vosk error: {e}", file=sys.stderr)
            return None
        except json.JSONDecodeError:
            print("Error: Vosk returned non-JSON result.", file=sys.stderr)
            return None
        except Exception as e:
            # Catch other potential errors during transcription
            print(f"An unexpected error occurred during transcription: {e}", file=sys.stderr)
            return None

    except AttributeError:
        print("[Error]: SpeechRecognition or Microphone object not initialized correctly.", file=sys.stderr)
        return None
    except OSError as e:
         print(f"[Error]: Microphone OS error: {e}. Is a microphone connected and configured?", file=sys.stderr)
         vosk_available = False # Modify global if mic fails persistently
         return None
    except Exception as e:
        print(f"[Error]: An unexpected error occurred in voice handling: {e}", file=sys.stderr)
        return None


# --- Modified Main Loop ---
def main_loop(server_url: str, vosk_model_path: str):
    """Runs the main interactive command-line loop with voice option."""
    # *** ADD global DECLARATION HERE ***
    global vosk_available

    print(f"Connecting to Agent API at: {server_url}")
    print("Type your prompt and press Enter.")
    if vosk_available: # Reads the global variable
        print("Type '/v' or '/voice' then Enter to use voice input.")
    print("Type 'exit' or 'quit' to quit.")

    recognizer = None
    microphone = None
    if vosk_available: # Reads the global variable
        try:
            recognizer = sr.Recognizer()
            # Optional: Adjust energy threshold if needed
            # recognizer.energy_threshold = 4000
            microphone = sr.Microphone()
            print("Microphone initialized.") # Add confirmation
        except AttributeError:
             print("\n[WARN]: Failed to initialize SpeechRecognition/Microphone. Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable
        except OSError as e:
             print(f"\n[WARN]: Failed to access Microphone ({e}). Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable
        except Exception as e:
             print(f"\n[WARN]: Unknown error initializing voice components ({e}). Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable


    while True:
        user_input_trigger = ""
        try:
            user_input_trigger = input("\n> ")
        except EOFError:
            print("\nExiting (EOF received).")
            break # Exit loop on Ctrl+D
        except KeyboardInterrupt:
             print("\nExiting (Interrupted by user).")
             break

        user_input_final = ""

        if user_input_trigger.lower() in ["exit", "quit"]:
            print("Exiting.")
            break
        elif user_input_trigger.lower() in ["/v", "/voice"]:
            if vosk_available and recognizer and microphone: # Reads global + checks local vars
                transcribed_text = handle_voice_input(recognizer, microphone, vosk_model_path)
                if transcribed_text:
                    user_input_final = transcribed_text
                else:
                    print("(Voice input failed or cancelled)")
                    continue # Skip sending empty prompt
            else:
                print("[Error]: Voice input is not available or not initialized.", file=sys.stderr)
                continue
        elif not user_input_trigger.strip():
             continue # Ignore empty text input
        else:
            user_input_final = user_input_trigger # Use typed text

        if not user_input_final.strip():
            continue # If voice failed and resulted in empty, skip

        # Send the final input (either text or transcribed voice)
        agent_response = send_prompt_to_agent(server_url, user_input_final)
        print(f"\nAgent: {agent_response}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="CLI Client for the C++ Agent API with Voice Input.")
    parser.add_argument(
        "--url",
        type=str,
        default=DEFAULT_SERVER_URL,
        help=f"Base URL of the agent API server (default: {DEFAULT_SERVER_URL})"
    )
    parser.add_argument(
        "--vosk-model",
        type=str,
        default=DEFAULT_VOSK_MODEL_PATH,
        help=f"Path to the Vosk language model directory (default: {DEFAULT_VOSK_MODEL_PATH})"
    )
    args = parser.parse_args()

    # Check Vosk availability again before starting main loop if voice is intended
    # This reads the global vosk_available which might have been set False during import
    if not vosk_available:
        print("\n--- VOICE INPUT DISABLED (Import Failed) ---", file=sys.stderr)
        print("Install libraries ('pip install SpeechRecognition vosk') and ensure microphone access.", file=sys.stderr)
        # Optional: Exit if voice is strictly required, but better to allow text fallback
        # sys.exit(1)


    main_loop(args.url, args.vosk_model)
