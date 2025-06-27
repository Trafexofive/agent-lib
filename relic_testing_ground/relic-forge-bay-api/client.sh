#!/bin/bash

# Default Base URL - can be overridden by .env or .env.example
DEFAULT_BASE_URL="http://localhost:8000" # Script's internal default
BASE_URL="$DEFAULT_BASE_URL"

# --- Configuration & Helper Functions ---

# Detect API port from .env or .env.example in the current directory
if [ -f .env ] && grep -q -E '^FORGE_API_PORT=' .env; then
    PORT=$(grep -E '^FORGE_API_PORT=' .env | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then
        BASE_URL="http://localhost:$PORT"
    fi
elif [ -f .env.example ] && grep -q -E '^FORGE_API_PORT=' .env.example; then
    PORT=$(grep -E '^FORGE_API_PORT=' .env.example | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then
        BASE_URL="http://localhost:$PORT"
    fi
fi

# RAW_OUTPUT: If set to true, curl output will not be piped to jq
: "${RAW_OUTPUT:=false}"
# DEBUG: If set to true, enable debug output
: "${DEBUG:=false}"

print_usage() {
    echo "Usage: $0 <command> [options]" >&2 # Changed to >&2 for TUI context
    echo "This script provides a TUI. For command-line usage, refer to the original script." >&2
    echo "Commands (available via TUI):" >&2
    echo "  forge_relic <plan_file.json>             - Forge a new relic from a JSON plan file." >&2
    echo "  list_relics                            - List all forged relics." >&2
    echo "  get_relic_metadata <relic_id>          - Get metadata for a specific relic." >&2
    echo "  download_relic <relic_id> <output_file>  - Download a forged relic package." >&2
    echo "  delete_relic <relic_id> [--force]        - Delete a forged relic. Requires --force to skip confirmation." >&2
    echo "  get_health                             - Check system health." >&2
    echo "  get_capabilities                       - Get system capabilities." >&2
    echo "" >&2
    echo "Global Options (can be toggled in TUI or set as ENV vars):" >&2
    echo "  RAW_OUTPUT=true                          - Disable jq formatting for the command's output." >&2
    echo "  DEBUG=true                             - Enable debug output (shows target URL, curl command)." >&2
    echo "" >&2
    echo "Effective API Base URL: $BASE_URL" >&2
    echo "Requires curl and jq to be installed (unless RAW_OUTPUT=true for jq)." >&2
}

ensure_curl() {
    if ! command -v curl &> /dev/null; then
        echo "Error: curl is not installed. Please install curl to use this script." >&2
        exit 1
    fi
}

ensure_jq() {
    if [ "$RAW_OUTPUT" = "false" ] && ! command -v jq &> /dev/null; then
        echo "Error: jq is not installed. Please install jq or set RAW_OUTPUT=true (can be done in TUI)." >&2
        # For TUI, we might not want to exit immediately, but let the user toggle RAW_OUTPUT
        # However, exiting is safer if a command requiring jq is chosen.
        # The toggle action will also call ensure_jq.
        return 1 # Return error code, TUI can decide to exit or allow toggle
    fi
    return 0
}

_call_api() {
    local method="$1"
    local url_path="$2"
    local data_file_path="$3"
    local expect_empty_body_on_success="$4"
    local full_url="${BASE_URL}${url_path}"

    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: Effective BASE_URL in _call_api: $BASE_URL" >&2
        echo "DEBUG: Attempting to call: $method $full_url" >&2
    fi

    local response_output
    local http_status
    local curl_options=(-s -L -w "\n%{http_code}")

    local cmd_array=("curl" "${curl_options[@]}" "-X" "$method")
    if [ -n "$data_file_path" ]; then
        cmd_array+=("-H" "Content-Type: application/json" "-d" "@$data_file_path")
    fi
    cmd_array+=("$full_url")

    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: curl command: ${cmd_array[*]}" >&2
    fi

    response_output=$("${cmd_array[@]}")
    local curl_exit_code=$?

    http_status=$(echo -e "$response_output" | tail -n1)
    body=$(echo -e "$response_output" | sed '$d')

    if [ $curl_exit_code -ne 0 ]; then
        echo "Error: curl command failed with exit code $curl_exit_code for $method $full_url" >&2
        echo "Response Body (if any):" >&2
        echo "$body" >&2
        return 1
    fi

    if [ "$RAW_OUTPUT" = "true" ]; then
        echo "$body"
        echo "HTTP Status: $http_status" >&2
    else
        if ! command -v jq &> /dev/null; then # Double check jq if RAW_OUTPUT is false
            echo "Error: jq is not installed, but RAW_OUTPUT is false. Cannot format JSON." >&2
            echo "Raw Body:" >&2
            echo "$body"
            echo "HTTP Status: $http_status" >&2
            return 1 # Indicate error due to missing jq for formatting
        fi
        if [ "$http_status" -ge 200 ] && [ "$http_status" -lt 300 ]; then
            if [ "$expect_empty_body_on_success" = "true" ] && [ -z "$body" ]; then
                echo "{\"status\": \"success\", \"http_code\": $http_status, \"message\": \"Operation successful, no content returned.\"}" | jq '.'
            elif echo "$body" | jq -e . > /dev/null 2>&1; then
                echo "$body" | jq '.'
            elif [ -z "$body" ]; then
                 echo "{\"status\": \"success_empty_body\", \"http_code\": $http_status, \"message\": \"Operation successful, empty response body.\"}" | jq '.'
            else
                echo "Warning: API returned non-JSON response for a successful request (HTTP $http_status)." >&2
                echo "Raw Body:" >&2
                echo "$body"
            fi
        else
            echo "Error: API responded with HTTP status $http_status for $method $full_url" >&2
            if echo "$body" | jq -e . > /dev/null 2>&1; then
                echo "Response Body (JSON):" >&2
                echo "$body" | jq '.'
            else
                echo "Response Body (Raw):" >&2
                echo "$body"
            fi
            return 1
        fi
    fi
    return 0
}

forge_relic_cmd() {
    ensure_curl
    ensure_jq || return 1 # Exit if jq needed and not found
    local plan_file="$1"
    if [ -z "$plan_file" ]; then
        echo "Error: Plan file not specified for forge_relic." >&2
        print_usage >&2 # This will now point to TUI context
        return 1
    fi
    if [ ! -f "$plan_file" ]; then
        echo "Error: Plan file '$plan_file' not found." >&2
        return 1
    fi
    echo "Forging relic from plan: $plan_file" >&2
    _call_api "POST" "/forge/relic" "$plan_file"
}

list_relics_cmd() {
    ensure_curl
    ensure_jq || return 1
    echo "Listing all relics" >&2
    _call_api "GET" "/forge/relics"
}

get_relic_metadata_cmd() {
    ensure_curl
    ensure_jq || return 1
    local relic_id="$1"
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for get_relic_metadata." >&2
        print_usage >&2
        return 1
    fi
    echo "Getting metadata for relic ID: $relic_id" >&2
    _call_api "GET" "/forge/relics/${relic_id}"
}

download_relic_cmd() {
    ensure_curl
    local relic_id="$1"
    local output_file="$2"
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for download_relic." >&2
        print_usage >&2
        return 1
    fi
    if [ -z "$output_file" ]; then
        echo "Error: Output file not specified for download_relic." >&2
        print_usage >&2
        return 1
    fi
    echo "Downloading relic ID: $relic_id to $output_file" >&2
    local full_url="${BASE_URL}/forge/relics/${relic_id}/download"
    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: Effective BASE_URL for download: $BASE_URL" >&2
        echo "DEBUG: curl -s -L -X GET $full_url -o $output_file --fail" >&2
    fi
    curl -s -L -X GET "$full_url" -o "$output_file" --fail
    local curl_exit_code=$?
    if [ $curl_exit_code -eq 0 ]; then
        # Check if file exists and is non-empty, then check for gzip
        if [ -s "$output_file" ] && file "$output_file" 2>/dev/null | grep -q 'gzip compressed data'; then
            echo "Download complete: $(realpath "$output_file")"
        elif [ -f "$output_file" ]; then # File exists but is not gzip or is empty
            echo "Error: Downloaded file '$output_file' is not a valid gzip file or is empty." >&2
            echo "Content of (potentially failed) download at '$output_file':" >&2
            cat "$output_file" >&2 # Show content if any
            # Consider removing the empty/invalid file: rm -f "$output_file"
            return 1
        else # File does not exist (curl -o might not create it on some failures)
            echo "Error: Download failed, output file '$output_file' not created." >&2
            return 1
        fi
    else
        echo "Error: Download failed (curl exit code: $curl_exit_code)." >&2
        if [ -f "$output_file" ]; then
             echo "Content of (potentially failed) download at '$output_file':" >&2
             cat "$output_file" >&2
        fi
        return 1
    fi
}

delete_relic_cmd() {
    ensure_curl
    ensure_jq || return 1 # For potential JSON error from API
    local relic_id="$1"
    local force_delete="$2" # Will be empty unless TUI explicitly sets it
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for delete_relic." >&2
        print_usage >&2
        return 1
    fi
    if [ "$force_delete" != "--force" ]; then
        local confirmation
        # Ensure cursor is visible for this prompt
        tput cnorm
        read -r -p "Are you sure you want to delete relic '$relic_id'? This action cannot be undone. (yes/NO): " confirmation
        if [[ "$confirmation" != "yes" ]]; then
            echo "Deletion cancelled by user."
            return 0 # Not an error, user cancelled
        fi
    fi
    echo "Deleting relic ID: $relic_id" >&2
    _call_api "DELETE" "/forge/relics/${relic_id}" "" "true"
}

get_health_cmd() {
    ensure_curl
    ensure_jq || return 1
    echo "System Health" >&2
    _call_api "GET" "/system/health"
}

get_capabilities_cmd() {
    ensure_curl
    ensure_jq || return 1
    echo "System Capabilities" >&2
    _call_api "GET" "/system/capabilities"
}

# --- TUI Variables ---
current_selection=0
menu_items=(
    "Forge Relic"
    "List Relics"
    "Get Relic Metadata"
    "Download Relic"
    "Delete Relic"
    "Get Health"
    "Get Capabilities"
    "Toggle RAW_OUTPUT"
    "Toggle DEBUG"
    "Exit"
)
num_menu_items=${#menu_items[@]}

# --- TUI Functions ---

# Function to get input with a prompt, returns the input via stdout
get_input() {
    local prompt_message="$1"
    local user_input
    
    tput cnorm # Ensure cursor is visible for input
    printf "%s: " "$prompt_message"
    read -r user_input # Read user input
    echo "$user_input"  # Output the captured input
}

draw_menu() {
    clear
    echo "=== Relic Management TUI ==="
    echo "Effective API Base URL: $BASE_URL"
    echo "RAW_OUTPUT: $RAW_OUTPUT | DEBUG: $DEBUG"
    echo "-----------------------------------------------------"
    for i in "${!menu_items[@]}"; do
        local item_text="${menu_items[i]}"
        if [ "$item_text" == "Toggle RAW_OUTPUT" ]; then
            item_text="Toggle RAW_OUTPUT (currently: $RAW_OUTPUT)"
        elif [ "$item_text" == "Toggle DEBUG" ]; then
            item_text="Toggle DEBUG (currently: $DEBUG)"
        fi

        if [ "$i" -eq "$current_selection" ]; then
            echo " > $(tput setaf 2)$item_text$(tput sgr0)" # Green for selected
        else
            echo "   $item_text"
        fi
    done
    echo "-----------------------------------------------------"
    echo "Use 'k' (up), 'j' (down), <Enter> (select), 'q' (quit)"
}

execute_command() {
    local selected_item_text="${menu_items[$current_selection]}"
    clear
    echo "--- Executing: $selected_item_text ---"
    tput cnorm # Make cursor visible for command output and potential prompts

    local cmd_status=0
    case "$selected_item_text" in
        "Forge Relic")
            local plan_file
            plan_file=$(get_input "Enter plan file path")
            if [ -n "$plan_file" ]; then
                forge_relic_cmd "$plan_file"
                cmd_status=$?
            else
                echo "Plan file not provided. Action cancelled."
                cmd_status=1
            fi
            ;;
        "List Relics")
            list_relics_cmd
            cmd_status=$?
            ;;
        "Get Relic Metadata")
            local relic_id
            relic_id=$(get_input "Enter Relic ID")
            if [ -n "$relic_id" ]; then
                get_relic_metadata_cmd "$relic_id"
                cmd_status=$?
            else
                echo "Relic ID not provided. Action cancelled."
                cmd_status=1
            fi
            ;;
        "Download Relic")
            local relic_id output_file
            relic_id=$(get_input "Enter Relic ID")
            if [ -n "$relic_id" ]; then
                output_file=$(get_input "Enter output file path")
                if [ -n "$output_file" ]; then
                    download_relic_cmd "$relic_id" "$output_file"
                    cmd_status=$?
                else
                    echo "Output file not provided. Action cancelled."
                    cmd_status=1
                fi
            else
                echo "Relic ID not provided. Action cancelled."
                cmd_status=1
            fi
            ;;
        "Delete Relic")
            local relic_id
            relic_id=$(get_input "Enter Relic ID to delete")
            if [ -n "$relic_id" ]; then
                # delete_relic_cmd handles its own confirmation prompt
                delete_relic_cmd "$relic_id" "" # Pass empty for force_delete, cmd will ask
                cmd_status=$?
            else
                echo "Relic ID not provided. Action cancelled."
                cmd_status=1
            fi
            ;;
        "Get Health")
            get_health_cmd
            cmd_status=$?
            ;;
        "Get Capabilities")
            get_capabilities_cmd
            cmd_status=$?
            ;;
        "Toggle RAW_OUTPUT")
            if [ "$RAW_OUTPUT" = "true" ]; then
                RAW_OUTPUT="false"
                # If we are turning jq ON, check if it's available
                if ! ensure_jq; then
                    echo "Warning: jq is not installed. RAW_OUTPUT set to false, but formatting may fail." >&2
                    # Optionally, revert: RAW_OUTPUT="true"; echo "Reverted RAW_OUTPUT to true."
                fi
            else
                RAW_OUTPUT="true"
            fi
            echo "RAW_OUTPUT set to $RAW_OUTPUT"
            ;;
        "Toggle DEBUG")
            if [ "$DEBUG" = "true" ]; then
                DEBUG="false"
            else
                DEBUG="true"
            fi
            echo "DEBUG set to $DEBUG"
            ;;
        "Exit")
            echo "Exiting."
            tput cnorm # Ensure cursor is visible
            clear
            exit 0
            ;;
    esac

    if [ "$selected_item_text" != "Toggle RAW_OUTPUT" ] && [ "$selected_item_text" != "Toggle DEBUG" ]; then
      if [ $cmd_status -ne 0 ]; then
        echo "--- Command failed with status: $cmd_status ---"
      else
        echo "--- Command finished ---"
      fi
    fi
    echo "Press any key to return to the menu..."
    read -rsn1 # Wait for any key press
}

