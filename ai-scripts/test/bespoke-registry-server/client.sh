#!/bin/bash

APP_PORT_FROM_ENV=$(grep ^APP_PORT= .env 2>/dev/null | cut -d '=' -f2 | tr -d '[:space:]')
BASE_URL="http://localhost:${APP_PORT_FROM_ENV:-8009}"
JQ_INSTALLED=$(command -v jq &> /dev/null; echo $?)
PRINTED_JQ_WARNING="false"

_print_json_response() {
  if [ "$JQ_INSTALLED" -eq 0 ]; then
    echo "$1" | jq '.'
  else
    echo "$1"
    if [ "$PRINTED_JQ_WARNING" != "true" ]; then
      echo "[INFO] Install jq for pretty-printed JSON output." >&2
      PRINTED_JQ_WARNING="true"
    fi
  fi
}

upload_asset() {
  local file_path="$1"
  local description="$2"

  if [ -z "$file_path" ]; then echo "Error: File path is required." >&2; return 1; fi
  if [ ! -f "$file_path" ]; then echo "Error: File '$file_path' not found." >&2; return 1; fi

  echo "Uploading asset '$file_path' with description '$description' to $BASE_URL/registry/assets ..."
  RESPONSE=$(curl -s -X POST "$BASE_URL/registry/assets" \
    -F "file=@$file_path" \
    -F "user_description=$description")
  _print_json_response "$RESPONSE"
}

list_assets() {
  echo "Listing assets from $BASE_URL/registry/assets ..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/registry/assets")
  _print_json_response "$RESPONSE"
}

get_asset_metadata() {
  local asset_id="$1"
  if [ -z "$asset_id" ]; then echo "Error: Asset ID is required." >&2; return 1; fi
  echo "Getting metadata for asset ID '$asset_id' from $BASE_URL/registry/assets/$asset_id ..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/registry/assets/$asset_id")
  _print_json_response "$RESPONSE"
}

download_asset() {
  local asset_id="$1"
  if [ -z "$asset_id" ]; then echo "Error: Asset ID is required." >&2; return 1; fi
  
  # Get filename from metadata first (optional, could also let curl use Content-Disposition)
  METADATA_JSON=$(curl -s -X GET "$BASE_URL/registry/assets/$asset_id")
  DOWNLOAD_FILENAME=$(echo "$METADATA_JSON" | jq -r '.original_filename // "downloaded_asset"')
  if [ -z "$DOWNLOAD_FILENAME" ] || [ "$DOWNLOAD_FILENAME" == "null" ]; then 
      DOWNLOAD_FILENAME="asset_${asset_id}_file"
  fi

  echo "Downloading asset ID '$asset_id' from $BASE_URL/registry/assets/$asset_id/download as '$DOWNLOAD_FILENAME' ..."
  curl -s -L -J -X GET "$BASE_URL/registry/assets/$asset_id/download" --output "$DOWNLOAD_FILENAME"
  
  if [ -f "$DOWNLOAD_FILENAME" ]; then
    echo "Download successful: $DOWNLOAD_FILENAME"
  else
    echo "Download failed or file not saved correctly."
  fi
}

delete_asset() {
  local asset_id="$1"
  if [ -z "$asset_id" ]; then echo "Error: Asset ID is required." >&2; return 1; fi
  echo "Deleting asset ID '$asset_id' from $BASE_URL/registry/assets/$asset_id ..."
  RESPONSE=$(curl -s -X DELETE "$BASE_URL/registry/assets/$asset_id")
  _print_json_response "$RESPONSE"
}

check_health() {
  echo "Checking system health at $BASE_URL/system/health ..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/system/health")
  _print_json_response "$RESPONSE"
}

check_capabilities() {
  echo "Checking system capabilities at $BASE_URL/system/capabilities ..."
  RESPONSE=$(curl -s -X GET "$BASE_URL/system/capabilities")
  _print_json_response "$RESPONSE"
}

COMMAND="$1"
shift

case "$COMMAND" in
  upload)
    upload_asset "$1" "$2"
    ;;
  list)
    list_assets
    ;;
  get)
    get_asset_metadata "$1"
    ;;
  download)
    download_asset "$1"
    ;;
  delete)
    delete_asset "$1"
    ;;
  health)
    check_health
    ;;
  capabilities)
    check_capabilities
    ;;
  help|*)
    echo "Usage: $0 {upload <file_path> [description]|list|get <asset_id>|download <asset_id>|delete <asset_id>|health|capabilities}"
    exit 1
    ;;
esac
