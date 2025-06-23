#!/bin/bash

# Terminal Wizard's Toolkit - A TUI Swiss Army Knife powered by gum

# --- Prerequisites Check ---
command -v gum >/dev/null || { echo -e "Error: gum is not installed. Please install it.\nOn Arch: sudo pacman -S gum" >&2; exit 1; }

# --- Configuration ---
SPINNER="dots"
BORDER_STYLE="double"
PADDING="0 1"
ACCENT_COLOR="87" # A nice blue/violet
ERROR_COLOR="9"   # Red
WARN_COLOR="214"  # Orange
INFO_COLOR="12"   # Bright blue

# --- Helper: Check Command Existence ---
check_cmd() {
    command -v "$1" &>/dev/null
}

# --- Helper: Run command and display with gum pager ---
run_cmd_paged() {
    local title="$1" cmd="$2"
    gum style --bold --foreground "$INFO_COLOR" "Running: $cmd"
    gum spin --spinner "$SPINNER" --title "$title" -- \
        bash -c "$cmd" | gum pager --border "$BORDER_STYLE" --border-foreground "$ACCENT_COLOR"
    # No explicit pause needed as pager waits for 'q'
}

# --- Helper: Run interactive command ---
run_cmd_interactive() {
     local title="$1" cmd="$2"
     gum style --bold --foreground "$INFO_COLOR" "Launching: $cmd ..."
     sleep 1 # Give user time to read
     clear
     if check_cmd "$cmd"; then
        $cmd # Execute directly, taking over terminal
        clear # Clear after interactive command exits
        gum style --foreground "$ACCENT_COLOR" "$cmd finished." ; sleep 1
     else
        gum style --foreground "$ERROR_COLOR" "Command '$cmd' not found!" ; sleep 2
     fi
}

# --- Helper: Get user input ---
get_input() {
    local prompt="$1" placeholder="$2" default_val="${3:-}"
    gum input --prompt "$prompt " --placeholder "$placeholder" --value "$default_val" --header "" # Keep it clean
}

# --- Sidebar Content ---
update_sidebar() {
    local cpu_usage mem_usage load_avg uptime_str disk_usage ip_info host_name os_info kernel_info

    # Basic CPU (instantaneous %user) - adjust parsing if needed
    cpu_usage=$(vmstat 1 2 | tail -1 | awk '{print $13}')
    # Basic Memory (% used)
    mem_usage=$(free | grep Mem | awk '{printf "%.0f", $3/$2 * 100.0}')
    # Load Average (1-min)
    load_avg=$(uptime | awk -F'load average: ' '{print $2}' | cut -d, -f1)
    # Uptime
    uptime_str=$(uptime -p | sed 's/up //')
    # Root Disk Usage (%)
    disk_usage=$(df -h / | awk 'NR==2 {print $5}')
    # Primary IP (guess common interfaces)
    ip_info=$(ip -4 addr show scope global | grep 'inet' | head -n1 | awk '{print $2}' | cut -d/ -f1) || ip_info="N/A"
    # Hostname
    host_name=$(hostname)
    # OS Info (requires lsb_release or os-release)
    if check_cmd lsb_release; then
        os_info=$(lsb_release -ds)
    elif [[ -f /etc/os-release ]]; then
        os_info=$(grep PRETTY_NAME /etc/os-release | cut -d= -f2 | tr -d '"')
    else
        os_info="Unknown OS"
    fi
     # Kernel
    kernel_info=$(uname -r)


    # Assemble with gum style
    sidebar_content=$(gum style --border "$BORDER_STYLE" --padding "$PADDING" --border-foreground "$ACCENT_COLOR" \
        "$(gum style --bold "$host_name ($os_info)")" \
        "$(gum style --faint "Kernel: $kernel_info")" \
        "$(gum style --faint "Uptime: $uptime_str")" \
        "" \
        "CPU: $cpu_usage% | Mem: $mem_usage% | Load: $load_avg" \
        "Disk (/): $disk_usage | IP: $ip_info"
    )
    echo "$sidebar_content"
}