# --- Main TUI Loop ---
main_tui() {
    ensure_curl # Curl is always needed
    # Initial check for jq if RAW_OUTPUT is false
    if [ "$RAW_OUTPUT" = "false" ]; then
        if ! ensure_jq; then
            # If jq is not found, and RAW_OUTPUT is false, inform user and offer to switch
            echo "jq is not installed, which is needed for formatted JSON output." >&2
            echo "You can either install jq, or enable RAW_OUTPUT mode in the TUI." >&2
            echo "Press 'r' to attempt to run with RAW_OUTPUT=true, or any other key to exit." >&2
            read -rsn1 key_choice
            if [ "$key_choice" = "r" ]; then
                RAW_OUTPUT="true"
                echo "RAW_OUTPUT has been set to true." >&2
                sleep 1
            else
                exit 1
            fi
        fi
    fi

    # Hide cursor during menu navigation
    tput civis
    # Ensure cursor is made visible and terminal is sane on exit
    trap 'tput cnorm; stty sane; clear' EXIT INT TERM

    while true; do
        draw_menu
        # Read a single character, silently, without needing Enter
        read -rsn1 key 
        
        case "$key" in
            'k') # Up
                current_selection=$(( (current_selection - 1 + num_menu_items) % num_menu_items ))
                ;;
            'j') # Down
                current_selection=$(( (current_selection + 1) % num_menu_items ))
                ;;
            '') # Enter key (reads as empty string with -rsn1)
                execute_command
                # After command execution, re-hide cursor for menu
                tput civis 
                ;;
            'q') # Quit
                tput cnorm # Make cursor visible before exiting
                clear
                exit 0
                ;;
        esac
    done
}

# --- Script Entry Point ---
# All logic is now handled by the TUI
main_tui

exit $?
