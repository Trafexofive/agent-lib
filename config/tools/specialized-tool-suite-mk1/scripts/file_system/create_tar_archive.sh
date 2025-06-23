#!/bin/bash
PARAMS_JSON="$1"
SOURCE_PATH=$(echo "$PARAMS_JSON" | jq -r .source_path)
DEST_ARCHIVE=$(echo "$PARAMS_JSON" | jq -r .destination_archive_path)
ARCHIVE_BASE_NAME=$(echo "$PARAMS_JSON" | jq -r .archive_base_name // "null")

if [[ -z "$SOURCE_PATH" || "$SOURCE_PATH" == "null" || -z "$DEST_ARCHIVE" || "$DEST_ARCHIVE" == "null" ]]; then
  echo '{"error":"source_path and destination_archive_path are required"}'; exit 1;
fi
if [ ! -e "$SOURCE_PATH" ]; then echo "{\"error\":\"Source path not found: $SOURCE_PATH\"}"; exit 1; fi

mkdir -p "$(dirname "$DEST_ARCHIVE")"

SOURCE_DIR=$(dirname "$SOURCE_PATH")
SOURCE_ITEM=$(basename "$SOURCE_PATH")

if [ "$ARCHIVE_BASE_NAME" != "null" ] && [ -n "$ARCHIVE_BASE_NAME" ]; then
  TEMP_STAGE_DIR=$(mktemp -d)
  mkdir -p "$TEMP_STAGE_DIR/$ARCHIVE_BASE_NAME"
  cp -a "$SOURCE_PATH" "$TEMP_STAGE_DIR/$ARCHIVE_BASE_NAME/"
  if tar -czf "$DEST_ARCHIVE" -C "$TEMP_STAGE_DIR" "$ARCHIVE_BASE_NAME"; then
    rm -rf "$TEMP_STAGE_DIR"
    echo "{\"status\":\"success\", \"archive_path\":\"$DEST_ARCHIVE\"}"
  else
    rm -rf "$TEMP_STAGE_DIR"
    echo "{\"error\":\"Failed to create archive from $SOURCE_PATH with base name $ARCHIVE_BASE_NAME\"}"; exit 1;
  fi
else
  if tar -czf "$DEST_ARCHIVE" -C "$SOURCE_DIR" "$SOURCE_ITEM"; then
    echo "{\"status\":\"success\", \"archive_path\":\"$DEST_ARCHIVE\"}"
  else
    echo "{\"error\":\"Failed to create archive from $SOURCE_PATH\"}"; exit 1;
  fi
fi
