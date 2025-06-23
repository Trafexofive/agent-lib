#!/bin/bash

# Read APP_PORT from .env file, default to 8000 if not set
if [ -f .env ] && grep -q '^APP_PORT=' .env;
  then APP_PORT_FROM_ENV=$(grep '^APP_PORT=' .env | cut -d'=' -f2 | tr -d '[:space:]')
fi
BASE_URL="http://localhost:${APP_PORT_FROM_ENV:-8000}"
JQ_INSTALLED=$(command -v jq &> /dev/null; echo $?)

_print_json_response() {
  if [ "$JQ_INSTALLED" -eq 0 ]; then
    echo "$1" | jq '.'
  else
    echo "$1"
    if [ "$JQ_INSTALLED" -ne 0 ] && [ "$PRINTED_JQ_WARNING" != "true" ]; then
      echo "[INFO] Install jq for pretty-printed JSON output." >&2
      PRINTED_JQ_WARNING="true"
    fi
  fi
}

forge_relic() {
  local plan_file=$1
  if [ -z "$plan_file" ]; then echo "Error: Plan file path is required for forge." >&2; return 1; fi
  if [ ! -f "$plan_file" ]; then echo "Error: Plan file '$plan_file' not found." >&2; return 1; fi
  echo "Attempting to forge relic from '$plan_file' against $BASE_URL/forge/relic..."
  RESPONSE=$(curl -s -X POST "$BASE_URL/forge/relic" \
    -H "Content-Type: application/json" \
    -d @"$plan_file")
  _print_json_response "$RESPONSE"
}

list_relics() {
  echo "Listing relics from $BASE_URL/forge/relics..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/forge/relics")
  _print_json_response "$RESPONSE"
}

download_relic() {
  local relic_id=$1
  if [ -z "$relic_id" ]; then echo "Error: Relic ID is required for download." >&2; return 1; fi
  local output_filename="relic_${relic_id}.tar.gz"
  echo "Downloading relic ID '$relic_id' from $BASE_URL/forge/relics/$relic_id/download to $output_filename..."
  curl -s -L -X GET "$BASE_URL/forge/relics/$relic_id/download" --output "$output_filename"
  if [ -f "$output_filename" ] && [ $(file "$output_filename" | grep -c 'gzip compressed data') -gt 0 ]; then
    echo "Download successful: $output_filename"
  else
    echo "Download failed or output is not a valid gzip file. Response was:"
    cat "$output_filename" # Show potential error message if not binary
    # rm -f "$output_filename" # Optionally remove failed download
  fi
}

delete_relic() {
  local relic_id=$1
  if [ -z "$relic_id" ]; then echo "Error: Relic ID is required for delete." >&2; return 1; fi
  echo "Deleting relic ID '$relic_id' from $BASE_URL/forge/relics/$relic_id..."
  RESPONSE=$(curl -s -X DELETE "$BASE_URL/forge/relics/$relic_id")
  _print_json_response "$RESPONSE"
}

check_health() {
  echo "Checking system health at $BASE_URL/system/health..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/system/health")
  _print_json_response "$RESPONSE"
}

check_capabilities() {
  echo "Checking system capabilities at $BASE_URL/system/capabilities..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/system/capabilities")
  _print_json_response "$RESPONSE"
}

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 {forge <plan_file.json>|list|download <relic_id>|delete <relic_id>|health|capabilities}"
    exit 1
fi

COMMAND=$1
shift

case $COMMAND in
  forge)
    forge_relic "$@"
    ;;
  list)
    list_relics
    ;;
  download)
    download_relic "$@"
    ;;
  delete)
    delete_relic "$@"
    ;;
  health)
    check_health
    ;;
  capabilities)
    check_capabilities
    ;;
  *)
    echo "Error: Unknown command '$COMMAND'"
    echo "Usage: $0 {forge <plan_file.json>|list|download <relic_id>|delete <relic_id>|health|capabilities}"
    exit 1
    ;;
esac
