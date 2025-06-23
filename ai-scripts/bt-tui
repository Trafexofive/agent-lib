#!/bin/bash

# Glamorous bluetoothctl TUI Wrapper using gum

# --- Prerequisites Check ---
command -v gum >/dev/null || { echo -e "Error: gum is not installed. Please install it.\nOn Arch: sudo pacman -S gum" >&2; exit 1; }
command -v bluetoothctl >/dev/null || { echo "Error: bluetoothctl command not found. Is bluez-utils installed?" >&2; exit 1; }

# --- Configuration ---
SPINNER="dots" # gum spinner type
BORDER_STYLE="rounded" # gum style border
PADDING="0 1" # gum style padding
ACCENT_COLOR="69" # gum style color (e.g., light blue)
ERROR_COLOR="9"  # Red
SCAN_DURATION=10 # Seconds to scan for devices

# --- Helper Functions ---

# Get Bluetooth Controller Info (Power Status)
get_controller_info() {
    local powered discovering controller_name
    # Use 'show' which is less verbose than 'list'
    local info=$(bluetoothctl show)
    powered=$(echo "$info" | grep -i 'Powered:' | sed 's/.*Powered: //')
    discovering=$(echo "$info" | grep -i 'Discovering:' | sed 's/.*Discovering: //')
    controller_name=$(echo "$info" | grep -i 'Name:' | sed 's/.*Name: //')

    echo "$(gum style --bold 'Bluetooth Status:')"
    echo "Controller: $controller_name"
    if [[ "$powered" == "yes" ]]; then
        echo "Power: $(gum style --foreground 2 'On')" # Green
    else
        echo "Power: $(gum style --foreground 1 'Off')" # Red
    fi
     if [[ "$discovering" == "yes" ]]; then
        echo "Scanning: $(gum style --foreground "$ACCENT_COLOR" 'Yes')"
    else
        echo "Scanning: No"
    fi
}

# Parse bluetoothctl devices/paired-devices output
# Args: $1 = command ("devices" or "paired-devices")
# Output: Newline-separated list of "MAC Name"
parse_devices() {
    local cmd="$1"
    bluetoothctl "$cmd" | grep '^Device' | awk '{
        mac=$2;
        # Reconstruct name which might have spaces
        name="";
        for (i=3; i<=NF; i++) {
            name = name $i " ";
        }
        # Trim trailing space
        sub(/ $/, "", name);
        print mac " " name;
    }' | sort
}

# Choose a device using gum
# Args: $1 = type ("Available" or "Paired")
#       $2 = command to list devices ("devices" or "paired-devices")
# Output: Selected device MAC address, or empty string if cancelled
choose_device() {
    local type="$1"
    local list_cmd="$2"
    local device_list device_choice device_mac

    # Use gum spin for feedback while fetching
    device_list=$(gum spin --spinner "$SPINNER" --title "Fetching ${type} devices..." -- \
        parse_devices "$list_cmd")

    if [[ -z "$device_list" ]]; then
        gum style --foreground "$ACCENT_COLOR" "No ${type} devices found."
        sleep 2
        return ""
    fi

    # Let user choose using gum choose, displaying "MAC Name"
    device_choice=$(echo "$device_list" | gum choose --header "Select ${type} Device" --cursor-prefix "> " --height 15)

    if [[ -n "$device_choice" ]]; then
        device_mac=$(echo "$device_choice" | cut -d' ' -f1)
        echo "$device_mac" # Return the MAC address
    else
        echo "" # Return empty if user cancelled
    fi
}