# --- Menu Sections ---

show_system_menu() {
    local choice
    choice=$(printf "System Info Summary\nDisk Usage (df)\nTop Processes (htop)\nKernel Log (journalctl)\nBack" | \
        gum choose --header "System Tools" --cursor-prefix "> " --height 10)

    case "$choice" in
        "System Info Summary")
             clear
             # Use gum format for a potentially nicer static view
             ( echo "# System Summary"; uname -a; echo ; lscpu | head -n 20 ; echo; free -h; echo; df -h / ) | gum format -t markdown
             gum input --placeholder "Press Enter..." > /dev/null
             ;;
        "Disk Usage (df)") run_cmd_paged "Fetching Disk Usage..." "df -h";;
        "Top Processes (htop)") run_cmd_interactive "Launching htop..." "htop";;
        "Kernel Log (journalctl)")
            local filter=$(get_input "Journal Filter (optional, e.g., -u unit):" "Leave empty for kernel log")
            run_cmd_paged "Fetching Logs..." "journalctl -k $filter -n 500 --no-pager --output cat" ;; # --no-pager for gum pager
        "Back") return;;
    esac
}

show_process_menu() {
    local choice
    choice=$(printf "List All Processes (ps)\nFind Process by Name\nKill Process Interactively\nBack" | \
        gum choose --header "Process Management" --cursor-prefix "> " --height 10)

    case "$choice" in
        "List All Processes (ps)") run_cmd_paged "Fetching Processes..." "ps auxf";;
        "Find Process by Name")
            local name=$(get_input "Process Name:" "e.g., nginx, firefox")
            [[ -n "$name" ]] && run_cmd_paged "Searching for '$name'..." "ps aux | grep -iE '[ ]$name'" || gum style --foreground "$WARN_COLOR" "No name entered." ; sleep 1
            ;;
        "Kill Process Interactively")
             # Use fzf if available for a better experience, fallback to pgrep+gum choose
             if check_cmd fzf; then
                 local pid_to_kill
                 pid_to_kill=$(ps -eo pid,user,comm --no-headers | fzf --header="Select process to kill (Ctrl+C to cancel)" --layout=reverse | awk '{print $1}')
             else
                local process_list selected_process
                process_list=$(ps -eo pid,user,comm --no-headers | gum filter --placeholder "Type to filter processes...")
                selected_process=$(echo "$process_list" | gum choose --header "Select process to kill" --limit 1)
                pid_to_kill=$(echo "$selected_process" | awk '{print $1}')
             fi

            if [[ -n "$pid_to_kill" ]]; then
                 local signal=$(gum choose --header "Signal?" "TERM (15)" "KILL (9)" "HUP (1)")
                 signal_num=$(echo "$signal" | cut -d'(' -f2 | cut -d')' -f1)
                 if [[ -n "$signal_num" ]]; then
                     if gum confirm "Kill process PID $pid_to_kill with signal $signal_num?"; then
                         gum spin --spinner "$SPINNER" --title "Sending signal $signal_num to PID $pid_to_kill..." -- \
                            kill "-$signal_num" "$pid_to_kill"
                         if [[ $? -eq 0 ]]; then gum style --foreground "$ACCENT_COLOR" "Signal sent."; else gum style --foreground "$ERROR_COLOR" "Failed to send signal."; fi
                         sleep 1.5
                     else
                         gum style --foreground "$WARN_COLOR" "Kill cancelled." ; sleep 1
                     fi
                 fi
            else
                gum style --foreground "$WARN_COLOR" "No process selected or found." ; sleep 1
            fi
            ;;

        "Back") return;;
    esac
}


