#!/bin/bash

# Glamorous Zellij Session Manager TUI using gum

# --- Prerequisites Check ---
command -v gum >/dev/null || { echo -e "Error: gum is not installed. Please install it.\nOn Arch: sudo pacman -S gum" >&2; exit 1; }
command -v zellij >/dev/null || { echo "Error: zellij command not found. Please install zellij." >&2; exit 1; }

# --- Configuration ---
SPINNER="line"
BORDER_STYLE="rounded"
PADDING="0 1"
ACCENT_COLOR="208" # Orange - Zellij-like
ERROR_COLOR="9"    # Red
INFO_COLOR="12"    # Blue
WARN_COLOR="214"   # Yellow/Orange

# --- Helper Functions ---

# Get Running Zellij Sessions
# Output: Newline-separated list of "SessionName (Tabs)"
get_running_sessions() {
    local sessions_raw sessions_formatted
    # list-sessions output is simple: "SessionName" on each line (as of Zellij 0.39)
    # We can enhance this if future versions add more info, but for now, just the name is reliable.
    sessions_raw=$(zellij list-sessions 2>/dev/null)
    exit_code=$?

    if [[ $exit_code -ne 0 ]]; then
        # Check common errors
        if echo "$sessions_raw" | grep -qi "zellij server is not running"; then
            echo "(No Zellij server running)" # Special indicator for sidebar
            return 1 # Indicate no sessions / server down
        else
             echo "(Error listing sessions)"
             echo "$sessions_raw" >&2 # Print error to stderr
             return $exit_code
        fi
    elif [[ -z "$sessions_raw" ]]; then
        echo "(No active sessions)"
        return 1 # Indicate no sessions
    else
        # Just return the raw list, gum choose will handle it
        echo "$sessions_raw"
        return 0
    fi
}

# Choose a session using gum
# Output: Selected session name, or empty string if cancelled/none
choose_session() {
    local session_list session_choice
    session_list=$(get_running_sessions)
    local get_sessions_exit_code=$?

    # Handle cases where there are no sessions or an error occurred
    if [[ $get_sessions_exit_code -ne 0 ]]; then
        # get_running_sessions already printed a message for the sidebar
        # Add a message for the action context if needed
        if [[ "$session_list" == "(No Zellij server running)"* || "$session_list" == "(No active sessions)"* ]]; then
             gum style --foreground "$WARN_COLOR" "$session_list"
             sleep 2
        else
             gum style --foreground "$ERROR_COLOR" "$session_list" # Contains "(Error listing sessions)"
             sleep 2
        fi
        return "" # Return empty, indicating no choice possible/made
    fi

    # Let user choose using gum choose
    session_choice=$(echo "$session_list" | gum choose --header "Select Zellij Session" --cursor-prefix "> " --height 10)

    if [[ -n "$session_choice" ]]; then
        echo "$session_choice" # Return the session name
    else
        echo "" # Return empty if user cancelled (e.g., Ctrl+C)
    fi
}

# Run a non-interactive zellij command with feedback
run_zellij_cmd() {
    local title="$1"
    shift # Remove title arg
    local cmd_output
    cmd_output=$(gum spin --spinner "$SPINNER" --title "$title" -- zellij "$@")
    local exit_code=$?

    if [[ $exit_code -eq 0 ]]; then
        gum style --bold --foreground "$ACCENT_COLOR" "Operation successful."
        sleep 1.5
        return 0
    else
        # Try to give a slightly better error message
        local err_msg
        if echo "$cmd_output" | grep -qi "Session.*not found"; then
             err_msg="Session not found."
        elif echo "$cmd_output" | grep -qi "failed"; then
             err_msg="Operation failed (see details below)."
        else
            err_msg="Command failed (exit code $exit_code)."
        fi
         gum style --bold --foreground "$ERROR_COLOR" "$err_msg"
         # Show actual command output only if it contains something potentially useful
         if [[ -n "$cmd_output" && "$cmd_output" != *"Operation successful"* ]]; then
             echo "$cmd_output"
         fi
         sleep 3
        return $exit_code
    fi
}

# --- Main Actions ---

# Attach to selected session (takes over terminal)
action_attach_session() {
    local session_name
    session_name=$(choose_session)
    if [[ -n "$session_name" ]]; then
        gum style --foreground "$INFO_COLOR" "Attaching to session '$session_name'..."
        sleep 0.5
        clear
        # Execute zellij attach directly - it needs to take over
        zellij attach "$session_name"
        # After detaching, clear screen and show message
        clear
        gum style --foreground "$ACCENT_COLOR" "Detached from session '$session_name'."
        sleep 1.5
    fi
}

