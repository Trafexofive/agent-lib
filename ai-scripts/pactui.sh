#!/bin/bash

# TUI wrapper for pactl to control default SINK (OUTPUT) volume with j/k
# Version with added pizazz (colors!)

# --- Configuration ---
VOLUME_STEP=5 # Percentage change per key press
BAR_WIDTH=30  # Width of the volume bar in characters
POLL_INTERVAL=0.1 # Seconds between key read attempts

# --- Pizzazz Elements (ANSI Colors) ---
# Standard Colors
COLOR_RESET='\e[0m'
COLOR_BLACK='\e[0;30m'
COLOR_RED='\e[0;31m'
COLOR_GREEN='\e[0;32m'
COLOR_YELLOW='\e[0;33m'
COLOR_BLUE='\e[0;34m'
COLOR_MAGENTA='\e[0;35m'
COLOR_CYAN='\e[0;36m'
COLOR_WHITE='\e[0;37m'
# Bright/Bold Colors
COLOR_BOLD_BLACK='\e[1;30m' # Often appears grey
COLOR_BOLD_RED='\e[1;31m'
COLOR_BOLD_GREEN='\e[1;32m'
COLOR_BOLD_YELLOW='\e[1;33m'
COLOR_BOLD_BLUE='\e[1;34m'
COLOR_BOLD_MAGENTA='\e[1;35m'
COLOR_BOLD_CYAN='\e[1;36m'
COLOR_BOLD_WHITE='\e[1;37m'
# Dim/Faint (May not work on all terminals)
COLOR_DIM='\e[2m'

# Assign colors to elements
C_TITLE=${COLOR_BOLD_CYAN}
C_LABEL=${COLOR_GREEN}
C_VALUE=${COLOR_BOLD_WHITE}
C_MUTED=${COLOR_BOLD_RED}
C_BAR_FILLED=${COLOR_BLUE}
C_BAR_EMPTY=${COLOR_DIM}${COLOR_WHITE} # Dim white/grey for the empty part
C_CONTROLS=${COLOR_YELLOW}
C_RESET=${COLOR_RESET} # Important to reset at the end

# Bar Characters (Using Unicode Block for smoother look)
BAR_FILLED_CHAR="█"
BAR_EMPTY_CHAR="-" # Using simple dash for contrast, could use '░' (Light Shade)

# --- Runtime Variables ---
current_vol="?"
mute_status="?"
default_sink_name=""

# --- Basic Requirements Check ---
if ! command -v pactl &> /dev/null || ! command -v awk &> /dev/null || \
   ! command -v tput &> /dev/null || ! command -v stty &> /dev/null; then
    echo -e "${COLOR_BOLD_RED}Error: Missing required command (pactl, awk, tput, stty).${C_RESET}" >&2
    exit 1
fi

# --- Functions ---

# Function to clean up terminal on exit
cleanup() {
    stty echo # Re-enable terminal echoing
    tput cnorm # Show cursor
    echo -e "${C_RESET}" # Ensure colors are reset fully on exit
}

# Function to get the name of the current default SINK
update_default_sink_name() {
    default_sink_name=$(pactl info | awk '/^Default Sink: /{print $3}')
    if [[ -z "$default_sink_name" ]]; then
        echo -e "${C_MUTED}Error: Could not determine default sink from 'pactl info'.${C_RESET}" >&2
        return 1
    fi
    return 0
}

# Function to get volume/mute status for the default SINK
update_sink_info() {
    local sink_name="$1"
    local pactl_output
    local found_vol="?"
    local found_mute="?"

    pactl_output=$(pactl list sinks)
    if [[ $? -ne 0 ]]; then
        echo -e "${C_MUTED}Error: 'pactl list sinks' command failed.${C_RESET}" >&2
        current_vol="ERR"
        mute_status="ERR"
        return 1
    fi

    read -r found_vol found_mute <<< "$(echo "$pactl_output" | awk -v name="$sink_name" '
        BEGIN { RS = ""; FS = "\n"; target_vol="?"; target_mute="?"; found_block=0; }
        {
            current_block_name="";
            for (i=1; i<=NF; i++) { if ($i ~ /^[[:space:]]*Name: /) { current_block_name = $i; sub(/^[[:space:]]*Name: /, "", current_block_name); break; } }
            if (current_block_name == name) {
                found_block=1;
                for (i=1; i<=NF; i++) {
                    if (target_mute == "?" && $i ~ /^[[:space:]]*Mute: (yes|no)/) { match($i, /(yes|no)$/); target_mute = substr($i, RSTART, RLENGTH); }
                    if (target_vol == "?" && $i ~ /^[[:space:]]*Volume:.* ([0-9]+)%.*/) { match($i, /([0-9]+)%/); target_vol = substr($i, RSTART, RLENGTH - 1); }
                    if (target_vol != "?" && target_mute != "?") { break; }
                }
                print target_vol, target_mute; exit;
            }
        }
        END { if (found_block == 0 || target_vol == "?" || target_mute == "?") { print "? ?"; } }
    ')"

    if [[ "$found_vol" == "?" ]] || [[ "$found_mute" == "?" ]] || [[ -z "$found_vol" ]]; then
        current_vol="ERR"
        mute_status="ERR"
        echo -e "${C_MUTED}Error: Could not parse Volume/Mute for sink '${sink_name}'.${C_RESET}" >&2
        echo -e "${C_MUTED}Check 'pactl list sinks' output format.${C_RESET}" >&2
        return 1
    fi

    current_vol="$found_vol"
    mute_status="$found_mute"
    return 0
}


