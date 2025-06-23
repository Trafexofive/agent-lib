#!/bin/bash
# list_relics.sh
# No parameters needed from JSON input.

source "$(dirname "$0")/common_vars.sh"

# PARAMS_JSON="$1" # Not used for this script, but good practice to acknowledge

call_api "GET" "/forge/relics"
exit $?