# Run a bluetoothctl command with feedback
# Args: $1 = Spinner title, $2... = bluetoothctl command and args
run_bt_cmd() {
    local title="$1"
    shift # Remove title arg
    local cmd_output
    cmd_output=$(gum spin --spinner "$SPINNER" --title "$title" -- bluetoothctl "$@")
    local exit_code=$?

    if [[ $exit_code -eq 0 ]]; then
        # Check output for common success/failure patterns if possible
        if echo "$cmd_output" | grep -qiE 'fail|error|not available'; then
            gum style --bold --foreground "$ERROR_COLOR" "Operation failed (check details below)."
            echo "$cmd_output" # Show output on failure
            sleep 3
            return 1 # Indicate failure
         else
             gum style --bold --foreground "$ACCENT_COLOR" "Operation successful."
             # echo "$cmd_output" # Optionally show output on success too
             sleep 1.5
             return 0 # Indicate success
         fi
    else
        gum style --bold --foreground "$ERROR_COLOR" "Command failed (exit code $exit_code)."
        echo "$cmd_output" # Show full output on command error
        sleep 3
        return $exit_code
    fi
}

# --- Main Actions ---

scan_for_devices() {
    gum style --bold "Scanning for devices for ${SCAN_DURATION} seconds..."
    # Run scan in background, sleep, then turn off
    bluetoothctl scan on > /dev/null &
    # Show spinner while scanning
    gum spin --spinner "$SPINNER" --title "Scanning..." -- sleep "$SCAN_DURATION"
    bluetoothctl scan off > /dev/null
    gum style --foreground "$ACCENT_COLOR" "Scan complete."
    sleep 1
    # Optionally list discovered devices right after
    clear
    echo "$(gum style --bold --underline 'Discovered Devices:')"
    parse_devices "devices" | gum format -t markdown || echo "No devices found."
    gum input --placeholder "Press Enter to continue..." > /dev/null
}

pair_device() {
    local mac
    mac=$(choose_device "Available" "devices")
    [[ -z "$mac" ]] && return
    gum style --bold "Attempting to pair with $mac..." \
        "You might need to confirm PINs on devices."
    # Pairing can be interactive, run_bt_cmd might struggle
    # Try simple execution first
     if run_bt_cmd "Pairing with $mac..." pair "$mac"; then
        # Often needs trusting after pairing
        if gum confirm "Trust this device ($mac)?"; then
             run_bt_cmd "Trusting $mac..." trust "$mac"
        fi
     fi
}

connect_device() {
    local mac
    mac=$(choose_device "Paired" "paired-devices")
    [[ -z "$mac" ]] && return
    run_bt_cmd "Connecting to $mac..." connect "$mac"
}

disconnect_device() {
    # List connected devices - grep for connected status in 'devices'
    local connected_devices device_choice mac
    connected_devices=$(bluetoothctl devices Connected | grep '^Device' | awk '{mac=$2; name=""; for (i=3; i<=NF; i++) {name=name $i " "} sub(/ $/,"",name); print mac " " name}' | sort)

     if [[ -z "$connected_devices" ]]; then
        gum style --foreground "$ACCENT_COLOR" "No devices currently connected."
        sleep 2
        return
    fi

     device_choice=$(echo "$connected_devices" | gum choose --header "Select Device to Disconnect" --cursor-prefix "> " --height 10)

    if [[ -n "$device_choice" ]]; then
        mac=$(echo "$device_choice" | cut -d' ' -f1)
        run_bt_cmd "Disconnecting $mac..." disconnect "$mac"
    fi
}

remove_device() {
    local mac name
    mac=$(choose_device "Paired" "paired-devices")
    [[ -z "$mac" ]] && return
    name=$(bluetoothctl devices Paired | grep "$mac" | awk '{name=""; for(i=3;i<=NF;i++) name=name $i " "; sub(/ $/,"",name); print name}')

    if gum confirm "Remove (unpair) device '$name' ($mac)?"; then
        run_bt_cmd "Removing $mac..." remove "$mac"
    else
         gum style "Operation cancelled."
         sleep 1
    fi
}