show_network_menu() {
     local choice
    choice=$(printf "Show IP Addresses\nShow Listening Ports (ss)\nShow Active Connections (ss)\nPing Host\nTraceroute Host\nDNS Lookup (dig)\nScan Local Network (nmap)\nBack" | \
        gum choose --header "Networking Tools" --cursor-prefix "> " --height 10)

     case "$choice" in
        "Show IP Addresses") run_cmd_paged "Fetching IPs..." "ip addr";;
        "Show Listening Ports (ss)") run_cmd_paged "Fetching Listening Ports..." "ss -tulnp";; # requires sudo for process names sometimes
        "Show Active Connections (ss)") run_cmd_paged "Fetching Connections..." "ss -tanp";; # requires sudo for process names sometimes
        "Ping Host")
             local host=$(get_input "Host or IP to ping:" "e.g., google.com or 8.8.8.8" "google.com")
             [[ -n "$host" ]] && run_cmd_paged "Pinging '$host'..." "ping -c 5 '$host'" || gum style --foreground "$WARN_COLOR" "No host entered." ; sleep 1
             ;;
        "Traceroute Host")
            local host=$(get_input "Host or IP to trace:" "e.g., google.com or 1.1.1.1" "google.com")
             [[ -n "$host" ]] && run_cmd_paged "Tracing route to '$host'..." "traceroute '$host'" || gum style --foreground "$WARN_COLOR" "No host entered." ; sleep 1
             ;;
        "DNS Lookup (dig)")
             local domain=$(get_input "Domain to lookup:" "e.g., github.com" "github.com")
             local type=$(get_input "Record Type (A, MX, TXT, ANY):" "Leave empty for A" "A")
             [[ -n "$domain" ]] && run_cmd_paged "Digging '$domain' ($type)..." "dig '$domain' $type +noall +answer" || gum style --foreground "$WARN_COLOR" "No domain entered." ; sleep 1
             ;;
         "Scan Local Network (nmap)")
             if ! check_cmd nmap; then gum style --foreground "$ERROR_COLOR" "nmap command not found!"; sleep 2; return; fi
             local target=$(get_input "Local Network Target:" "e.g., 192.168.1.0/24 or leave empty for auto-detect")
             # Basic auto-detect based on primary IP
             if [[ -z "$target" ]]; then
                 local ip_info=$(ip -4 addr show scope global | grep 'inet' | head -n1 | awk '{print $2}')
                 if [[ -n "$ip_info" ]]; then target="$ip_info"; else target="192.168.1.0/24"; fi # Fallback guess
                 gum style --foreground "$INFO_COLOR" "Auto-detected target: $target" ; sleep 1
             fi
             run_cmd_paged "Scanning '$target' (may take time)..." "sudo nmap -sn '$target'" # Simple ping scan
             ;;
        "Back") return;;
    esac
}

