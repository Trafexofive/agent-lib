#!/bin/bash
source "$(dirname "$0")/common_vars.sh"
call_api "GET" "/system/health"
exit $?