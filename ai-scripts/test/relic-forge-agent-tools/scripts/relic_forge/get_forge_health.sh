#!/bin/bash
# get_forge_health.sh

source "$(dirname "$0")/common_vars.sh"

call_api "GET" "/system/health"
exit $?