show_device_info() {
     local mac device_choice
     # Offer choice between all known or just paired
     local scope_choice=$(echo -e "All Known Devices\nPaired Devices" | gum choose --header "Show info for which set?")

     case "$scope_choice" in
        "All Known Devices") mac=$(choose_device "Known" "devices");;
        "Paired Devices") mac=$(choose_device "Paired" "paired-devices");;
        *) return;; # Cancelled
     esac

    [[ -z "$mac" ]] && return

    clear
    gum spin --spinner "$SPINNER" --title "Fetching info for $mac..." -- \
        bluetoothctl info "$mac" | gum format -t markdown # Use gum format for nice display
    gum input --placeholder "Press Enter to continue..." > /dev/null

}

toggle_power() {
    local info powered
    info=$(bluetoothctl show)
    powered=$(echo "$info" | grep -i 'Powered:' | sed 's/.*Powered: //')

    if [[ "$powered" == "yes" ]]; then
        if gum confirm "Turn Bluetooth Power Off?"; then
            run_bt_cmd "Turning power off..." power off
        fi
    else
         if gum confirm "Turn Bluetooth Power On?"; then
            run_bt_cmd "Turning power on..." power on
        fi
    fi
}


# --- Main Loop ---

while true; do
    # --- Prepare Sidebar Content ---
    sidebar_content=$(gum style --border "$BORDER_STYLE" --padding "$PADDING" --border-foreground "$ACCENT_COLOR" \
        "$(get_controller_info)")

    # --- Prepare Main Menu ---
    # Dynamically show Power On/Off based on current state
    power_state_text="Toggle Power" # Default, we'll use confirm later anyway
    # info=$(bluetoothctl show) # Could optimize by getting info once
    # powered=$(echo "$info" | grep -i 'Powered:' | sed 's/.*Powered: //')
    # [[ "$powered" == "yes" ]] && power_state_text="Power Off" || power_state_text="Power On"

    menu_options=(
        "Scan for Devices ($SCAN_DURATION sec)"
        "Pair New Device"
        "Connect Paired Device"
        "Disconnect Device"
        "Remove (Unpair) Device"
        "Trust Paired Device"
        "Show Device Info"
        "---"
        "$power_state_text"
        "---"
        "Quit"
    )
    main_menu_choice=$(printf "%s\n" "${menu_options[@]}" | gum choose --header "Bluetooth Control" --cursor-prefix "> " --height 15 --selected="Scan for Devices ($SCAN_DURATION sec)") # Default selection


    # --- Combine and Display ---
    clear
    # We won't show the choice in the sidebar this time, just the status
    gum join --align top --horizontal "$sidebar_content" "$(gum style --padding "$PADDING" " ")" # Placeholder for right side


    # --- Handle Choice ---
    clear # Clear layout before showing action output

    case "$main_menu_choice" in
        "Scan for Devices ($SCAN_DURATION sec)") scan_for_devices ;;
        "Pair New Device") pair_device ;;
        "Connect Paired Device") connect_device ;;
        "Disconnect Device") disconnect_device ;;
        "Remove (Unpair) Device") remove_device ;;
         "Trust Paired Device")
            mac=$(choose_device "Paired" "paired-devices")
            [[ -n "$mac" ]] && run_bt_cmd "Trusting $mac..." trust "$mac"
            ;;
        "Show Device Info") show_device_info ;;
        "$power_state_text") toggle_power ;;
        "Quit")
            gum style --bold --foreground "$ACCENT_COLOR" "Exiting."
            # Attempt to turn scanning off if it was left on? Risky.
            # bluetoothctl scan off &> /dev/null
            exit 0
            ;;
        "---") ;; # Separator
        *)
          if [[ -z "$main_menu_choice" ]]; then
                gum style --bold --foreground "$ACCENT_COLOR" "Exiting."
                exit 0
          fi
           gum style --foreground "$ERROR_COLOR" "Unknown action."
           sleep 2
           ;;
    esac

    # Pause slightly to let user see the result of the action
    # sleep 0.5 # Optional pause, adjust as needed
done