show_files_menu() {
    local choice
    choice=$(printf "Find Files by Name (fd/find)\nDirectory Disk Usage (du)\nView File Content (bat/less)\nBack" | \
        gum choose --header "File System Utilities" --cursor-prefix "> " --height 10)

     case "$choice" in
        "Find Files by Name (fd/find)")
             local name=$(get_input "Filename pattern:" "e.g., *.log or config.yaml")
             local dir=$(get_input "Search Directory:" "Leave empty for current (.)" ".")
             if [[ -n "$name" ]]; then
                if check_cmd fd; then
                    run_cmd_paged "Searching with fd..." "fd '$name' '$dir'"
                elif check_cmd find; then
                    run_cmd_paged "Searching with find..." "find '$dir' -type f -iname '$name'"
                else
                     gum style --foreground "$ERROR_COLOR" "Neither fd nor find command found!" ; sleep 2
                fi
             else
                 gum style --foreground "$WARN_COLOR" "No filename pattern entered." ; sleep 1
             fi
             ;;
        "Directory Disk Usage (du)")
             local dir=$(get_input "Directory Path:" "Leave empty for current (.)" ".")
             local depth=$(get_input "Max Depth (number):" "Leave empty for 1" "1")
             if [[ -d "$dir" ]]; then
                run_cmd_paged "Calculating usage for '$dir'..." "du -h --max-depth=$depth '$dir' | sort -hr"
             else
                gum style --foreground "$ERROR_COLOR" "Directory '$dir' not found." ; sleep 2
             fi
             ;;
        "View File Content (bat/less)")
             local file
             if check_cmd fzf; then
                 file=$(fzf --prompt="Select file to view > " --header="Use Tab for multi-select (views first)" --preview 'bat --color=always --style=numbers --line-range=:50 {} || head -n 50 {}' --layout=reverse)
             else
                 file=$(get_input "File path:" "Enter path to file")
             fi

            if [[ -n "$file" && -f "$file" ]]; then
                if check_cmd bat; then
                     run_cmd_interactive "Viewing with bat..." "bat --paging=always '$file'"
                elif check_cmd less; then
                     run_cmd_interactive "Viewing with less..." "less '$file'"
                else
                    gum style --foreground "$ERROR_COLOR" "Neither bat nor less command found!" ; sleep 2
                fi
            elif [[ -n "$file" ]]; then
                 gum style --foreground "$ERROR_COLOR" "File '$file' not found or not a regular file." ; sleep 2
            else
                 gum style --foreground "$WARN_COLOR" "No file selected." ; sleep 1
            fi
            ;;

        "Back") return;;
    esac
}

show_utils_menu() {
     local choice
    choice=$(printf "Encode Base64\nDecode Base64\nURL Encode\nURL Decode\nGenerate Password\nHex Dump File\nFormat JSON\nBack" | \
        gum choose --header "Handy Utilities" --cursor-prefix "> " --height 10)

    case "$choice" in
        "Encode Base64")
            local input=$(get_input "Text to encode:" "")
            [[ -n "$input" ]] && echo -n "$input" | base64 | gum format && gum input --placeholder "Encoded. Press Enter..." > /dev/null
            ;;
        "Decode Base64")
            local input=$(get_input "Base64 to decode:" "")
            [[ -n "$input" ]] && echo "$input" | base64 -d | gum format && gum input --placeholder "Decoded. Press Enter..." > /dev/null || gum style --foreground "$ERROR_COLOR" "Invalid Base64 input?" ; sleep 2
            ;;
         "URL Encode")
            local input=$(get_input "Text to URL encode:" "")
            [[ -n "$input" ]] && echo -n "$input" | jq -sRr @uri | gum format && gum input --placeholder "Encoded. Press Enter..." > /dev/null
             ;;
        "URL Decode")
            local input=$(get_input "URL encoded text to decode:" "")
             # Basic decoding using printf (might not handle all cases)
             [[ -n "$input" ]] && printf '%b\n' "${input//%/\\x}" | gum format && gum input --placeholder "Decoded. Press Enter..." > /dev/null
             ;;
        "Generate Password")
             local length=$(get_input "Password Length:" "e.g., 16" "16")
             local pw
             if check_cmd pwgen; then
                 pw=$(pwgen -s "$length" 1)
             elif check_cmd openssl; then
                 pw=$(openssl rand -base64 "$length" | tr -dc 'A-Za-z0-9' | head -c"$length") # Less secure chars
             else
                 gum style --foreground "$ERROR_COLOR" "Neither pwgen nor openssl found!" ; sleep 2; return
             fi
             gum style --bold "Generated Password:" "$pw"
             gum input --placeholder "Press Enter..." > /dev/null
             ;;
         "Hex Dump File")
            local file=$(get_input "File path for hex dump:" "")
             if [[ -f "$file" ]]; then
                if check_cmd xxd; then
                    run_cmd_paged "Hex dumping '$file'..." "xxd '$file'"
                elif check_cmd hexdump; then
                     run_cmd_paged "Hex dumping '$file'..." "hexdump -C '$file'"
                 else
                     gum style --foreground "$ERROR_COLOR" "Neither xxd nor hexdump command found!" ; sleep 2
                 fi
             elif [[ -n "$file" ]]; then
                  gum style --foreground "$ERROR_COLOR" "File '$file' not found." ; sleep 2
             fi
             ;;
        "Format JSON")
             local input=$(gum write --placeholder "Paste JSON here (Ctrl+D to finish)")
             if [[ -n "$input" ]]; then
                 if check_cmd jq; then
                    echo "$input" | jq '.' | gum pager || { gum style --foreground "$ERROR_COLOR" "Invalid JSON or jq error."; sleep 2; }
                 else
                     gum style --foreground "$ERROR_COLOR" "jq command not found!" ; sleep 2
                 fi
             fi
             ;;

        "Back") return;;
    esac
}

