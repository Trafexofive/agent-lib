#!/bin/bash
# list_relics.sh
# No parameters needed from JSON input.

source "$(dirname "$0")/common_vars.sh"

call_api "GET" "/forge/relics"
exit $?
