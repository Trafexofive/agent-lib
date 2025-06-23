#!/bin/bash
# get_forge_capabilities.sh

source "$(dirname "$0")/common_vars.sh"

call_api "GET" "/system/capabilities"
exit $?
