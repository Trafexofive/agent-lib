#!/bin/bash
source "$(dirname "$0")/common_vars.sh"
call_api "GET" "/forge/relics"
exit $?