# --- Main Loop ---
sidebar_content="" # Initialize
while true; do
    # --- Update and Display Sidebar ---
    # Run sidebar update in background? Could make menu feel faster but risk race conditions/partial data
    sidebar_content=$(update_sidebar)

    # --- Prepare Main Menu ---
    menu_options=(
        "System Info & Logs"
        "Process Management"
        "Networking Tools"
        "File System Utilities"
        "Handy Utilities"
    )
    # Conditionally add launchers if scripts exist
    found_scripts=()
    check_cmd pactl_glam_tui.sh && found_scripts+=("Audio Control (pactl TUI)")
    check_cmd bt_glam_tui.sh && found_scripts+=("Bluetooth Control (bt TUI)")

    if [[ ${#found_scripts[@]} -gt 0 ]]; then
        menu_options+=("--- Launchers ---")
        menu_options+=("${found_scripts[@]}")
    fi
    menu_options+=("---")
    menu_options+=("Quit")

    # --- Combine Sidebar and Menu ---
    clear
    # Use gum layout with join this time
    # This requires knowing terminal width or estimating
    # Simplified: Just show sidebar then menu below
    # echo "$sidebar_content"
    # echo # Separator
    # gum style --bold --padding "$PADDING" --align center "Terminal Wizard's Toolkit"
    # main_menu_choice=$(printf "%s\n" "${menu_options[@]}" | \
    #    gum choose --header "Select Category" --cursor-prefix "=> " --height 15)

    # Alternative using join (might wrap badly on small terminals)
    main_menu_rendered=$(printf "%s\n" "${menu_options[@]}" | \
       gum choose --header "Select Category" --cursor-prefix "=> " --height 15)

    # Only proceed if a choice was made (not escaped)
    if [[ -n "$main_menu_choice" ]]; then
        clear # Clear layout before showing action/submenu
        gum join --align top --horizontal "$sidebar_content" \
            "$(gum style --padding "$PADDING" --border normal --border-foreground "$ACCENT_COLOR" "$main_menu_rendered")"
        sleep 0.1 # Tiny pause for visual effect
        clear
    else
        # Handle escape / Ctrl+C from main menu
        gum style --bold --foreground "$ACCENT_COLOR" "Exiting Toolkit."
        exit 0
    fi


    # --- Handle Choice ---
    case "$main_menu_choice" in
        "System Info & Logs") show_system_menu ;;
        "Process Management") show_process_menu ;;
        "Networking Tools") show_network_menu ;;
        "File System Utilities") show_files_menu ;;
        "Handy Utilities") show_utils_menu ;;
        "Audio Control (pactl TUI)") run_cmd_interactive "Launching Audio TUI..." "pactl_glam_tui.sh" ;;
        "Bluetooth Control (bt TUI)") run_cmd_interactive "Launching Bluetooth TUI..." "sudo bt_glam_tui.sh" ;; # Remember sudo
        "Quit")
            gum style --bold --foreground "$ACCENT_COLOR" "Exiting Toolkit."
            exit 0
            ;;
        "---"|"--- Launchers ---") ;; # Do nothing for separators
        *)
           gum style --foreground "$ERROR_COLOR" "Unknown option: $main_menu_choice"
           sleep 2
           ;;
    esac
done
