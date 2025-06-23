#!/bin/bash

# Glamorous pactl TUI Wrapper with Sidebar using gum

# --- Prerequisites Check ---
command -v gum >/dev/null || { echo -e "Error: gum is not installed. Please install it.\nOn Arch: sudo pacman -S gum" >&2; exit 1; }
command -v pactl >/dev/null || { echo "Error: pactl command not found. Is pulseaudio or pipewire-pulse installed?" >&2; exit 1; }

# --- Configuration ---
SPINNER="dot" # gum spinner type (dot, line, pulse, etc.)
BORDER_STYLE="normal" # gum style border (normal, thick, double, rounded)
PADDING="0 1" # gum style padding (vertical horizontal)
ACCENT_COLOR="212" # gum style color (e.g., magenta/purple)

# --- Helper Functions ---

# Get Default Sink/Source Name/Description
get_default_device_info() {
    local type="$1" # "Sink" or "Source"
    local pactl_info pactl_list_short name description index
    pactl_info=$(pactl info)
    name=$(echo "$pactl_info" | grep "Default $type Name:" | cut -d':' -f2- | sed 's/^[ \t]*//')
    if [[ -z "$name" ]]; then
        echo "Default $type: N/A"
        return
    fi

    # Try to get the description for the name
    pactl_list_short=$(pactl list short "${type}s")
    index=$(echo "$pactl_list_short" | grep -w "$name" | awk '{print $1}')

    if [[ -n "$index" ]]; then
         description=$(pactl list "${type}s" | awk -v idx="$index" '
            /^'"$type"' #/ { current_index=$2 }
            current_index == "#" idx { in_dev=1 }
            in_dev && /Description: / { desc=$0; sub(/^[ \t]+Description: /,"",desc); print desc; exit }
            /^[ \t]*$/ { in_dev=0 }
        ')
    fi

    if [[ -n "$description" ]]; then
        echo "Default $type: $description ($name)"
    else
        echo "Default $type: $name" # Fallback to name if description fails
    fi
}

# List devices and let user choose one using gum
# Returns the chosen device identifier (index or name) or empty string if cancelled
choose_device() {
    local type="$1" # "sink" or "source"
    local type_plural="${type}s"
    local devices_formatted device_choice device_id

    # Use gum spin for feedback while fetching
    devices_formatted=$(gum spin --spinner "$SPINNER" --title "Fetching ${type_plural}..." -- \
        pactl list short "$type_plural" | while read -r line; do
            local index name description full_desc
            index=$(echo "$line" | awk '{print $1}')
            name=$(echo "$line" | awk '{print $2}')
            # Get description more reliably
            full_desc=$(pactl list "$type_plural" | awk -v idx="$index" '
                /^'"${type^}"' #/ { current_index=$2 } # Match Sink # or Source #
                current_index == "#" idx { in_dev=1 }
                in_dev && /Description: / { desc=$0; sub(/^[ \t]+Description: /,"",desc); print desc; exit }
                /^[ \t]*$/ { in_dev=0 }
            ')
            if [[ -n "$full_desc" ]]; then
                printf "%s: %s (%s)\n" "$index" "$full_desc" "$name"
            else
                printf "%s: %s\n" "$index" "$name" # Fallback
            fi
        done | sort -n)

    if [[ -z "$devices_formatted" ]]; then
        gum style --foreground "$ACCENT_COLOR" "No ${type_plural} found."
        sleep 2
        return ""
    fi

    # Let user choose using gum choose
    device_choice=$(echo "$devices_formatted" | gum choose --header "Select $type" --cursor-prefix "> " --height 10)

    # Extract index or name (prefer index)
    if [[ -n "$device_choice" ]]; then
        device_id=$(echo "$device_choice" | cut -d':' -f1)
        echo "$device_id" # Return the index
    else
        echo "" # Return empty if user cancelled (e.g., Ctrl+C)
    fi
}

# Set default device
set_default_device() {
    local type="$1" # "sink" or "source"
    local device_id
    device_id=$(choose_device "$type")
    if [[ -n "$device_id" ]]; then
        gum spin --spinner "$SPINNER" --title "Setting default $type..." -- \
            pactl set-default-"$type" "$device_id"
        if [[ $? -eq 0 ]]; then
             gum style --bold --foreground "$ACCENT_COLOR" "Default $type set to '$device_id'."
        else
             gum style --bold --foreground 9 "Error setting default $type." # Red
        fi
        sleep 1.5
    fi
}

# Adjust volume
adjust_volume() {
    local type="$1" # "sink" or "source"
    local device_id volume_change
    device_id=$(choose_device "$type")
    if [[ -n "$device_id" ]]; then
        volume_change=$(gum input --placeholder "Volume (+5%, -10%, 65%)" --prompt "Volume change for '$device_id': ")
        if [[ -n "$volume_change" ]]; then
            # Validate input slightly
            if [[ ! "$volume_change" =~ ^([+-]?[0-9]{1,3}%?)$ ]]; then
                 gum style --bold --foreground 9 "Invalid volume format: '$volume_change'"
                 sleep 2
                 return
            fi
            gum spin --spinner "$SPINNER" --title "Adjusting volume..." -- \
                pactl set-"$type"-volume "$device_id" "$volume_change"
           if [[ $? -eq 0 ]]; then
                 gum style --bold --foreground "$ACCENT_COLOR" "Volume for '$device_id' adjusted to '$volume_change'."
            else
                 gum style --bold --foreground 9 "Error adjusting volume."
            fi
            sleep 1.5
        fi
    fi
}

# Toggle mute
toggle_mute() {
    local type="$1" # "sink" or "source"
    local device_id
    device_id=$(choose_device "$type")
    if [[ -n "$device_id" ]]; then
        gum spin --spinner "$SPINNER" --title "Toggling mute..." -- \
            pactl set-"$type"-mute "$device_id" toggle
        if [[ $? -eq 0 ]]; then
             gum style --bold --foreground "$ACCENT_COLOR" "Mute toggled for '$device_id'."
        else
             gum style --bold --foreground 9 "Error toggling mute."
        fi
        sleep 1.5
    fi
}

# --- Main Loop ---

while true; do
    # --- Prepare Sidebar Content ---
    sidebar_content=$(gum style --border "$BORDER_STYLE" --padding "$PADDING" --border-foreground "$ACCENT_COLOR" \
        "$(gum style --bold 'Current Defaults:')" \
        "$(get_default_device_info Sink)" \
        "$(get_default_device_info Source)")

    # --- Prepare Main Menu ---
    menu_options=(
        "Set Default Sink"
        "Adjust Sink Volume"
        "Toggle Sink Mute"
        "---"
        "Set Default Source"
        "Adjust Source Volume"
        "Toggle Source Mute"
        "---"
        "List Sinks (raw)"
        "List Sources (raw)"
        "---"
        "Quit"
    )
    main_menu_choice=$(printf "%s\n" "${menu_options[@]}" | gum choose --header "PulseAudio Control" --cursor-prefix "> " --height 15)

    # --- Combine and Display ---
    # Clear screen before drawing new layout
    clear
    gum join --align top --horizontal "$sidebar_content" "$(echo "$main_menu_choice" | gum style --padding "$PADDING")" # Display choice temporarily for visual feedback

    # --- Handle Choice ---
    # Need a small delay or clear before action output sometimes
    # clear # uncomment if action output overlaps badly

    case "$main_menu_choice" in
        "Set Default Sink") set_default_device "sink";;
        "Adjust Sink Volume") adjust_volume "sink";;
        "Toggle Sink Mute") toggle_mute "sink";;

        "Set Default Source") set_default_device "source";;
        "Adjust Source Volume") adjust_volume "source";;
        "Toggle Source Mute") toggle_mute "source";;

        "List Sinks (raw)")
            clear
            echo "--- Available Sinks ---"
            pactl list short sinks | sort -n
            echo "-----------------------"
            gum input --placeholder "Press Enter to continue..." > /dev/null
            ;;
        "List Sources (raw)")
             clear
            echo "--- Available Sources ---"
            pactl list short sources | sort -n
            echo "------------------------"
            gum input --placeholder "Press Enter to continue..." > /dev/null
             ;;

        "Quit")
            gum style --bold --foreground "$ACCENT_COLOR" "Exiting."
            exit 0
            ;;
        "---") # Separator, do nothing
            ;;
        *) # No choice / Ctrl+C
          # gum choose returns empty string on ESC/Ctrl+C
          if [[ -z "$main_menu_choice" ]]; then
                gum style --bold --foreground "$ACCENT_COLOR" "Exiting."
                exit 0
          fi
           gum style --foreground 9 "Unknown option selected: $main_menu_choice"
           sleep 2
           ;;
    esac

    # No explicit pause needed as gum actions often involve interaction or sleep
done
