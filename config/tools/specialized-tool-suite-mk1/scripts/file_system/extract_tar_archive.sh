#!/bin/bash
PARAMS_JSON="$1"
ARCHIVE_PATH=$(echo "$PARAMS_JSON" | jq -r .archive_path)
DEST_DIR=$(echo "$PARAMS_JSON" | jq -r .destination_dir)

if [[ -z "$ARCHIVE_PATH" || "$ARCHIVE_PATH" == "null" || -z "$DEST_DIR" || "$DEST_DIR" == "null" ]]; then
  echo '{"error":"archive_path and destination_dir are required"}'; exit 1;
fi
if [ ! -f "$ARCHIVE_PATH" ]; then echo "{\"error\":\"Archive not found: $ARCHIVE_PATH\"}"; exit 1; fi

mkdir -p "$DEST_DIR"

if tar -xzf "$ARCHIVE_PATH" -C "$DEST_DIR"; then
  echo "{\"status\":\"success\", \"extracted_to\":\"$DEST_DIR\"}"
else
  echo "{\"error\":\"Failed to extract archive $ARCHIVE_PATH to $DEST_DIR\"}"; exit 1;
fi
