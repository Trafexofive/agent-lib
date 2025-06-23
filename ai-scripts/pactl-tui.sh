#!/bin/bash

# Simple pactl TUI Wrapper - VIMified Navigation

# --- Helper Functions (identical to previous script) ---

# List devices (sinks or sources) with index and description
list_devices() {
    local type="$1" # "sinks" or "sources"
    echo "--- Available $type ---"
    pactl list short "$type" | while read -r line; do
        index=$(echo "$line" | awk '{print $1}')
        name=$(echo "$line" | awk '{print $2}')
        # Get description from the full list (more robust parsing attempt)
        description=$(pactl list "$type" | awk -v idx="$index" '
            /^Sink #/ || /^Source #/ { current_index=$2 }
            current_index == "#" idx { in_dev=1 }
            in_dev && /Description: / { desc=$0; sub(/^[ \t]+Description: /,"",desc); print idx ": " desc; exit }
            /^[ \t]*$/ { in_dev=0 }
        ')
         # Fallback if description parsing fails
        if [[ -z "$description" ]]; then
             description_line=$(pactl list "$type" | grep -A 5 "Name: $name" | grep 'Description:')
             description=$(echo "$description_line" | sed 's/^[ \t]*Description: //')
        fi
        # Even more basic fallback
        if [[ -z "$description" ]]; then
           printf "%s\t%s\n" "$index" "$name"
        else
           # Use index from description if available, otherwise original index
           desc_idx=$(echo "$description" | cut -d':' -f1)
           if [[ "$desc_idx" =~ ^[0-9]+$ ]]; then
                printf "%s\n" "$description"
           else
               printf "%s\t%s (%s)\n" "$index" "$description" "$name"
           fi
        fi
    done | sort -n # Sort numerically by index
    echo "---------------------"
}


# Set default device (sink or source)
set_default_device() {
    local type="$1" # "sink" or "source"
    local type_plural="${type}s"
    list_devices "$type_plural"
    read -p "Enter index or name of the desired default $type: " device_id
    if [[ -n "$device_id" ]]; then
        pactl set-default-"$type" "$device_id" && \
        echo "Default $type set to '$device_id'." || \
        echo "Error setting default $type."
    else
        echo "No device specified."
    fi
}

# Adjust volume (sink or source)
adjust_volume() {
    local type="$1" # "sink" or "source"
    local type_plural="${type}s"
    list_devices "$type_plural"
    read -p "Enter index or name of the $type to adjust: " device_id
    if [[ -z "$device_id" ]]; then
        echo "No device specified."
        return
    fi
    read -p "Enter volume change (e.g., +5%, -10%, 65%): " volume_change
     if [[ -n "$volume_change" ]]; then
        pactl set-"$type"-volume "$device_id" "$volume_change" && \
        echo "Volume for '$device_id' adjusted to '$volume_change'." || \
        echo "Error adjusting volume."
    else
        echo "No volume change specified."
    fi
}

# Toggle mute (sink or source)
toggle_mute() {
    local type="$1" # "sink" or "source"
    local type_plural="${type}s"
    list_devices "$type_plural"
    read -p "Enter index or name of the $type to toggle mute: " device_id
    if [[ -n "$device_id" ]]; then
        pactl set-"$type"-mute "$device_id" toggle && \
        echo "Mute toggled for '$device_id'." || \
        echo "Error toggling mute."
    else
        echo "No device specified."
    fi
}

# --- Main Menu ---

# Define menu items (text only)
menu_options=(
    "List Sinks"
    "Set Default Sink"
    "Adjust Sink Volume"
    "Toggle Sink Mute"
    "List Sources"
    "Set Default Source"
    "Adjust Source Volume"
    "Toggle Source Mute"
    "Quit"
)
num_options=${#menu_options[@]}
selected_item=0 # 0-based index

# Function to display the menu with selection highlight
display_menu() {
    clear
    echo "============================="
    echo " Simple PulseAudio Control (VIM Keys)"
    echo " (j: Down, k: Up, l/Enter: Select, q: Quit)"
    echo "============================="
    for i in "${!menu_options[@]}"; do
        if [[ $i -eq $selected_item ]]; then
            printf " > %s\n" "${menu_options[i]}" # Highlight selected
        else
            printf "   %s\n" "${menu_options[i]}"
        fi
    done
    echo "============================="
}

# --- Main Loop ---

while true; do
    display_menu

    # Read single character input without needing Enter
    read -n 1 -s key # -s hides the input

    case "$key" in
        j) # Move down
            selected_item=$(( (selected_item + 1) % num_options ))
            ;;
        k) # Move up
            selected_item=$(( (selected_item - 1 + num_options) % num_options ))
            ;;
        l|'') # Select ('' is often what Enter sends with read -n 1)
            action_index=$selected_item
            clear # Clear menu before showing action output
            case "$action_index" in
                0) list_devices "sinks";;
                1) set_default_device "sink";;
                2) adjust_volume "sink";;
                3) toggle_mute "sink";;
                4) list_devices "sources";;
                5) set_default_device "source";;
                6) adjust_volume "source";;
                7) toggle_mute "source";;
                8) echo "Exiting."; exit 0;;
                *) # Should not happen
                   echo "Internal error." ;;
            esac
             # Pause after action before showing menu again
            read -n 1 -s -r -p "Press any key to continue..."
            ;;
        q|Q)
            echo "Exiting."
            exit 0
            ;;
    esac
done