# Function to draw the UI with pizazz
draw_ui() {
    tput clear
    echo -e "${C_TITLE}--- PaCtl Sink (Output) Volume ---${C_RESET}"

    # Display Sink Name
    echo -e "${C_LABEL}Sink: ${C_VALUE}${default_sink_name}${C_RESET}"

    # Display Status/Volume
    if [[ "$current_vol" == "ERR" ]]; then
        echo -e "${C_LABEL}Status: ${C_MUTED}ERROR reading volume/mute status!${C_RESET}"
        echo -e "${C_YELLOW}Check pactl output manually.${C_RESET}"
        local bar_string="[ ${C_MUTED}--- Error ---${C_RESET} ]" # Simple error bar
    elif [[ "$mute_status" == "yes" ]]; then
        echo -e "${C_LABEL}Status: ${C_MUTED}MUTED ${C_DIM}(Volume was ${current_vol}%)${C_RESET}"
        local display_vol=0 # Treat muted as 0 for bar
    else
        echo -e "${C_LABEL}Volume: ${C_VALUE}${current_vol}%${C_RESET}"
        local display_vol=$current_vol
    fi

    # Draw the bar only if volume is known and not error
    if [[ "$current_vol" != "ERR" && "$current_vol" != "?" ]]; then
        local clamped_vol=$display_vol
        (( clamped_vol < 0 )) && clamped_vol=0
        (( clamped_vol > 100 )) && clamped_vol=100

        local filled_width=$(( (clamped_vol * BAR_WIDTH) / 100 ))
        local empty_width=$(( BAR_WIDTH - filled_width ))
        (( filled_width < 0 )) && filled_width=0
        (( empty_width < 0 )) && empty_width=0

        # Build the bar string with colors and characters
        local filled_part=""
        local empty_part=""
        [[ $filled_width -gt 0 ]] && filled_part=$(printf "%${filled_width}s" | tr ' ' "${BAR_FILLED_CHAR}")
        [[ $empty_width -gt 0 ]] && empty_part=$(printf "%${empty_width}s" | tr ' ' "${BAR_EMPTY_CHAR}")

        bar_string="[${C_BAR_FILLED}${filled_part}${C_BAR_EMPTY}${empty_part}${C_RESET}]"
    elif [[ "$current_vol" != "ERR" ]]; then
        # Handle case where volume is "?" but not "ERR" (shouldn't happen with current logic)
        bar_string="[ ${C_YELLOW}--- Initializing ---${C_RESET} ]"
    fi

    echo -e "${bar_string}" # Print the constructed bar

    echo # Blank line
    # Make controls stand out a bit more
    echo -e "${C_CONTROLS}[k]${C_RESET} Vol Up ▲ | ${C_CONTROLS}[j]${C_RESET} Vol Down ▼ | ${C_CONTROLS}[q]${C_RESET} Quit"
    echo # Blank line
}

# --- Main Execution ---
trap cleanup EXIT INT TERM
tput civis
stty -echo

if ! update_default_sink_name; then exit 1; fi
if ! update_sink_info "$default_sink_name"; then
    draw_ui # Show error state
    sleep 3
    exit 1
fi

# --- Main Loop ---
while true; do
    update_sink_info "$default_sink_name"
    draw_ui

    if read -rsN 1 -t "$POLL_INTERVAL" key; then
        case "$key" in
            k|K)
                pactl set-sink-volume @DEFAULT_SINK@ "+${VOLUME_STEP}%" > /dev/null 2>&1
                update_sink_info "$default_sink_name" # Immediate refresh for feel
                ;;
            j|J)
                if [[ "$mute_status" == "no" && "$current_vol" -gt 0 ]]; then
                    pactl set-sink-volume @DEFAULT_SINK@ "-${VOLUME_STEP}%" > /dev/null 2>&1
                    update_sink_info "$default_sink_name" # Immediate refresh
                else
                    # tput bel # Optional subtle feedback (beep)
                    :
                fi
                ;;
            q|Q)
                break
                ;;
            *)
                # tput bel # Optional beep for invalid keys
                ;;
        esac
    else
        : # Timeout, loop continues
    fi
done

exit 0 # Trap handles cleanup