# Create a new session (takes over terminal)
action_new_session() {
    local session_name
    session_name=$(gum input --placeholder "Leave empty for default name" --prompt "Enter new session name (optional): ")

    if [[ -n "$session_name" ]]; then
        gum style --foreground "$INFO_COLOR" "Creating and attaching to new session '$session_name'..."
        sleep 0.5
        clear
        zellij --session "$session_name" # takes over
    else
        # Check if default session exists before creating unnamed
        if zellij list-sessions | grep -qE '^[0-9]+$'; then # Crude check for default numeric name
            if gum confirm "Default session might exist. Attach instead?"; then
                action_attach_session # Let user choose
                return
            fi
        fi
        gum style --foreground "$INFO_COLOR" "Creating and attaching to new default session..."
        sleep 0.5
        clear
        zellij # takes over
    fi
     # After detaching, clear screen and show message
    clear
    local final_name=${session_name:-"default"}
    gum style --foreground "$ACCENT_COLOR" "Detached from session '$final_name'."
    sleep 1.5
}

# Attach or create (simplified logic)
action_attach_or_create() {
     # Check if any sessions exist first
     if zellij list-sessions &>/dev/null; then
         # Sessions exist, try attaching
         action_attach_session
     else
         # No sessions (or server down), try creating
         action_new_session
     fi
}


# Kill a specific session
action_kill_session() {
    local session_name
    session_name=$(choose_session)
    if [[ -n "$session_name" ]]; then
        if gum confirm "Really kill session '$session_name'?"; then
             run_zellij_cmd "Killing session '$session_name'..." kill-session "$session_name"
        else
            gum style --foreground "$WARN_COLOR" "Kill cancelled." ; sleep 1
        fi
    fi
}

# Kill all sessions
action_kill_all_sessions() {
    # Check if server is running first
     if ! zellij list-sessions &>/dev/null; then
         gum style --foreground "$WARN_COLOR" "Zellij server is not running. Nothing to kill."
         sleep 2
         return
     fi

    if gum confirm "Really kill ALL Zellij sessions?"; then
        run_zellij_cmd "Killing all sessions..." kill-all-sessions
    else
        gum style --foreground "$WARN_COLOR" "Kill cancelled." ; sleep 1
    fi
}

# --- Main Loop ---
sidebar_content="" # Initialize
while true; do
    # --- Update and Display Sidebar ---
    session_list_for_sidebar=$(get_running_sessions)
    sidebar_content=$(gum style --border "$BORDER_STYLE" --padding "$PADDING" --border-foreground "$ACCENT_COLOR" \
        "$(gum style --bold 'Running Sessions:')" \
        "$(echo "$session_list_for_sidebar" | sed 's/^/ • /')") # Add bullet points

    # --- Prepare Main Menu ---
    menu_options=(
        "Attach to Session"
        "Create New Session"
        "Attach or Create"
        "---"
        "Kill Specific Session"
        "Kill ALL Sessions"
        "---"
        "Quit"
    )

    # --- Combine Sidebar and Menu ---
    clear
    main_menu_rendered=$(printf "%s\n" "${menu_options[@]}" | \
       gum choose --header "Zellij Session Manager" --cursor-prefix "=> " --height 12)

    # Only proceed if a choice was made (not escaped)
    if [[ -n "$main_menu_rendered" ]]; then
        # Display layout briefly before clearing for action
        gum join --align top --horizontal "$sidebar_content" \
            "$(gum style --padding "$PADDING" --border normal --border-foreground "$ACCENT_COLOR" "$main_menu_rendered")"
        sleep 0.1 # Tiny pause for visual effect
        clear
    else
        # Handle escape / Ctrl+C from main menu
        gum style --bold --foreground "$ACCENT_COLOR" "Exiting Zellij TUI."
        exit 0
    fi


    # --- Handle Choice ---
    case "$main_menu_rendered" in
        "Attach to Session") action_attach_session ;;
        "Create New Session") action_new_session ;;
        "Attach or Create") action_attach_or_create ;;
        "Kill Specific Session") action_kill_session ;;
        "Kill ALL Sessions") action_kill_all_sessions ;;
        "Quit")
            gum style --bold --foreground "$ACCENT_COLOR" "Exiting Zellij TUI."
            exit 0
            ;;
        "---") ;; # Do nothing for separators
        *)
           gum style --foreground "$ERROR_COLOR" "Unknown option: $main_menu_rendered"
           sleep 2
           ;;
    esac
done
